#include "ShaderSpirVPatcherRemoveUnusedLocations.h"
#include "Renderer/Core/Context.h"
#include "Utils/Logger.h"

#ifdef USE_SPIRV
#   include <shaderc/third_party/glslang/SPIRV/spirv.hpp>

namespace v3d
{
namespace resource
{

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

bool PatchRemoveUnusedLocations::patch(const std::vector<u32>& spirv, std::vector<u32>& patchedSpirv, u32 flags)
{
    //[ UNASSIGNED-CoreValidation-Shader-OutputNotConsumed ] Object: 0x0 (Type = 15) | vertex shader writes to output location 0.0 which is not consumed by fragment shader
    LOG_DEBUG("PatchRemoveUnusedLocations::patch");

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

    patchedSpirv.clear();
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

        case spv::Op::OpName:
        {
            u32 targetID = *std::next(word, 1);
            auto found = std::find_if(unusedVertexLocations.begin(), unusedVertexLocations.end(), [targetID](const Location& location) -> bool
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

        /*case spv::Op::OpVariable:
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
        break;*/

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

        case spv::Op::OpStore:
        {
            u32 pointerID = *std::next(word, 1);
            auto found = std::find_if(unusedVertexLocations.begin(), unusedVertexLocations.end(), [pointerID](const Location& location) -> bool
                {
                    return location.bindID == pointerID;
                });

            if (found != unusedVertexLocations.end())
            {
                //just skip
                patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, word);
                lastPatchedWord = std::next(word, count);
            }
        }
        break;

        default:
            break;
        }

        std::advance(word, count);
    }

    patchedSpirv.insert(patchedSpirv.end(), lastPatchedWord, spirv.end());
    
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