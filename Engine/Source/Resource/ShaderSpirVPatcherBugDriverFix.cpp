#include "ShaderSpirVPatcherBugDriverFix.h"
#include "Renderer/Context.h"
#include "Utils/Logger.h"

#ifdef USE_SPIRV
#   include <shaderc/third_party/glslang/SPIRV/spirv.hpp>

namespace v3d
{
namespace resource
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

struct TypeFloat
{
    u32 word = getWordInstruction(spv::Op::OpTypeFloat, 3);
    u32 result = (u32)~0;
    u32 width = sizeof(f32) * 8;
};

struct TypeBool
{
    u32 word = getWordInstruction(spv::Op::OpTypeBool, 2);
    u32 result = (u32)~0;
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

template<u32 Dim>
struct Composite
{
    u32 word = getWordInstruction(spv::Op::OpConstantComposite, 3 + Dim);
    u32 resultType = (u32)~0;
    u32 result = (u32)~0;
    std::array<u32, Dim> constituents;
};


bool PatchDriverBugOptimization::patch(const std::vector<u32>& spirv1, std::vector<u32>& patchedSpirv, u32 flags)
{
    //outFragColor = (color == vec4(1.0)) ? vec4(0.99) : color;
    bool result = false;

    auto word = patchedSpirv.begin();
    u32 boundIDs = *std::next(word, 3);
    word = std::next(word, 5);

    std::vector<u32> locations;
    u32 outputLocation = ~0;
    u32 outputResult = ~0;
    u32 entryPoint = ~0;

    bool declarationBlock = false;

    std::tuple<bool, TypeFloat> floatType(false, {});
    std::tuple<bool, TypeBool> boolType(false, {});
    std::tuple<bool, TypeVector<4>> vectorFloatType(false, {});
    std::tuple<bool, TypeVector<4>> vectorBoolType(false, {});

    std::tuple<bool, Constant<f32>> constantFloat1(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 1.0f });
    std::tuple<bool, Composite<4>> constantCompositeFloat1(false, { getWordInstruction(spv::Op::OpConstantComposite, 7), (u32)~0, (u32)~0, {} });
    std::get<1>(constantCompositeFloat1).constituents.fill((u32)~0);

    std::tuple<bool, Constant<f32>> constantFloat99(false, { getWordInstruction(spv::Op::OpConstant, 3 + 1), (u32)~0, (u32)~0, 0.99f });
    std::tuple<bool, Composite<4>> constantCompositeFloat99(false, { getWordInstruction(spv::Op::OpConstantComposite, 7), (u32)~0, (u32)~0, {} });
    std::get<1>(constantCompositeFloat99).constituents.fill((u32)~0);


    while (word != patchedSpirv.end())
    {
        u32 op = getOpCode(*word);
        u32 count = getCountWords(*word);

        switch (op)
        {
            case spv::Op::OpEntryPoint:
            {
                u32 offset = 0;
                [[maybe_unused]] u32 executionModel = *std::next(word, offset + 1);
                entryPoint = *std::next(word, offset + 2);
                [[maybe_unused]] std::string name = getLiteralString(*std::next(word, offset + 3), offset);

                u32 interfaceCount = count - (offset + 4);
                for (u32 i = 0; i < interfaceCount; ++i)
                {
                    u32 interfaceID = *std::next(word, i + offset + 4);
                    locations.push_back(interfaceID);
                }

                ASSERT(offset < count, "range out");
            }
            break;

            case spv::Op::OpVariable:
            {
                if (locations.empty())
                {
                    break;
                }

                [[maybe_unused]] u32 resultType = *std::next(word, 1);
                u32 resultID = *std::next(word, 2);
                u32 storageClass = *std::next(word, 3);

                if (storageClass == spv::StorageClass::StorageClassOutput)
                {
                    auto found = std::find(locations.cbegin(), locations.cend(), resultID);
                    if (found != locations.cend())
                    {
                        outputLocation = *found;
                        locations.clear();

                        break;
                    }
                }
            }
            break;

            case spv::Op::OpTypeFloat:
            {
                u32 resultID = *std::next(word, 1);
                u32 width = *std::next(word, 2);
                std::get<0>(floatType) = true;
                std::get<1>(floatType).result = resultID;
                ASSERT(std::get<1>(floatType).word == *word, "wrond word");
                ASSERT(std::get<1>(floatType).width == width, "wrond size");
            }
            break;

            case spv::Op::OpTypeBool:
            {
                u32 resultID = *std::next(word, 1);
                std::get<0>(boolType) = true;
                std::get<1>(boolType).result = resultID;
                ASSERT(std::get<1>(boolType).word == *word, "wrond word");
            }
            break;

            case spv::Op::OpTypeVector:
            {
                if (!std::get<0>(floatType))
                {
                    break;
                }

                u32 resultID = *std::next(word, 1);
                u32 componentType = *std::next(word, 2);
                u32 dim = *std::next(word, 3);

                if (std::get<1>(floatType).result == componentType && dim == std::get<1>(vectorFloatType).size)
                {
                    std::get<0>(vectorFloatType) = true;
                    std::get<1>(vectorFloatType).componentType = componentType;
                    std::get<1>(vectorFloatType).result = resultID;
                    ASSERT(std::get<1>(vectorFloatType).word == *word, "wrond word");

                    break;
                }

                if (std::get<1>(boolType).result == componentType && dim == std::get<1>(vectorBoolType).size)
                {
                    std::get<0>(vectorBoolType) = true;
                    std::get<1>(vectorBoolType).componentType = componentType;
                    std::get<1>(vectorBoolType).result = resultID;
                    ASSERT(std::get<1>(vectorBoolType).word == *word, "wrond word");

                    break;
                }
            }
            break;

            case spv::Op::OpConstant:
            {
                if (!std::get<0>(floatType))
                {
                    break;
                }

                u32 resultType = *std::next(word, 1);
                u32 resultID = *std::next(word, 2);

                //1.0f
                if (!std::get<0>(constantFloat1) && std::get<1>(floatType).result == resultType)
                {
                    const f32* value = reinterpret_cast<f32*>(&*std::next(word, 3));
                    if (std::get<1>(constantFloat1).value == *value)
                    {
                        std::get<0>(constantFloat1) = true;
                        std::get<1>(constantFloat1).result = resultID;
                    }
                    std::get<1>(constantFloat1).resultType = resultType;
                    ASSERT(std::get<1>(constantFloat1).word == *word, "wrond word");
                    break;
                }

                //0.99f
                if (!std::get<0>(constantFloat99) && std::get<1>(floatType).result == resultType)
                {
                    const f32* value = reinterpret_cast<f32*>(&*std::next(word, 3));
                    if (std::get<1>(constantFloat99).value == *value)
                    {
                        std::get<0>(constantFloat99) = true;
                        std::get<1>(constantFloat99).result = resultID;
                    }
                    std::get<1>(constantFloat99).resultType = resultType;
                    ASSERT(std::get<1>(constantFloat99).word == *word, "wrond word");
                    break;
                }
            }
            break;

            case spv::Op::OpConstantComposite:
            {
                u32 resultType = *std::next(word, 1);
                if (!std::get<0>(floatType) || !std::get<0>(vectorFloatType) || std::get<1>(vectorFloatType).result != resultType)
                {
                    break;
                }

                u32 resultID = *std::next(word, 2);
                std::array<u32, 4> vec4 = {};
                for (u32 i = 0; i < count - 3; ++i)
                {
                    vec4[i] = *std::next(word, 3 + i);
                }

                //vec4(1.0f)
                if (!std::get<0>(constantCompositeFloat1) && std::get<0>(constantFloat1))
                {
                    std::array<u32, 4> constants = {};
                    constants.fill(std::get<1>(constantFloat1).result);
                    if (constants == vec4)
                    {

                        std::get<0>(constantCompositeFloat1) = true;
                        std::get<1>(constantCompositeFloat1).result = resultID;
                    }
                    std::get<1>(constantCompositeFloat1).resultType = resultType;
                    ASSERT(std::get<1>(constantCompositeFloat1).word == *word, "wrond word");
                    break;
                }

                //vec4(0.99f)
                if (!std::get<0>(constantCompositeFloat99) && std::get<0>(constantFloat99))
                {
                    std::array<u32, 4> constants = {};
                    constants.fill(std::get<1>(constantFloat99).result);
                    if (constants == vec4)
                    {

                        std::get<0>(constantCompositeFloat99) = true;
                        std::get<1>(constantCompositeFloat99).result = resultID;
                    }
                    std::get<1>(constantCompositeFloat99).resultType = resultType;
                    ASSERT(std::get<1>(constantCompositeFloat99).word == *word, "wrond word");
                    break;
                }

            }
            break;

            case spv::Op::OpFunction:
            {
                [[maybe_unused]] u32 resultType = *std::next(word, 1);
                u32 resultID = *std::next(word, 2);
                if (entryPoint != resultID)
                {
                    break;
                }

                std::vector<u32> declarationSubSpirv;
                result = composePatch([&](std::vector<u32>& spirv) -> bool
                    {
                        if (!std::get<0>(floatType))
                        {
                            std::get<0>(floatType) = true;
                            std::get<1>(floatType).result = boundIDs++;

                            spirv.push_back(std::get<1>(floatType).word);
                            spirv.push_back(std::get<1>(floatType).result);
                            spirv.push_back(std::get<1>(floatType).width);
                            ASSERT(getOpCode(std::get<1>(floatType).word) == spv::Op::OpTypeFloat && getCountWords(std::get<1>(floatType).word) == 3, "wrond word");
                        }

                        if (!std::get<0>(vectorFloatType))
                        {
                            std::get<0>(vectorFloatType) = true;
                            std::get<1>(vectorFloatType).result = boundIDs++;
                            std::get<1>(vectorFloatType).componentType = std::get<1>(floatType).result;

                            spirv.push_back(std::get<1>(vectorFloatType).word);
                            spirv.push_back(std::get<1>(vectorFloatType).result);
                            spirv.push_back(std::get<1>(vectorFloatType).componentType);
                            spirv.push_back(std::get<1>(vectorFloatType).size);
                            ASSERT(getOpCode(std::get<1>(vectorFloatType).word) == spv::Op::OpTypeVector && getCountWords(std::get<1>(vectorFloatType).word) == 4, "wrond word");
                            ASSERT(std::get<1>(vectorFloatType).componentType == std::get<1>(floatType).result, "wrond type");
                        }

                        if (!std::get<0>(constantFloat1))
                        {
                            std::get<0>(constantFloat1) = true;
                            std::get<1>(constantFloat1).resultType = std::get<1>(floatType).result;
                            std::get<1>(constantFloat1).result = boundIDs++;

                            spirv.push_back(std::get<1>(constantFloat1).word);
                            spirv.push_back(std::get<1>(constantFloat1).resultType);
                            spirv.push_back(std::get<1>(constantFloat1).result);
                            u32 value = *reinterpret_cast<u32*>(&std::get<1>(constantFloat1).value);
                            spirv.push_back(value);
                            ASSERT(getOpCode(std::get<1>(constantFloat1).word) == spv::Op::OpConstant && getCountWords(std::get<1>(constantFloat1).word) == 3 + 1, "wrond word");
                        }

                        if (!std::get<0>(constantCompositeFloat1))
                        {
                            std::get<0>(constantCompositeFloat1) = true;
                            std::get<1>(constantCompositeFloat1).resultType = std::get<1>(vectorFloatType).result;
                            std::get<1>(constantCompositeFloat1).result = boundIDs++;
                            std::get<1>(constantCompositeFloat1).constituents.fill(std::get<1>(constantFloat1).result);

                            spirv.push_back(std::get<1>(constantCompositeFloat1).word);
                            spirv.push_back(std::get<1>(constantCompositeFloat1).resultType);
                            spirv.push_back(std::get<1>(constantCompositeFloat1).result);
                            for (u32 i = 0; i < std::get<1>(constantCompositeFloat1).constituents.size(); ++i)
                            {
                                spirv.push_back(std::get<1>(constantCompositeFloat1).constituents[1]);
                            }
                            ASSERT(getOpCode(std::get<1>(constantCompositeFloat1).word) == spv::Op::OpConstantComposite && getCountWords(std::get<1>(constantCompositeFloat1).word) == 3 + 4, "wrond word");
                        }

                        if (!std::get<0>(constantFloat99))
                        {
                            std::get<0>(constantFloat99) = true;
                            std::get<1>(constantFloat99).resultType = std::get<1>(floatType).result;
                            std::get<1>(constantFloat99).result = boundIDs++;

                            spirv.push_back(std::get<1>(constantFloat99).word);
                            spirv.push_back(std::get<1>(constantFloat99).resultType);
                            spirv.push_back(std::get<1>(constantFloat99).result);
                            u32 value = *reinterpret_cast<u32*>(&std::get<1>(constantFloat99).value);
                            spirv.push_back(value);
                            ASSERT(getOpCode(std::get<1>(constantFloat99).word) == spv::Op::OpConstant && getCountWords(std::get<1>(constantFloat99).word) == 3 + 1, "wrond word");
                        }

                        if (!std::get<0>(constantCompositeFloat99))
                        {
                            std::get<0>(constantCompositeFloat99) = true;
                            std::get<1>(constantCompositeFloat99).resultType = std::get<1>(vectorFloatType).result;
                            std::get<1>(constantCompositeFloat99).result = boundIDs++;
                            std::get<1>(constantCompositeFloat99).constituents.fill(std::get<1>(constantFloat99).result);

                            spirv.push_back(std::get<1>(constantCompositeFloat99).word);
                            spirv.push_back(std::get<1>(constantCompositeFloat99).resultType);
                            spirv.push_back(std::get<1>(constantCompositeFloat99).result);
                            for (u32 i = 0; i < std::get<1>(constantCompositeFloat99).constituents.size(); ++i)
                            {
                                spirv.push_back(std::get<1>(constantCompositeFloat99).constituents[1]);
                            }
                            ASSERT(getOpCode(std::get<1>(constantCompositeFloat99).word) == spv::Op::OpConstantComposite && getCountWords(std::get<1>(constantCompositeFloat99).word) == 3 + 4, "wrond word");
                        }

                        if (!std::get<0>(boolType))
                        {
                            std::get<0>(boolType) = true;
                            std::get<1>(boolType).result = boundIDs++;

                            spirv.push_back(std::get<1>(boolType).word);
                            spirv.push_back(std::get<1>(boolType).result);
                            ASSERT(getOpCode(std::get<1>(boolType).word) == spv::Op::OpTypeBool && getCountWords(std::get<1>(boolType).word) == 2, "wrond word");
                        }

                        if (!std::get<0>(vectorBoolType))
                        {
                            std::get<0>(vectorBoolType) = true;
                            std::get<1>(vectorBoolType).result = boundIDs++;
                            std::get<1>(vectorBoolType).componentType = std::get<1>(boolType).result;

                            spirv.push_back(std::get<1>(vectorBoolType).word);
                            spirv.push_back(std::get<1>(vectorBoolType).result);
                            spirv.push_back(std::get<1>(vectorBoolType).componentType);
                            spirv.push_back(std::get<1>(vectorBoolType).size);
                            ASSERT(getOpCode(std::get<1>(vectorBoolType).word) == spv::Op::OpTypeVector && getCountWords(std::get<1>(vectorBoolType).word) == 4, "wrond word");
                            ASSERT(std::get<1>(vectorBoolType).componentType == std::get<1>(boolType).result, "wrond type");
                        }

                        return true;
                    }, declarationSubSpirv);

                if (!result)
                {
                    return false;
                }

                if (!declarationSubSpirv.empty())
                {
                    word = patchedSpirv.insert(word, declarationSubSpirv.cbegin(), declarationSubSpirv.cend());
                    std::advance(word, declarationSubSpirv.size());
                }

                declarationBlock = true;
            }
            break;

            case spv::Op::OpLabel:
            {
                [[maybe_unused]] u32 resultID = *std::next(word, 1);
                if (!declarationBlock)
                {
                    ASSERT(false, "somthing is wrong");
                    return false;
                }
            }
            break;

            case spv::Op::OpStore:
            {
                u32 pointer = *std::next(word, 1);
                u32& object = *std::next(word, 2);

                if (outputLocation == pointer)
                {
                    if (!declarationBlock)
                    {
                        return false;
                    }

                    outputResult = object;

                    u32 selectResult = (u32)~0;
                    std::vector<u32> conditionSubSpirv;
                    composePatch([&](std::vector<u32>& spirv) -> bool
                        {
                            /*
                            % 32 = OpFOrdEqual % v4bool % 24 % 29
                            % 33 = OpAll % bool% 32
                            % 37 = OpCompositeConstruct % v4bool % 33 % 33 % 33 % 33
                            % 38 = OpSelect % v4float % 37 % 35 % 24
                            */

                            if (!std::get<0>(vectorBoolType) || !std::get<0>(constantCompositeFloat1))
                            {
                                return false;
                            }

                            struct FOrdEqual
                            {
                                u32 word = getWordInstruction(spv::Op::OpFOrdEqual, 5);
                                u32 resultType = (u32)~0;
                                u32 result = (u32)~0;
                                u32 operand1 = (u32)~0;
                                u32 operand2 = (u32)~0;
                            };
                            FOrdEqual fOrdEqual{ getWordInstruction(spv::Op::OpFOrdEqual, 5), std::get<1>(vectorBoolType).result, boundIDs++, outputResult, std::get<1>(constantCompositeFloat1).result };

                            spirv.push_back(fOrdEqual.word);
                            spirv.push_back(fOrdEqual.resultType);
                            spirv.push_back(fOrdEqual.result);
                            spirv.push_back(fOrdEqual.operand1);
                            spirv.push_back(fOrdEqual.operand2);


                            if (!std::get<0>(boolType))
                            {
                                return false;
                            }

                            struct All
                            {
                                u32 word = getWordInstruction(spv::Op::OpAll, 4);
                                u32 resultType = (u32)~0;
                                u32 result = (u32)~0;
                                u32 vector = (u32)~0;
                            };
                            All all{ getWordInstruction(spv::Op::OpAll, 4), std::get<1>(boolType).result, boundIDs++, fOrdEqual.result };

                            spirv.push_back(all.word);
                            spirv.push_back(all.resultType);
                            spirv.push_back(all.result);
                            spirv.push_back(all.vector);

                            Composite<4> compositeConstruct{ getWordInstruction(spv::Op::OpCompositeConstruct, 3 + 4), std::get<1>(vectorBoolType).result, boundIDs++, {} };
                            compositeConstruct.constituents.fill(all.result);

                            spirv.push_back(compositeConstruct.word);
                            spirv.push_back(compositeConstruct.resultType);
                            spirv.push_back(compositeConstruct.result);
                            for (auto& elem : compositeConstruct.constituents)
                            {
                                spirv.push_back(elem);
                            }

                            if (!std::get<0>(vectorFloatType) || !std::get<0>(constantCompositeFloat99))
                            {
                                return false;
                            }

                            struct Select
                            {
                                u32 word = getWordInstruction(spv::Op::OpSelect, 6);
                                u32 resultType = (u32)~0;
                                u32 result = (u32)~0;
                                u32 condition = (u32)~0;
                                u32 oject1 = (u32)~0;
                                u32 oject2 = (u32)~0;
                            };
                            Select select{ getWordInstruction(spv::Op::OpSelect, 6), std::get<1>(vectorFloatType).result, boundIDs++, compositeConstruct.result, std::get<1>(constantCompositeFloat99).result, outputResult };
                            selectResult = select.result;

                            spirv.push_back(select.word);
                            spirv.push_back(select.resultType);
                            spirv.push_back(select.result);
                            spirv.push_back(select.condition);
                            spirv.push_back(select.oject1);
                            spirv.push_back(select.oject2);

                            return true;

                        }, conditionSubSpirv);

                    if (!conditionSubSpirv.empty())
                    {
                        object = selectResult;

                        word = patchedSpirv.insert(word, conditionSubSpirv.cbegin(), conditionSubSpirv.cend());
                        std::advance(word, conditionSubSpirv.size());

                        u32& remapBoundID = *(patchedSpirv.begin() + 3);
                        remapBoundID = boundIDs;

                        return true;
                    }


                }
            }
            break;

        default:
            break;
        }

        std::advance(word, count);
    }

    return result;
}

bool PatchDriverBugOptimization::composePatch(std::function<bool(std::vector<u32>& subSpirv)> func, std::vector<u32>& spirv)
{
    return func(spirv);
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV