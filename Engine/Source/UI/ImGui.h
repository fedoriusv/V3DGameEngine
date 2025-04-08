#pragma once

#if USE_IMGUI
#include "Common.h"
#include "WigetHandler.h"
#include "WigetLayout.h"

#include "Renderer/Buffer.h"

struct ImGuiContext;
struct ImDrawData;

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

    struct ImGuiWigetEvents;
    struct ImGuiWigetViewportData;

    /**
    * @brief ImGuiWigetHandler class
    */
    class ImGuiWigetHandler final : public WigetHandler
    {
    public:

        explicit ImGuiWigetHandler(renderer::Device* device) noexcept;
        ~ImGuiWigetHandler();

        void update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt) override;
        void render(renderer::CmdListRender* cmdList) override;

        void handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event) override;
        void handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event) override;
        void handleGamepadCallback(const v3d::event::InputEventHandler* handler, const event::GamepadInputEvent* event) override;
        void handleSystemCallback(const v3d::event::InputEventHandler* handler, const event::SystemEvent* event) override;

        void showDemoUI();
        void hideDemoUI();

    public:

        bool draw_MenuBar(Wiget* menu, Wiget::Context* context, f32 dt) override;
        bool draw_Menu(Wiget* menu, Wiget::Context* context, f32 dt) override;
        bool draw_MenuItem(Wiget* item, Wiget::Context* context, f32 dt) override;
        bool draw_TabBar(Wiget* item, Wiget::Context* context, f32 dt) override;

        bool draw_Window(Wiget::Context* context, f32 dt) override;
        bool draw_Button(Wiget* button, Wiget::Context* context, f32 dt) override;
        bool draw_Image(Wiget* image, Wiget::Context* context, f32 dt) override;

    private:

        bool create(renderer::CmdListRender* cmdList, const renderer::RenderPassDesc& renderpassDesc) override;
        void destroy() override;

        ImGuiContext* m_ImGuiContext;
        bool m_showDemo;

    private:

        bool createFontTexture(renderer::CmdListRender* cmdList);
        void destroyFontTexture();

        bool createPipeline(const renderer::RenderPassDesc& renderpassDesc);
        void destroyPipeline();

        bool createBuffers(ImGuiWigetViewportData* viewportData, u32 indexCount, u32 vertexCount);
        void destroyBuffers(ImGuiWigetViewportData* viewportData);

        bool renderDrawData(ImGuiWigetViewportData* viewportData, ImDrawData* drawData);

        renderer::Texture2D* m_fontAtlas;
        renderer::SamplerState* m_fontSampler;

        renderer::ShaderProgram* m_UIProgram;
        renderer::GraphicsPipelineState* m_UIPipeline;

        const static u32 k_countSwapchaints = 3;
        ImGuiWigetViewportData* m_viewportData;

        u32 m_frameCounter;

        friend ImGuiWigetEvents;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI