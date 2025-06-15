#pragma once

#include "Common.h"
#include "Renderer/Device.h"

namespace v3d
{
namespace renderer
{
    //TODO
    class RenderTask
    {
    public:

        RenderTask(const std::function<void(Device* device, CmdListRender* cmdList)>& func) noexcept
            : m_func(func)
        {
        }

        void execute(Device* device, CmdListRender* cmdList)
        {
            std::invoke(m_func, device, cmdList);
        }

    private:

        std::function<void(Device* device, CmdListRender* cmdList)> m_func;
    };


} //namespace renderer
} //namespace v3d