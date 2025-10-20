#pragma once

#if USE_IMGUI
#include "Common.h"
#include "UI/WidgetHandler.h"
#include "UI/WidgetLayout.h"

#include "Renderer/Buffer.h"

struct ImGuiContext;
struct ImDrawData;
struct ImFont;
struct ImFontConfig;

namespace v3d
{
namespace renderer
{
    class Device;
    class Texture2D;
    class SamplerState;
    class ShaderProgram;
    class GraphicsPipelineState;
    class IndexBuffer;
    class VertexBuffer;

} // namespace render
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ImGuiWidgetViewportEvents;
    struct ImGuiWidgetViewportData;
    class ImGuiWidgetDrawer;

    /**
    * @brief ImGuiWidgetHandler class
    */
    class ImGuiWidgetHandler final : public WidgetHandler
    {
    public:

        enum ImGuiWidgetFlag
        {
            ImGui_ViewportMode = 1 << 0,
            ImGui_Gizmo = 1 << 1,
        };
        typedef u32 ImGuiWidgetFlags;

        explicit ImGuiWidgetHandler(renderer::Device* device, ImGuiWidgetFlags flags = 0) noexcept;
        ~ImGuiWidgetHandler();

        void update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt) override;
        bool render(renderer::CmdListRender* cmdList) override;

        void handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event) override;
        void handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event) override;
        void handleGamepadCallback(const v3d::event::InputEventHandler* handler, const event::GamepadInputEvent* event) override;
        void handleSystemCallback(const v3d::event::InputEventHandler* handler, const event::SystemEvent* event) override;

        void showDemoUI();
        void hideDemoUI();

    private:

        bool create(const renderer::RenderPassDesc& renderpassDesc) override;
        void destroy() override;

        ImGuiContext* m_ImGuiContext;
        bool m_showDemo;

    private:

        bool createFontTexture(renderer::CmdListRender* cmdList, ImFontConfig* fontConfig);
        void destroyFontTexture();

        bool createPipeline(const renderer::RenderPassDesc& renderpassDesc);
        void destroyPipeline();

        bool createBuffers(ImGuiWidgetViewportData* viewportData, u32 indexCount, u32 vertexCount);
        void destroyBuffers(ImGuiWidgetViewportData* viewportData);

        bool renderDrawData(ImGuiWidgetViewportData* viewportData, ImDrawData* drawData);

        std::vector<renderer::Texture2D*>                   m_activeTextures;
        std::array<ImFont*, WidgetLayout::FontSize_Count>   m_fonts;
        renderer::Texture2D*                                m_fontAtlas;
        renderer::SamplerState*                             m_imageSampler;

        renderer::ShaderProgram*                            m_UIProgram;
        renderer::GraphicsPipelineState*                    m_UIPipeline;

        const static u32                                    k_countSwapchaints = 3;
        ImGuiWidgetViewportData*                            m_viewportData;

        u32                                                 m_frameCounter;
        ImGuiWidgetFlags                                    m_flags;
        u32                                                 m_mouseCursor;

        friend ImGuiWidgetViewportEvents;
        friend ImGuiWidgetDrawer;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI