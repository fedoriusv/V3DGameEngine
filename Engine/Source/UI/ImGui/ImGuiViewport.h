#pragma once

#if USE_IMGUI
#include "Common.h"
#include "ThirdParty/imgui/imgui.h"

#include "Renderer/Buffer.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class Swapchain;
    class CmdListRender;
    class RenderTargetState;
    class IndexBuffer;
    class VertexBuffer;

} // namespace render
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_ImGui_IndexCount = 4096;
    constexpr u32 k_ImGui_VertexCount = 4096;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ImGuiWidgetViewportData
    {
        renderer::Swapchain*                        _swapchain = nullptr;
        renderer::CmdListRender*                    _cmdList = nullptr;
        renderer::RenderTargetState*                _renderTarget = nullptr;

        std::vector<renderer::IndexBuffer*>         _indexBuffer;
        std::vector<renderer::VertexBuffer*>        _vertexBuffer;
        std::vector<renderer::GeometryBufferDesc>   _geometryDesc;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ImGuiWidgetViewportEvents
    {
        static void ImGui_CreateWindow(ImGuiViewport* vp);
        static void ImGui_DestroyWindow(ImGuiViewport* vp);
        static void ImGui_ShowWindow(ImGuiViewport* vp);
        static void ImGui_SetWindowPos(ImGuiViewport* vp, ImVec2 pos);
        static ImVec2 ImGui_GetWindowPos(ImGuiViewport* vp);
        static void ImGui_SetWindowSize(ImGuiViewport* vp, ImVec2 size);
        static ImVec2 ImGui_GetWindowSize(ImGuiViewport* vp);
        static void ImGui_SetWindowFocus(ImGuiViewport* vp);
        static bool ImGui_GetWindowFocus(ImGuiViewport* vp);
        static bool ImGui_GetWindowMinimized(ImGuiViewport* vp);
        static void ImGui_SetWindowTitle(ImGuiViewport* vp, const char* str);
        static void ImGui_SetWindowAlpha(ImGuiViewport* vp, float alpha);
        static void ImGui_UpdateWindow(ImGuiViewport* vp);
        static float ImGui_GetWindowDpiScale(ImGuiViewport* vp);
        static void ImGui_OnChangedViewport(ImGuiViewport* vp);
        static ImVec4 ImGui_GetWindowWorkAreaInsets(ImGuiViewport* vp);

        static void ImGui_Renderer_CreateWindow(ImGuiViewport* vp);
        static void ImGui_Renderer_DestroyWindow(ImGuiViewport* vp);
        static void ImGui_Renderer_SetWindowSize(ImGuiViewport* vp, ImVec2 size);
        static void ImGui_Renderer_RenderWindow(ImGuiViewport* vp, void* render_arg);
        static void ImGui_Renderer_Present(ImGuiViewport* vp, void* render_arg);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI