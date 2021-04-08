#include "ShaderSpirVPatcherVertexTransform.h"
#include "Renderer/Context.h"
#include "Utils/Logger.h"

#ifdef USE_SPIRV
#   include <shaderc/third_party/glslang/SPIRV/spirv.hpp>

namespace v3d
{
namespace resource
{

struct TypeInt
{
    u32 word = getWordInstruction(spv::Op::OpTypeInt, 4);
    u32 result = (u32)~0;
    u32 width = sizeof(s32) * 8;
    u32 sign = 1;
};

struct TypeFloat
{
    u32 word = getWordInstruction(spv::Op::OpTypeFloat, 3);
    u32 result = (u32)~0;
    u32 width = sizeof(f32) * 8;
};

template<u32 Dim>
struct TypeVector
{
    u32 word = getWordInstruction(spv::Op::OpTypeVector, 4);
    u32 result = (u32)~0;
    u32 componentType = (u32)~0;
    u32 size = Dim;
};

template<typename Type>
struct Constant
{
    u32 word = getWordInstruction(spv::Op::OpConstant, 3 + sizeof(Type) / sizeof(u32));
    u32 resultType = (u32)~0;
    u32 result = (u32)~0;
    Type value = Type(0);
};

PatchVertexTransform::PatchVertexTransform(f32 angle) noexcept
    : m_angle(angle)
{
}

bool PatchVertexTransform::patch(std::vector<u32>& spirv, u32 flags)
{
    //gl_Position = vec4(gl_Position.x * cos(rad) - gl_Position.y * sin(rad), gl_Position.x * sin(rad) + gl_Position.y * cos(rad), gl_Position.z, gl_Position.w);

    if (spirv.empty())
    {
        return false;
    }

    std::tuple<bool, TypeFloat> float32Type(false, {});
    std::tuple<bool, TypeInt> int32Type(false, {});
    std::tuple<bool, TypeVector<4>> vectorFloat32Type(false, {});

    std::array<std::tuple<bool, Constant<s32>>, 4> constantIntOp =
    {
        std::tuple<bool, Constant<s32>>(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 0 }),
        std::tuple<bool, Constant<s32>>(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 1 }),
        std::tuple<bool, Constant<s32>>(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 2 }),
        std::tuple<bool, Constant<s32>>(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 3 }),
    };

    const f32 constantCos = std::cos(m_angle * core::k_degToRad);
    std::tuple<bool, Constant<f32>> constantCosOp(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, constantCos });

    const f32 constantSin = std::sin(m_angle * core::k_degToRad);
    std::tuple<bool, Constant<f32>> constantSinOp(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, constantSin });

    u32 buildInPositionID = (u32)~0;
    u32 entryPointID = (u32)~0;

    u32 float32PointerTypeID = (u32)~0;

    bool insideEntryPointFunction = false;

    std::vector<u32> patchedSpirv;

    auto word = spirv.begin();
    auto lastPatchedWord = word;

    u32 boundIDs = *std::next(word, 3);
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
            if (decoration == spv::Decoration::DecorationBuiltIn)
            {
                u32 builtIn = *std::next(word, 3);
                if (builtIn == spv::BuiltIn::BuiltInPosition)
                {
                    buildInPositionID = target;
                }
            }
        }
        break;

        case spv::Op::OpMemberDecorate:
        {
            /*if (buildInPositionID != ~0)
            {
                break;
            }

            u32 structType = *std::next(word, 1);
            u32 decoration = *std::next(word, 2);
            if (decoration == spv::Decoration::DecorationBuiltIn)
            {
                u32 builtIn = *std::next(word, 3);
                if (builtIn == spv::BuiltIn::BuiltInPosition)
                {
                    buildInPositionID = structType;
                }
            }*/
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

        case spv::Op::OpTypeVector:
        {
            u32 result = *std::next(word, 1);
            u32 componentType = *std::next(word, 2);
            u32 componentCount = *std::next(word, 3);
            if (!std::get<0>(vectorFloat32Type))
            {
                if (!std::get<0>(float32Type))
                {
                    ASSERT(false, "must be known");
                    return false;
                }

                if (componentType == std::get<1>(float32Type).result && componentCount == 4)
                {
                    std::get<0>(vectorFloat32Type) = true;
                    std::get<1>(vectorFloat32Type).result = result;
                    std::get<1>(vectorFloat32Type).componentType = componentType;
                    std::get<1>(vectorFloat32Type).size = componentCount;
                }
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

            if (result == entryPointID)
            {
                patchedSpirv.insert(patchedSpirv.end(), spirv.begin(), word);
                lastPatchedWord = word;

                if (!std::get<0>(float32Type) || !std::get<0>(int32Type) || !std::get<0>(vectorFloat32Type))
                {
                    ASSERT(false, "must be known");
                    return false;
                }

                {
                    u32 opTypePointer = getWordInstruction(spv::Op::OpTypePointer, 4);
                    float32PointerTypeID = boundIDs++;
                    patchedSpirv.push_back(opTypePointer);
                    patchedSpirv.push_back(float32PointerTypeID);
                    patchedSpirv.push_back(spv::StorageClass::StorageClassOutput);
                    patchedSpirv.push_back(std::get<1>(float32Type).result);
                }

                {
                    std::get<0>(constantCosOp) = true;
                    std::get<1>(constantCosOp).resultType = std::get<1>(float32Type).result;
                    std::get<1>(constantCosOp).result = boundIDs++;

                    patchedSpirv.push_back(std::get<1>(constantCosOp).word);
                    patchedSpirv.push_back(std::get<1>(constantCosOp).resultType);
                    patchedSpirv.push_back(std::get<1>(constantCosOp).result);
                    u32 value = *reinterpret_cast<u32*>(&std::get<1>(constantCosOp).value);
                    patchedSpirv.push_back(value);
                    ASSERT(getOpCode(std::get<1>(constantCosOp).word) == spv::Op::OpConstant && getCountWords(std::get<1>(constantCosOp).word) == 3 + 1, "wrond word");
                }

                {
                    std::get<0>(constantSinOp) = true;
                    std::get<1>(constantSinOp).resultType = std::get<1>(float32Type).result;
                    std::get<1>(constantSinOp).result = boundIDs++;

                    patchedSpirv.push_back(std::get<1>(constantSinOp).word);
                    patchedSpirv.push_back(std::get<1>(constantSinOp).resultType);
                    patchedSpirv.push_back(std::get<1>(constantSinOp).result);
                    u32 value = *reinterpret_cast<u32*>(&std::get<1>(constantSinOp).value);
                    patchedSpirv.push_back(value);
                    ASSERT(getOpCode(std::get<1>(constantSinOp).word) == spv::Op::OpConstant && getCountWords(std::get<1>(constantSinOp).word) == 3 + 1, "wrond word");
                }

                for (auto& constantInt : constantIntOp)
                {
                    std::get<0>(constantInt) = true;
                    std::get<1>(constantInt).resultType = std::get<1>(int32Type).result;
                    std::get<1>(constantInt).result = boundIDs++;

                    patchedSpirv.push_back(std::get<1>(constantInt).word);
                    patchedSpirv.push_back(std::get<1>(constantInt).resultType);
                    patchedSpirv.push_back(std::get<1>(constantInt).result);
                    u32 value = *reinterpret_cast<u32*>(&std::get<1>(constantInt).value);
                    patchedSpirv.push_back(value);
                    ASSERT(getOpCode(std::get<1>(constantInt).word) == spv::Op::OpConstant && getCountWords(std::get<1>(constantInt).word) == 3 + 1, "wrond word");
                }

                insideEntryPointFunction = true;
            }
        }
        break;

        case spv::Op::OpReturn:
        {
            if (insideEntryPointFunction)
            {
                patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, word);
                lastPatchedWord = word;

                if (buildInPositionID == ~0 || float32PointerTypeID == ~0 || !std::get<0>(float32Type) || !std::get<0>(int32Type) || !std::get<0>(vectorFloat32Type))
                {
                    ASSERT(false, "must be known");
                    return false;
                }

                if (!std::get<0>(constantCosOp) || !std::get<0>(constantSinOp))
                {
                    ASSERT(false, "must be added");
                    return false;
                }

                u32 opFSubResultId_X = (u32)~0;
                {
                    u32 opFMulResultId0 = (u32)~0;
                    //gl_Position.x * cos(rad)
                    {
                        u32 opAccessChain = getWordInstruction(spv::Op::OpAccessChain, 4 + 1);
                        u32 opAccessChainResultId = boundIDs++;
                        patchedSpirv.push_back(opAccessChain);
                        patchedSpirv.push_back(float32PointerTypeID);
                        patchedSpirv.push_back(opAccessChainResultId);
                        patchedSpirv.push_back(buildInPositionID);
                        patchedSpirv.push_back(std::get<1>(constantIntOp[0]).result);

                        u32 opLoad = getWordInstruction(spv::Op::OpLoad, 4);
                        u32 opLoadResultId = boundIDs++;
                        patchedSpirv.push_back(opLoad);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(opAccessChainResultId);

                        u32 opFMul = getWordInstruction(spv::Op::OpFMul, 5);
                        opFMulResultId0 = boundIDs++;
                        patchedSpirv.push_back(opFMul);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opFMulResultId0);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(std::get<1>(constantCosOp).result);
                    }

                    u32 opFMulResultId1 = (u32)~0;
                    //gl_Position.y * sin(rad)
                    {
                        u32 opAccessChain = getWordInstruction(spv::Op::OpAccessChain, 4 + 1);
                        u32 opAccessChainResultId = boundIDs++;
                        patchedSpirv.push_back(opAccessChain);
                        patchedSpirv.push_back(float32PointerTypeID);
                        patchedSpirv.push_back(opAccessChainResultId);
                        patchedSpirv.push_back(buildInPositionID);
                        patchedSpirv.push_back(std::get<1>(constantIntOp[1]).result);

                        u32 opLoad = getWordInstruction(spv::Op::OpLoad, 4);
                        u32 opLoadResultId = boundIDs++;
                        patchedSpirv.push_back(opLoad);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(opAccessChainResultId);

                        u32 opFMul = getWordInstruction(spv::Op::OpFMul, 5);
                        opFMulResultId1 = boundIDs++;
                        patchedSpirv.push_back(opFMul);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opFMulResultId1);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(std::get<1>(constantSinOp).result);
                    }

                    //gl_Position.x* cos(rad) - gl_Position.y * sin(rad)
                    {
                        u32 opFSub = getWordInstruction(spv::Op::OpFSub, 5);
                        opFSubResultId_X = boundIDs++;
                        patchedSpirv.push_back(opFSub);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opFSubResultId_X);
                        patchedSpirv.push_back(opFMulResultId0);
                        patchedSpirv.push_back(opFMulResultId1);
                    }
                }

                u32 opFAddResultId_Y = (u32)~0;
                {
                    u32 opFMulResultId0 = (u32)~0;
                    //gl_Position.x * sin(rad)
                    {
                        u32 opAccessChain = getWordInstruction(spv::Op::OpAccessChain, 4 + 1);
                        u32 opAccessChainResultId = boundIDs++;
                        patchedSpirv.push_back(opAccessChain);
                        patchedSpirv.push_back(float32PointerTypeID);
                        patchedSpirv.push_back(opAccessChainResultId);
                        patchedSpirv.push_back(buildInPositionID);
                        patchedSpirv.push_back(std::get<1>(constantIntOp[0]).result);

                        u32 opLoad = getWordInstruction(spv::Op::OpLoad, 4);
                        u32 opLoadResultId = boundIDs++;
                        patchedSpirv.push_back(opLoad);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(opAccessChainResultId);

                        u32 opFMul = getWordInstruction(spv::Op::OpFMul, 5);
                        opFMulResultId0 = boundIDs++;
                        patchedSpirv.push_back(opFMul);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opFMulResultId0);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(std::get<1>(constantSinOp).result);
                    }

                    u32 opFMulResultId1 = (u32)~0;
                    //gl_Position.y * cos(rad)
                    {
                        u32 opAccessChain = getWordInstruction(spv::Op::OpAccessChain, 4 + 1);
                        u32 opAccessChainResultId = boundIDs++;
                        patchedSpirv.push_back(opAccessChain);
                        patchedSpirv.push_back(float32PointerTypeID);
                        patchedSpirv.push_back(opAccessChainResultId);
                        patchedSpirv.push_back(buildInPositionID);
                        patchedSpirv.push_back(std::get<1>(constantIntOp[1]).result);

                        u32 opLoad = getWordInstruction(spv::Op::OpLoad, 4);
                        u32 opLoadResultId = boundIDs++;
                        patchedSpirv.push_back(opLoad);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(opAccessChainResultId);

                        u32 opFMul = getWordInstruction(spv::Op::OpFMul, 5);
                        opFMulResultId1 = boundIDs++;
                        patchedSpirv.push_back(opFMul);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opFMulResultId1);
                        patchedSpirv.push_back(opLoadResultId);
                        patchedSpirv.push_back(std::get<1>(constantCosOp).result);
                    }

                    //gl_Position.x * sin(rad) + gl_Position.y * cos(rad)
                    {
                        u32 opFAdd = getWordInstruction(spv::Op::OpFAdd, 5);
                        opFAddResultId_Y = boundIDs++;
                        patchedSpirv.push_back(opFAdd);
                        patchedSpirv.push_back(std::get<1>(float32Type).result);
                        patchedSpirv.push_back(opFAddResultId_Y);
                        patchedSpirv.push_back(opFMulResultId0);
                        patchedSpirv.push_back(opFMulResultId1);
                    }
                }

                u32 opLoadResultId_Z = (u32)~0;
                //gl_Position.z
                {
                    u32 opAccessChain = getWordInstruction(spv::Op::OpAccessChain, 4 + 1);
                    u32 opAccessChainResultId = boundIDs++;
                    patchedSpirv.push_back(opAccessChain);
                    patchedSpirv.push_back(float32PointerTypeID);
                    patchedSpirv.push_back(opAccessChainResultId);
                    patchedSpirv.push_back(buildInPositionID);
                    patchedSpirv.push_back(std::get<1>(constantIntOp[2]).result);

                    u32 opLoad = getWordInstruction(spv::Op::OpLoad, 4);
                    opLoadResultId_Z = boundIDs++;
                    patchedSpirv.push_back(opLoad);
                    patchedSpirv.push_back(std::get<1>(float32Type).result);
                    patchedSpirv.push_back(opLoadResultId_Z);
                    patchedSpirv.push_back(opAccessChainResultId);
                }

                u32 opLoadResultId_W = (u32)~0;
                //gl_Position.w
                {
                    u32 opAccessChain = getWordInstruction(spv::Op::OpAccessChain, 4 + 1);
                    u32 opAccessChainResultId = boundIDs++;
                    patchedSpirv.push_back(opAccessChain);
                    patchedSpirv.push_back(float32PointerTypeID);
                    patchedSpirv.push_back(opAccessChainResultId);
                    patchedSpirv.push_back(buildInPositionID);
                    patchedSpirv.push_back(std::get<1>(constantIntOp[3]).result);

                    u32 opLoad = getWordInstruction(spv::Op::OpLoad, 4);
                    opLoadResultId_W = boundIDs++;
                    patchedSpirv.push_back(opLoad);
                    patchedSpirv.push_back(std::get<1>(float32Type).result);
                    patchedSpirv.push_back(opLoadResultId_W);
                    patchedSpirv.push_back(opAccessChainResultId);
                }

                ASSERT(opFSubResultId_X != ~0 && opFAddResultId_Y != ~0 && opLoadResultId_Z != ~0 && opLoadResultId_W != ~0, "must be setted");
                u32 opCompositeConstruct = getWordInstruction(spv::Op::OpCompositeConstruct, 3 + 4);
                u32 opCompositeConstructResultId = boundIDs++;
                patchedSpirv.push_back(opCompositeConstruct);
                patchedSpirv.push_back(std::get<1>(vectorFloat32Type).result);
                patchedSpirv.push_back(opCompositeConstructResultId);
                patchedSpirv.push_back(opFSubResultId_X);
                patchedSpirv.push_back(opFAddResultId_Y);
                patchedSpirv.push_back(opLoadResultId_Z);
                patchedSpirv.push_back(opLoadResultId_W);

                u32 opStore = getWordInstruction(spv::Op::OpStore, 3);
                u32 opStoreResultId = boundIDs++;
                patchedSpirv.push_back(opStore);
                patchedSpirv.push_back(buildInPositionID);
                patchedSpirv.push_back(opCompositeConstructResultId);
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

    std::swap(spirv, patchedSpirv);

    return true;
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV