#pragma once

#include "Context.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanGraphicContext final : public Context
    {
    public:

        struct DeviceInfo
        {
            //TODO:
            u32 queueIndex;
        };

        VulkanGraphicContext();
        ~VulkanGraphicContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;

    private:

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
