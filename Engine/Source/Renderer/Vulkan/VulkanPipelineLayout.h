#pragma once

#include "Common.h"
#include "Renderer/Shader.h"
//#include "Renderer/ObjectTracker.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
    class ShaderProgram;

namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanPipelineLayoutDescription struct. Vulkan Render side
    */
    struct VulkanDescriptorSetLayoutDescription
    {
        struct Hasher
        {
            static u64 hash(const VulkanDescriptorSetLayoutDescription& data) noexcept
            {
                return static_cast<u64>(crc32c::Extend(static_cast<u32>(data._bindings.size()), reinterpret_cast<const u8*>(data._bindings.data()), data._bindings.size() * sizeof(VkDescriptorSetLayoutBinding)));
            }
        };

        VulkanDescriptorSetLayoutDescription() noexcept = default;
        VulkanDescriptorSetLayoutDescription(const std::vector<VkDescriptorSetLayoutBinding>& bindings) noexcept;

        bool operator==(const VulkanDescriptorSetLayoutDescription& other) const;

        const std::vector<VkDescriptorSetLayoutBinding>& _bindings;
    };

    inline VulkanDescriptorSetLayoutDescription::VulkanDescriptorSetLayoutDescription(const std::vector<VkDescriptorSetLayoutBinding>& bindings) noexcept
        : _bindings(bindings)
    {
    }

    inline bool VulkanDescriptorSetLayoutDescription::operator==(const VulkanDescriptorSetLayoutDescription& other) const
    {
        if (_bindings.size() != other._bindings.size())
        {
            return false;
        }

        if (!_bindings.empty() && memcmp(_bindings.data(), other._bindings.data(), _bindings.size() * sizeof(VkDescriptorSetLayoutBinding)) != 0)
        {
            return false;
        }

        return true;
    }

    using VulkanDescriptorSetLayoutDescriptionType = DescInfo<VulkanDescriptorSetLayoutDescription, VulkanDescriptorSetLayoutDescription::Hasher>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanPipelineLayoutDescription struct. Vulkan Render side
    */
    struct VulkanPipelineLayoutDescription
    {
        VulkanPipelineLayoutDescription() noexcept;
        
        struct Compare
        {
            bool operator()(const VulkanPipelineLayoutDescription& descl, const VulkanPipelineLayoutDescription& descr) const;
        };

        struct Hash
        {
            size_t operator()(const VulkanPipelineLayoutDescription& desc) const;
        };

        u64 _key;
        u32 _bindingsSetsMask;
        std::array<std::vector<VkDescriptorSetLayoutBinding>, k_maxDescriptorSetCount> _bindingsSet;
        std::vector<VkPushConstantRange> _pushConstant;
    };

    /**
    * @brief VulkanPipelineLayout struct. Vulkan Render side
    */
    struct VulkanPipelineLayout
    {
        VulkanPipelineLayout() noexcept;

        VkPipelineLayout _pipelineLayout;
        std::array<VkDescriptorSetLayout, k_maxDescriptorSetCount> _setLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanPipelineLayoutManager class. Vulkan Render side
    */
    class VulkanPipelineLayoutManager final
    {
    public:

        struct DescriptorSetLayoutCreator
        {
            explicit DescriptorSetLayoutCreator(VulkanDevice& device, const renderer::ShaderProgram* program) noexcept;

            DescriptorSetLayoutCreator() = delete;
            DescriptorSetLayoutCreator(const DescriptorSetLayoutCreator&) = delete;

            VulkanPipelineLayoutDescription _description;
        };

        static VkShaderStageFlagBits convertShaderTypeToVkStage(ShaderType type);

        explicit VulkanPipelineLayoutManager(VulkanDevice* device) noexcept;
        ~VulkanPipelineLayoutManager();

        [[nodiscard]] VulkanPipelineLayout acquirePipelineLayout(const VulkanPipelineLayoutDescription& desc);
        bool removePipelineLayout(const VulkanPipelineLayoutDescription& desc);

        void clear();

    private:

        VulkanPipelineLayoutManager() = delete;
        VulkanPipelineLayoutManager(const VulkanPipelineLayoutManager&) = delete;

        VkDescriptorSetLayout acquireDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

        VkPipelineLayout createPipelineLayout(const VulkanPipelineLayoutDescription& desc, const std::array<VkDescriptorSetLayout, k_maxDescriptorSetCount>& descriptorSetLayouts);

        VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        void destroyDescriptorSetLayouts(std::array<VkDescriptorSetLayout, k_maxDescriptorSetCount>& descriptorSetLayouts);

        VulkanDevice&   m_device;
        utils::Spinlock m_mutex;
        std::unordered_map<VulkanDescriptorSetLayoutDescriptionType, VkDescriptorSetLayout, VulkanDescriptorSetLayoutDescriptionType::Hash, VulkanDescriptorSetLayoutDescriptionType::Compare> m_descriptorSetLayouts;
        std::unordered_map<VulkanPipelineLayoutDescription, VulkanPipelineLayout, VulkanPipelineLayoutDescription::Hash, VulkanPipelineLayoutDescription::Compare> m_pipelinesLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
