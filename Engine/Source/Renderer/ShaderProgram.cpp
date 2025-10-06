#include "ShaderProgram.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

ShaderProgram::ShaderProgram(Device* device, const VertexShader* vertex, const FragmentShader* fragment) noexcept
    : m_device(device)
{
    m_shaders.fill(nullptr);
    memset(m_resourceSlots, k_invalidBinding, sizeof(m_resourceSlots));

    ASSERT(vertex && fragment, "must be valid");
    m_shaders[toEnumType(ShaderType::Vertex)] = vertex;
    m_shaders[toEnumType(ShaderType::Fragment)] = fragment;
    m_shaderMask = 0b011;

    u32 slotIndex[k_maxDescriptorSetCount] = {};
    auto forEach = [this, &slotIndex](const auto& obj) -> void
        {
            auto inserted = m_resourceBindings.emplace(obj._name, obj._binding);
            if (inserted.second)
            {
                m_resourceSlots[obj._set][obj._binding] = slotIndex[obj._set];
                ++slotIndex[obj._set];
            }
        };


    for (u32 type = toEnumType(ShaderType::Vertex); type <= toEnumType(ShaderType::Fragment); ++type)
    {
        const auto& mappedResources = m_shaders[type]->getMappingResources();
        std::for_each(mappedResources._uniformBuffers.begin(), mappedResources._uniformBuffers.end(), forEach);
        std::for_each(mappedResources._sampledImages.begin(), mappedResources._sampledImages.end(), forEach);
        std::for_each(mappedResources._samplers.begin(), mappedResources._samplers.end(), forEach);
        std::for_each(mappedResources._images.begin(), mappedResources._images.end(), forEach);
        std::for_each(mappedResources._storageImages.begin(), mappedResources._storageImages.end(), forEach);
        std::for_each(mappedResources._storageBuffers.begin(), mappedResources._storageBuffers.end(), forEach);
    }
}

ShaderProgram::ShaderProgram(Device* device, const ComputeShader* compute) noexcept
    : m_device(device)
{
    m_shaders.fill(nullptr);
    memset(m_resourceSlots, k_invalidBinding, sizeof(m_resourceSlots));

    ASSERT(compute, "must be valid");
    m_shaders[toEnumType(ShaderType::Compute)] = compute;
    m_shaderMask = 0b100;

    u32 slotIndex[k_maxDescriptorSetCount] = {};
    auto forEach = [this, &slotIndex](const auto& obj) -> void
        {
            auto inserted = m_resourceBindings.emplace(obj._name, obj._binding);
            if (inserted.second)
            {
                m_resourceSlots[obj._set][obj._binding] = slotIndex[obj._set];
                ++slotIndex[obj._binding];
            }
        };

    const auto& mappedResources = m_shaders[toEnumType(ShaderType::Compute)]->getMappingResources();
    std::for_each(mappedResources._uniformBuffers.begin(), mappedResources._uniformBuffers.end(), forEach);
    std::for_each(mappedResources._sampledImages.begin(), mappedResources._sampledImages.end(), forEach);
    std::for_each(mappedResources._samplers.begin(), mappedResources._samplers.end(), forEach);
    std::for_each(mappedResources._images.begin(), mappedResources._images.end(), forEach);
    std::for_each(mappedResources._storageImages.begin(), mappedResources._storageImages.end(), forEach);
    std::for_each(mappedResources._storageBuffers.begin(), mappedResources._storageBuffers.end(), forEach);
}

} //renderer
} //v3d
