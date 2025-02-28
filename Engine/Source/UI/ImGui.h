#pragma once

#if USE_IMGUI
#include "Common.h"
#include "WigetHandler.h"
#include "WigetLayout.h"

#include "Renderer/Buffer.h"

struct ImGuiContext;

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

    class ImGuiWigetHandler final : public WigetHandler
    {
    public:

        explicit ImGuiWigetHandler(renderer::Device* device) noexcept;
        ~ImGuiWigetHandler();

        WigetLayout* createWigetLayout(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos) override;
        void destroyWigetLayout(WigetLayout* layout) override;

        WigetLayout* createWigetMenuLayout() override;
        void createWigetMenuLayout(WigetLayout* layout) override;

        void update(platform::Window* window, f32 dt) override;
        void render(renderer::CmdListRender* cmdList) override;

        void handleMouseCallback(event::InputEventHandler* handler, const event::MouseInputEvent* event) override;
        void handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event) override;

        void showDemoUI();
        void hideDemoUI();

    public:

        bool drawButton(Wiget* button, f32 dt) override;

        bool beginDrawMenu(Wiget* menu, f32 dt) override;
        bool endDrawMenu(Wiget* menu, f32 dt) override;
        bool drawMenuItem(Wiget* item, f32 dt) override;

    private:

        bool create(renderer::CmdListRender* cmdList, renderer::RenderTargetState* renderTarget) override;
        void destroy() override;

        ImGuiContext* m_ImGuiContext;
        bool m_showDemo;

    private:

        bool createFontTexture(renderer::CmdListRender* cmdList);
        void destroyFontTexture();

        bool createPipeline(v3d::renderer::RenderTargetState* renderTarget);
        void destroyPipeline();

        bool createBuffers(u32 indexCount, u32 vertexCount);
        void destroyBuffers();

        renderer::Device* const m_device;

        renderer::Texture2D* m_fontAtlas;
        renderer::SamplerState* m_fontSampler;

        renderer::ShaderProgram* m_UIProgram;
        renderer::GraphicsPipelineState* m_UIPipeline;

        const static u32 k_countSwapchaints = 6;
        renderer::IndexBuffer* m_UIIndexBuffer[k_countSwapchaints];
        renderer::VertexBuffer* m_UIVertexBuffer[k_countSwapchaints];
        renderer::GeometryBufferDesc m_UIGeometryDesc[k_countSwapchaints];

        u32 m_offsetIB = 0;
        u32 m_offsetVB = 0;

        u32 m_frameCounter;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ImGuiWigetLayout : public WigetLayout
    {
    public:

        explicit ImGuiWigetLayout(WigetHandler* handler, const std::string& title = "") noexcept;
        ~ImGuiWigetLayout() = default;

    private:

        void update(f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ImGuiWigetMenuLayout : public WigetLayout
    {
    public:

        explicit ImGuiWigetMenuLayout(WigetHandler* handler) noexcept;
        ~ImGuiWigetMenuLayout() = default;

    private:

        void update(f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI