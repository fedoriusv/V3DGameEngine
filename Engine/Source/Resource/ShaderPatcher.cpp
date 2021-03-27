#include "ShaderPatcher.h"
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
    u32 width = sizeof(u32) * 8;
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

PatchDriverBugOptimization::PatchDriverBugOptimization(u32 flags) noexcept
    : m_flags(flags)
{
}

bool PatchDriverBugOptimization::patch(std::vector<u32>& spirv, u32 flags)
{
    //outFragColor = (color == vec4(1.0)) ? vec4(0.99) : color;
    bool result = false;

    auto word = spirv.begin();
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


    while (word != spirv.end())
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
                    word = spirv.insert(word, declarationSubSpirv.cbegin(), declarationSubSpirv.cend());
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

                        word = spirv.insert(word, conditionSubSpirv.cbegin(), conditionSubSpirv.cend());
                        std::advance(word, conditionSubSpirv.size());

                        u32& remapBoundID = *(spirv.begin() + 3);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

bool PatchRemoveUnusedLocations::collectDataFromSpirv(const std::vector<u32>& spirv)
{
    if (spirv.empty())
    {
        return false;
    }

    auto word = spirv.begin();

    u32 magicNumber = *word;
    if (magicNumber != 0x07230203)
    {
        return false;
    }

    u32 entryPoint = ~0;
    u32 executionModel = ~0;

    word = std::next(word, 5);
    while (word != spirv.end())
    {
        u32 op = getOpCode(*word);
        u32 count = getCountWords(*word);

        switch (op)
        {
        case spv::Op::OpEntryPoint:
        {
            executionModel = *std::next(word, 1);
            entryPoint = *std::next(word, 2);

            u32 offset = 0;
            [[maybe_unused]] std::string name = getLiteralString(*std::next(word, 3), offset);

            u32 interfaceCount = count - (offset + 4);
            for (u32 i = 0; i < interfaceCount; ++i)
            {
                u32 interfaceID = *std::next(word, i + offset + 4);
                m_locations.push_back({ interfaceID, (u32)~0, (u32)~0, executionModel, (u32)~0, (u32)~0 });
            }
        }
        break;

        case spv::Op::OpVariable:
        {
            if (m_locations.empty())
            {
                break;
            }

            u32 resultType = *std::next(word, 1);
            u32 resultID = *std::next(word, 2);
            u32 storageClass = *std::next(word, 3);

            auto found = std::find_if(m_locations.begin(), m_locations.end(), [resultID, executionModel](const Location& location)->bool
                {
                    return location.executionModel == executionModel && location.bindID == resultID;
                });

            if (found != m_locations.end())
            {
                found->typePointerID = resultType;
                found->storageClass = storageClass;
            }
        }
        break;

        case spv::Op::OpDecorate:
        {
            if (m_locations.empty())
            {
                break;
            }

            u32 targetID = *std::next(word, 1);
            u32 decoration = *std::next(word, 2);
            if (decoration == spv::Decoration::DecorationLocation)
            {
                u32 location = *std::next(word, 3);
                auto found = std::find_if(m_locations.begin(), m_locations.end(), [targetID, executionModel](const Location& location)->bool
                    {
                        return location.executionModel == executionModel && location.bindID == targetID;
                    });

                if (found != m_locations.end())
                {
                    found->location = location;
                }
            }
        }
        break;

        case spv::Op::OpTypeFloat:
        {
            u32 resultID = *std::next(word, 1);
            [[maybe_unused]] u32 width = *std::next(word, 2);

            for (auto& vector : m_vector)
            {
                if (vector.typeID == ~0)
                {
                    vector.typeID = resultID;
                    vector.type = spv::Op::OpTypeVector;
                }
            }
        }
        break;

        case spv::Op::OpTypeVector:
        {
            u32 resultID = *std::next(word, 1);
            u32 componentType = *std::next(word, 2);
            u32 count = *std::next(word, 3);

            ASSERT(count <= 4, "wrong vector size");
            u32 index = count - 1;
            if (m_vector[index].type == spv::Op::OpTypeVector && componentType == m_vector[index].typeID)
            {
                if (m_vector[index].bindID == ~0)
                {
                    m_vector[index].bindID = resultID;
                    m_vector[index].components = count;
                }
            }

        }
        break;

        default:
            break;
        }

        std::advance(word, count);
    }

    return true;
}

bool PatchRemoveUnusedLocations::patch(std::vector<u32>& spirv, u32 flags)
{
    //[ UNASSIGNED-CoreValidation-Shader-OutputNotConsumed ] Object: 0x0 (Type = 15) | vertex shader writes to output location 0.0 which is not consumed by fragment shader

    std::vector<Location> unusedVertexLocations;
    PatchRemoveUnusedLocations::extractUnusedLocations(unusedVertexLocations);

    if (unusedVertexLocations.empty())
    {
        return true;
    }

    LOG_WARNING("PatchRemoveUnusedLocations::patch: Found %u unused attributes", (u32)unusedVertexLocations.size())
    for (auto& loc : unusedVertexLocations)
    {
        LOG_WARNING("PatchRemoveUnusedLocations::patch: Present in a Vertex shader location %u, but is not used in a Fragment shader", loc.location);
    }

    std::vector<u32> patchedSpirv;

    auto word = spirv.begin();
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
            patchedSpirv.insert(patchedSpirv.end(), spirv.begin(), word);

            u32 executionModel = *std::next(word, 1);
            u32 entryPoint = *std::next(word, 2);
            if (executionModel != spv::ExecutionModel::ExecutionModelVertex)
            {
                ASSERT(false, "wrong shader model");
                return false;
            }
            u32 offset = 0;
            [[maybe_unused]] std::string name = getLiteralString(*std::next(word, 3), offset);

            s32 newCount = count - static_cast<s32>(unusedVertexLocations.size());
            ASSERT(newCount > 0, "wrong");
            u32 newOp = getWordInstruction(op, (u32)newCount);
            patchedSpirv.push_back(newOp);
            patchedSpirv.push_back(executionModel);
            patchedSpirv.push_back(entryPoint);
            patchedSpirv.insert(patchedSpirv.end(), std::next(word, 3), std::next(word, 3) + (offset + 1));

            u32 interfaceCount = count - (offset + 4);
            u32 interfaceAdded = 0;
            for (u32 i = 0; i < interfaceCount; ++i)
            {
                u32 interfaceID = *std::next(word, i + (offset + 4));
                auto found = std::find_if(unusedVertexLocations.begin(), unusedVertexLocations.end(), [interfaceID](const Location& location) -> bool
                    {
                        return location.bindID == interfaceID;
                    });

                if (found == unusedVertexLocations.end())
                {
                    patchedSpirv.push_back(interfaceID);
                    ++interfaceAdded;
                }
            }
            ASSERT(interfaceAdded == interfaceCount - unusedVertexLocations.size(), "must be new size");
            lastPatchedWord = std::next(word, count);
        }
        break;

        case spv::Op::OpDecorate:
        {
            u32 targetID = *std::next(word, 1);
            u32 decoration = *std::next(word, 2);
            if (decoration == spv::Decoration::DecorationLocation)
            {
                [[maybe_unused]] u32 location = *std::next(word, 3);
                auto found = std::find_if(unusedVertexLocations.begin(), unusedVertexLocations.end(), [targetID](const Location& location)->bool
                    {
                        return location.bindID == targetID;
                    });

                if (found != unusedVertexLocations.end())
                {
                    //just skip
                    patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, word);
                    lastPatchedWord = std::next(word, count);
                }
            }
        }
        break;

        case spv::Op::OpVariable:
        {
            [[maybe_unused]] u32 resultType = *std::next(word, 1);
            u32 resultID = *std::next(word, 2);
            u32 storageClass = *std::next(word, 3);

            if (storageClass != spv::StorageClass::StorageClassOutput)
            {
                break;
            }

            auto found = std::find_if(unusedVertexLocations.begin(), unusedVertexLocations.end(), [resultID](const Location& location) -> bool
                {
                    return location.bindID == resultID;
                });

            if (found == unusedVertexLocations.end())
            {
                break;
            }
            
            u32 vectorTypeID = found->typeID;
            if (vectorTypeID == ~0)
            {
                ASSERT(false, "type Id not found");
                return false;
            }
            auto foundVectorType = std::find_if(m_vector.begin(), m_vector.end(), [vectorTypeID](const Type& location) -> bool
                {
                    return location.bindID == vectorTypeID;
                });

            if (foundVectorType == m_vector.end())
            {
                ASSERT(false, "type Id not found");
                return false;
            }

            patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, word);


            u32 newBindId = *std::next(patchedSpirv.begin(), 3);
            patchedSpirv[3] = newBindId + 1;
            //OpTypePointer 
            {
                u32 newOp = getWordInstruction(spv::Op::OpTypePointer, 4);
                patchedSpirv.push_back(newOp);
                patchedSpirv.push_back(newBindId);
                patchedSpirv.push_back(spv::StorageClass::StorageClassPrivate);
                patchedSpirv.push_back(vectorTypeID);
            }

            //OpVariable
            {
                u32 newOp = getWordInstruction(spv::Op::OpVariable, 4);
                ASSERT(newOp == *word, "must be same");
                patchedSpirv.push_back(newOp);
                patchedSpirv.push_back(newBindId);
                patchedSpirv.push_back(resultID);
                patchedSpirv.push_back(spv::StorageClass::StorageClassPrivate);
            }

            lastPatchedWord = std::next(word, count);
        }
        break;

        case spv::Op::OpTypePointer:
        {
            u32 resultID = *std::next(word, 1);
            u32 storageClass = *std::next(word, 2);
            u32 typeID = *std::next(word, 3);

            if (storageClass != spv::StorageClass::StorageClassOutput)
            {
                break;
            }

            for (auto& location : unusedVertexLocations)
            {
                if (location.typePointerID == resultID && location.typeID == ~0)
                {
                    location.typeID = typeID;
                }
            }
        }
        break;

        default:
            break;
        }

        std::advance(word, count);
    }

    patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, spirv.end());
    std::swap(spirv, patchedSpirv);

    LOG_WARNING("PatchRemoveUnusedLocations::patch: Shader has been patched");
    return true;
}

void PatchRemoveUnusedLocations::extractUnusedLocations(std::vector<Location>& unusedLocations)
{
    unusedLocations.clear();
    for (auto& locationVertexOut : m_locations)
    {
        if (locationVertexOut.executionModel == spv::ExecutionModel::ExecutionModelVertex && locationVertexOut.storageClass == spv::StorageClass::StorageClassOutput && locationVertexOut.location != ~0)
        {
            bool notFound = true;
            for (auto& locationFragmentIn : m_locations)
            {
                if (locationFragmentIn.executionModel == spv::ExecutionModel::ExecutionModelFragment && locationFragmentIn.storageClass == spv::StorageClass::StorageClassInput && locationFragmentIn.location != ~0)
                {
                    if (locationVertexOut.location == locationFragmentIn.location)
                    {
                        notFound = false;
                        break;
                    }
                }
            }

            if (notFound)
            {
                unusedLocations.push_back(locationVertexOut);
            }
        }
    }
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV