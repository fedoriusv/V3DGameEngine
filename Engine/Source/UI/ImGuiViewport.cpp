#include "ImGuiViewport.h"

#include "Platform/Window.h"
#include "Platform/Platform.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"

#if USE_IMGUI
#include "ImGuiHandler.h"
#include "ThirdParty/imgui/imgui.h"

namespace v3d
{
namespace ui
{

void ImGuiWigetViewportEvents::ImGui_CreateWindow(ImGuiViewport* vp)
{
    if (ImGuiViewport* parentViewport = ImGui::FindViewportByID(vp->ParentViewportId))
    {
        platform::Window* parentWindow = reinterpret_cast<platform::Window*>(parentViewport->PlatformUserData);
        ASSERT(parentWindow, "parentWindow is nullptr");

        platform::Window* window = platform::Window::createWindow({ (u32)vp->Size.x, (u32)vp->Size.y }, { (s32)vp->Pos.x, (s32)vp->Pos.y }, parentWindow, false, "ChildWindow");
        ASSERT(window, "window is nullptr");

        vp->PlatformUserData = window;
        vp->PlatformRequestResize = false;
        vp->PlatformHandle = window->getWindowHandle();
        vp->PlatformHandleRaw = window->getWindowHandle();
    }
}

void ImGuiWigetViewportEvents::ImGui_DestroyWindow(ImGuiViewport* vp)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "window is nullptr");

    platform::Window::detroyWindow(window);

    vp->PlatformUserData = nullptr;
    vp->PlatformHandle = nullptr;
}

void ImGuiWigetViewportEvents::ImGui_ShowWindow(ImGuiViewport* vp)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    window->show();
}

void ImGuiWigetViewportEvents::ImGui_SetWindowPos(ImGuiViewport* vp, ImVec2 pos)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    window->setPosition({ (s32)pos.x, (s32)pos.y });
}

ImVec2 ImGuiWigetViewportEvents::ImGui_GetWindowPos(ImGuiViewport* vp)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    const math::Point2D& pos = window->getPosition();
    return ImVec2(pos.m_x, pos.m_y);
}

void ImGuiWigetViewportEvents::ImGui_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    window->setSize({ (u32)size.x, (u32)size.y });
}

ImVec2 ImGuiWigetViewportEvents::ImGui_GetWindowSize(ImGuiViewport* vp)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    const math::Dimension2D& size = window->getSize();
    return ImVec2(size.m_width, size.m_height);
}

void ImGuiWigetViewportEvents::ImGui_SetWindowFocus(ImGuiViewport* vp)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    window->focus();
}

bool ImGuiWigetViewportEvents::ImGui_GetWindowFocus(ImGuiViewport* vp)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    return window->isFocused();
}

bool ImGuiWigetViewportEvents::ImGui_GetWindowMinimized(ImGuiViewport* vp)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    return window->isMinimized();
}

void ImGuiWigetViewportEvents::ImGui_SetWindowTitle(ImGuiViewport* vp, const char* str)
{
    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "nullptr");

    window->setText(std::string(str));
}

void ImGuiWigetViewportEvents::ImGui_SetWindowAlpha(ImGuiViewport* vp, float alpha)
{
}

void ImGuiWigetViewportEvents::ImGui_UpdateWindow(ImGuiViewport* vp)
{
}

float ImGuiWigetViewportEvents::ImGui_GetWindowDpiScale(ImGuiViewport* vp)
{
    const platform::Window* window = reinterpret_cast<const platform::Window*>(vp->PlatformUserData);
    if (window)
    {
        return platform::Platform::getDpiScaleForWindow(window);
    }

    return 0;
}

void ImGuiWigetViewportEvents::ImGui_OnChangedViewport(ImGuiViewport* vp)
{
}

ImVec4 ImGuiWigetViewportEvents::ImGui_GetWindowWorkAreaInsets(ImGuiViewport* vp)
{
    return ImVec4();
}

void ImGuiWigetViewportEvents::ImGui_Renderer_CreateWindow(ImGuiViewport* vp)
{
    ASSERT(ImGui::GetCurrentContext(), "ImGui context is not valid");
    ImGuiWigetHandler* handler = reinterpret_cast<ImGuiWigetHandler*>(ImGui::GetIO().BackendPlatformUserData);
    ASSERT(handler, "handler is nullptr");

    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "window is nullptr");

    renderer::Swapchain::SwapchainParams params;
    params._size = window->getSize();

    renderer::Swapchain* swapchain = handler->m_device->createSwapchain(window, params);
    ASSERT(swapchain, "swapchain is nullptr");

    ImGuiWigetViewportData* viewportData = V3D_NEW(ImGuiWigetViewportData, memory::MemoryLabel::MemoryUI);
    viewportData->_swapchain = swapchain;
    viewportData->_cmdList = handler->m_device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
    viewportData->_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryUI)(handler->m_device, swapchain->getBackbufferSize(), 1, 0, "WindowViewport");
    viewportData->_renderTarget->setColorTexture(0, swapchain->getBackbuffer(),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
        }
    );

    if (!handler->createBuffers(viewportData, k_ImGui_IndexCount, k_ImGui_VertexCount))
    {
        ASSERT(false, "createBuffers is failed");
        handler->destroyBuffers(viewportData);
    }

    vp->RendererUserData = viewportData;
}

void ImGuiWigetViewportEvents::ImGui_Renderer_DestroyWindow(ImGuiViewport* vp)
{
    ASSERT(ImGui::GetCurrentContext(), "ImGui context is not valid");
    ImGuiWigetHandler* handler = reinterpret_cast<ImGuiWigetHandler*>(ImGui::GetIO().BackendPlatformUserData);
    ASSERT(handler, "handler is nullptr");

    ImGuiWigetViewportData* viewportData = reinterpret_cast<ImGuiWigetViewportData*>(vp->RendererUserData);
    ASSERT(viewportData, "viewportData is nullptr");

    V3D_DELETE(viewportData->_renderTarget, memory::MemoryLabel::MemoryUI);
    handler->m_device->destroyCommandList(viewportData->_cmdList);
    handler->m_device->destroySwapchain(viewportData->_swapchain);
    V3D_DELETE(viewportData, memory::MemoryLabel::MemoryUI);

    vp->RendererUserData = nullptr;
}

void ImGuiWigetViewportEvents::ImGui_Renderer_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
{
    ASSERT(ImGui::GetCurrentContext(), "ImGui context is not valid");
    ImGuiWigetHandler* handler = reinterpret_cast<ImGuiWigetHandler*>(ImGui::GetIO().BackendPlatformUserData);
    ASSERT(handler, "handler is nullptr");

    platform::Window* window = reinterpret_cast<platform::Window*>(vp->PlatformUserData);
    ASSERT(window, "window is nullptr");

    ImGuiWigetViewportData* viewportData = reinterpret_cast<ImGuiWigetViewportData*>(vp->RendererUserData);
    ASSERT(viewportData, "viewportData is nullptr");


    V3D_DELETE(viewportData->_renderTarget, memory::MemoryLabel::MemoryUI);
    handler->m_device->destroySwapchain(viewportData->_swapchain);


    renderer::Swapchain::SwapchainParams params;
    params._size = window->getSize();

    renderer::Swapchain* swapchain = handler->m_device->createSwapchain(window, params);
    ASSERT(swapchain, "swapchain is nullptr");

    viewportData->_swapchain = swapchain;
    viewportData->_renderTarget = V3D_NEW(renderer::RenderTargetState, memory::MemoryLabel::MemoryUI)(handler->m_device, swapchain->getBackbufferSize(), 1, 0, "WindowViewport");
    viewportData->_renderTarget->setColorTexture(0, swapchain->getBackbuffer(),
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, math::Vector4D(0.0f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_Present
        }
    );
}

void ImGuiWigetViewportEvents::ImGui_Renderer_RenderWindow(ImGuiViewport* vp, void* render_arg)
{
    ASSERT(ImGui::GetCurrentContext(), "ImGui context is not valid");
    ImGuiWigetHandler* handler = reinterpret_cast<ImGuiWigetHandler*>(ImGui::GetIO().BackendPlatformUserData);
    ASSERT(handler, "handler is nullptr");

    ImGuiWigetViewportData* viewportData = reinterpret_cast<ImGuiWigetViewportData*>(vp->RendererUserData);
    ASSERT(viewportData, "viewportData is nullptr");

    viewportData->_swapchain->beginFrame();

    viewportData->_cmdList->beginRenderTarget(*viewportData->_renderTarget);
    if (!handler->renderDrawData(viewportData, vp->DrawData))
    {
        viewportData->_cmdList->clear(viewportData->_swapchain->getBackbuffer(), { 1, 0, 0, 0 });
    }
    viewportData->_cmdList->endRenderTarget();

    handler->m_device->submit(viewportData->_cmdList);

    viewportData->_swapchain->endFrame();
}

void ImGuiWigetViewportEvents::ImGui_Renderer_Present(ImGuiViewport* vp, void* render_arg)
{
    ASSERT(ImGui::GetCurrentContext(), "ImGui context is not valid");
    ImGuiWigetViewportData* viewportData = reinterpret_cast<ImGuiWigetViewportData*>(vp->RendererUserData);
    ASSERT(viewportData, "viewportData is nullptr");

    viewportData->_swapchain->presentFrame();
}

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI