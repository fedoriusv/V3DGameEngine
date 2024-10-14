#include "ShaderSpirVPatcherInvertOrdinate.h"
#include "Renderer/Core/Context.h"
#include "Utils/Logger.h"

#ifdef USE_SPIRV
#   include <shaderc/third_party/glslang/SPIRV/spirv.hpp>

namespace v3d
{
namespace resource
{

struct TypeFloat
{
    u32 word = getWordInstruction(spv::Op::OpTypeFloat, 3);
    u32 result = (u32)~0;
    u32 width = sizeof(f32) * 8;
};

struct TypeInt
{
    u32 word = getWordInstruction(spv::Op::OpTypeInt, 4);
    u32 result = (u32)~0;
    u32 width = sizeof(s32) * 8;
    u32 sign = 1;
};

template<int Count>
struct TypeStruct
{
    u32 word = getWordInstruction(spv::Op::OpTypeStruct, 2);
    u32 result = (u32)~0;
    u32 id[Count];
    u32 typeId[Count];
};

struct TypePointer
{
    u32 word = getWordInstruction(spv::Op::OpTypePointer, 4);
    u32 result = (u32)~0;
    u32 storageClass = (u32)~0;
    u32 type = (u32)~0;
};

template<typename Type>
struct Constant
{
    u32 word = getWordInstruction(spv::Op::OpConstant, 3 + sizeof(Type) / sizeof(u32));
    u32 resultType = (u32)~0;
    u32 result = (u32)~0;
    Type value = Type(0);
};

bool PatchInvertOrdinate::patch(const std::vector<u32>& spirv, std::vector<u32>& patchedSpirv, u32 flags)
{
    //gl_Position.y = -gl_Position.y;
    LOG_DEBUG("PatchInvertOrdinate::patch");

    if (spirv.empty())
    {
        return false;
    }

    u32 entryPointID = (u32)~0;
    u32 buildInPositionID = (u32)~0;
    u32 buildInPositionStructID = (u32)~0;
    bool insideMainEntryPointFunction = false;

    std::tuple<bool, TypeInt> int32Type(false, {});
    std::tuple<bool, Constant<s32>> constantInt0(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 0 });
    std::tuple<bool, Constant<s32>> constantInt1(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 1 });

    std::tuple<bool, TypeFloat> float32Type(false, {});
    std::tuple<bool, TypePointer> float32TypePointer(false, {});

    u32 buildInStructID = (u32)~0;
    std::tuple<bool, TypePointer> structTypePointer(false, {});

    auto word = spirv.begin();
    u32 boundIDs = *std::next(word, 3);

    patchedSpirv.clear();
    auto lastPatchedWord = word;

    word = std::next(word, 5);
    while (word != spirv.end())
    {
        u32 op = getOpCode(*word);
        u32 count = getCountWords(*word);

        switch (op)
        {
        case spv::Op::OpEntryPoint:
        {
            u32 executionModel = *std::next(word, 1);
            if (executionModel != spv::ExecutionModel::ExecutionModelVertex)
            {
                ASSERT(false, "must be vertex");
                return false;
            }

            u32 entryPoint = *std::next(word, 2);
            entryPointID = entryPoint;
        }
        break;

        case spv::Op::OpDecorate:
        {
            if (buildInPositionID != ~0)
            {
                break;
            }

            u32 target = *std::next(word, 1);
            u32 decoration = *std::next(word, 2);
            u32 builtIn = *std::next(word, 3);
            if (decoration == spv::Decoration::DecorationBuiltIn && builtIn == spv::BuiltIn::BuiltInPosition)
            {
                buildInPositionID = target; //first way
            }
        }
        break;

        case spv::Op::OpMemberDecorate:
        {
            if (buildInPositionID != ~0 || buildInStructID != ~0)
            {
                break;
            }

            u32 structType = *std::next(word, 1);
            u32 decoration = *std::next(word, 3);
            if (decoration == spv::Decoration::DecorationBuiltIn)
            {
                u32 builtIn = *std::next(word, 4);
                if (builtIn == spv::BuiltIn::BuiltInPosition)
                {
                    buildInStructID = structType; //second way
                }
            }
        }
        break;

        case spv::Op::OpTypeInt:
        {
            u32 result = *std::next(word, 1);
            u32 size = *std::next(word, 2);
            u32 signedness = *std::next(word, 3);
            if (size == 32 && !std::get<0>(int32Type))
            {
                std::get<0>(int32Type) = true;
                std::get<1>(int32Type).result = result;
                std::get<1>(int32Type).width = size;
                std::get<1>(int32Type).sign = signedness;
            }
        }
        break;

        case spv::Op::OpTypeFloat:
        {
            u32 result = *std::next(word, 1);
            u32 size = *std::next(word, 2);
            if (size == 32 && !std::get<0>(float32Type))
            {
                std::get<0>(float32Type) = true;
                std::get<1>(float32Type).result = result;
                std::get<1>(float32Type).width = size;
            }
        }
        break;

        case spv::Op::OpTypePointer:
        {
            u32 result = *std::next(word, 1);
            u32 storageClass = *std::next(word, 2);
            u32 type = *std::next(word, 3);

            if (std::get<0>(float32Type) || !std::get<0>(float32TypePointer))
            {
                if (storageClass == spv::StorageClass::StorageClassOutput && std::get<1>(float32Type).result == type)
                {
                    std::get<0>(float32TypePointer) = true;
                    std::get<1>(float32TypePointer).result = result;
                    std::get<1>(float32TypePointer).storageClass = storageClass;
                    std::get<1>(float32TypePointer).type = type;
                }
            }

            if (!std::get<0>(structTypePointer) || buildInStructID != ~0)
            {
                if (storageClass == spv::StorageClass::StorageClassOutput && buildInStructID == type)
                {
                    std::get<0>(structTypePointer) = true;
                    std::get<1>(structTypePointer).result = result;
                    std::get<1>(structTypePointer).storageClass = storageClass;
                    std::get<1>(structTypePointer).type = type;
                }
            }
        }
        break;

        case spv::Op::OpVariable:
        {
            if (buildInPositionID != ~0 || !std::get<0>(structTypePointer))
            {
                break;
            }

            u32 resultType = *std::next(word, 1);
            u32 result = *std::next(word, 2);
            u32 storageClass = *std::next(word, 3);

            if (storageClass == spv::StorageClass::StorageClassOutput && std::get<1>(structTypePointer).result == resultType)
            {
                buildInPositionStructID = result; //second way
            }

        }
        break;

        case spv::Op::OpFunction:
        {
            [[maybe_unused]] u32 resultType = *std::next(word, 1);
            u32 result = *std::next(word, 2);
            if (entryPointID == ~0)
            {
                ASSERT(false, "must be known");
                return false;
            }

            if ((buildInPositionID == ~0 && buildInPositionStructID == ~0) || !std::get<0>(float32Type))
            {
                ASSERT(false, "must be known");
                return false;
            }

            if (result == entryPointID)
            {
                patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, word);
                lastPatchedWord = word;

                if (!std::get<0>(int32Type))
                {
                    u32 opTypeIntId = boundIDs++;
                    std::get<0>(int32Type) = true;
                    std::get<1>(int32Type).word = getWordInstruction(spv::Op::OpTypeInt, 4);
                    std::get<1>(int32Type).result = opTypeIntId;
                    std::get<1>(int32Type).width = 32;
                    std::get<1>(int32Type).sign = 0;

                    patchedSpirv.push_back(std::get<1>(int32Type).word);
                    patchedSpirv.push_back(std::get<1>(int32Type).result);
                    patchedSpirv.push_back(std::get<1>(int32Type).width);
                    patchedSpirv.push_back(std::get<1>(int32Type).sign);
                }

                if (!std::get<0>(float32TypePointer))
                {
                    u32 opTypeFloatId = boundIDs++;
                    std::get<0>(float32TypePointer) = true;
                    std::get<1>(float32TypePointer).word = getWordInstruction(spv::Op::OpTypePointer, 4);
                    std::get<1>(float32TypePointer).result = opTypeFloatId;
                    std::get<1>(float32TypePointer).storageClass = spv::StorageClass::StorageClassOutput;
                    std::get<1>(float32TypePointer).type = std::get<1>(float32Type).result;

                    patchedSpirv.push_back(std::get<1>(float32TypePointer).word);
                    patchedSpirv.push_back(std::get<1>(float32TypePointer).result);
                    patchedSpirv.push_back(std::get<1>(float32TypePointer).storageClass);
                    patchedSpirv.push_back(std::get<1>(float32TypePointer).type);
                }

                if (!std::get<0>(constantInt0))
                {
                    u32 constantInt0Id = boundIDs++;
                    std::get<0>(constantInt0) = true;
                    std::get<1>(constantInt0).resultType = std::get<1>(int32Type).result;
                    std::get<1>(constantInt0).result = constantInt0Id;

                    patchedSpirv.push_back(std::get<1>(constantInt0).word);
                    patchedSpirv.push_back(std::get<1>(constantInt0).resultType);
                    patchedSpirv.push_back(std::get<1>(constantInt0).result);
                    patchedSpirv.push_back(std::get<1>(constantInt0).value);
                }

                if (!std::get<0>(constantInt1))
                {
                    u32 constantInt1Id = boundIDs++;
                    std::get<0>(constantInt1) = true;
                    std::get<1>(constantInt1).resultType = std::get<1>(int32Type).result;
                    std::get<1>(constantInt1).result = constantInt1Id;

                    patchedSpirv.push_back(std::get<1>(constantInt1).word);
                    patchedSpirv.push_back(std::get<1>(constantInt1).resultType);
                    patchedSpirv.push_back(std::get<1>(constantInt1).result);
                    patchedSpirv.push_back(std::get<1>(constantInt1).value);
                }

                insideMainEntryPointFunction = true;
            }
        }
        break;

        case spv::Op::OpReturn:
        {
            if (insideMainEntryPointFunction)
            {
                patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, word);
                lastPatchedWord = word;

                if (buildInPositionID == ~0 && buildInPositionStructID == ~0)
                {
                    ASSERT(false, "must be known");
                    return false;
                }

                if (!std::get<0>(int32Type) || !std::get<0>(float32Type) || !std::get<0>(float32TypePointer) || !std::get<0>(constantInt0) || !std::get<0>(constantInt1))
                {
                    ASSERT(false, "must be known");
                    return false;
                }

                u32 buildInInsideStruct = (buildInPositionStructID != ~0) ? 1 : 0;
                u32 opAccessChainResultYId = (u32)~0;
                {
                    u32 opAccessChain = getWordInstruction(spv::Op::OpAccessChain, 4 + 1 + buildInInsideStruct);
                    opAccessChainResultYId = boundIDs++;
                    patchedSpirv.push_back(opAccessChain);
                    patchedSpirv.push_back(std::get<1>(float32TypePointer).result);
                    patchedSpirv.push_back(opAccessChainResultYId);
                    if (buildInInsideStruct)
                    {
                        patchedSpirv.push_back(buildInPositionStructID);
                        patchedSpirv.push_back(std::get<1>(constantInt0).result);
                    }
                    else
                    {
                        patchedSpirv.push_back(buildInPositionID);
                    }
                    patchedSpirv.push_back(std::get<1>(constantInt1).result);
                }

                u32 opLoadYId = (u32)~0;
                {
                    u32 opLoad = getWordInstruction(spv::Op::OpLoad, 4);
                    opLoadYId = boundIDs++;
                    patchedSpirv.push_back(opLoad);
                    patchedSpirv.push_back(std::get<1>(float32Type).result);
                    patchedSpirv.push_back(opLoadYId);
                    patchedSpirv.push_back(opAccessChainResultYId);
                }

                u32 opFNegateYId = (u32)~0;
                {
                    u32 opFNegate = getWordInstruction(spv::Op::OpFNegate, 4);
                    opFNegateYId = boundIDs++;
                    patchedSpirv.push_back(opFNegate);
                    patchedSpirv.push_back(std::get<1>(float32Type).result);
                    patchedSpirv.push_back(opFNegateYId);
                    patchedSpirv.push_back(opLoadYId);
                }

                u32 opStore = getWordInstruction(spv::Op::OpStore, 3);
                patchedSpirv.push_back(opStore);
                patchedSpirv.push_back(opAccessChainResultYId);
                patchedSpirv.push_back(opFNegateYId);
            }
        }
        break;

        default:
            break;
        }

        std::advance(word, count);
    }

    patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, spirv.end());
    patchedSpirv[3] = boundIDs;

    if (!insideMainEntryPointFunction) //main funct not found
    {
        return false;
    }

    return true;
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV