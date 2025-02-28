#include "ImGui.h"
#include "Wiget.h"

#include "Utils/Logger.h"
#include "Platform/Window.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Events/InputEventHandler.h"
#include "Events/InputEventReceiver.h"
#include "Events/InputEventMouse.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Stream/StreamManager.h"
#include "Resource/ShaderCompiler.h"

#if USE_IMGUI
#include "ThirdParty/imgui/imgui.h"

namespace v3d
{
namespace ui
{

ImGuiWigetHandler::ImGuiWigetHandler(renderer::Device* device) noexcept
    : WigetHandler(device)
    , m_ImGuiContext(nullptr)
    , m_showDemo(false)

    , m_device(device)

    , m_frameCounter(~1)
{
}


ImGuiWigetHandler::~ImGuiWigetHandler()
{
    ASSERT(!m_ImGuiContext, "must be nullptr");
}

bool ImGuiWigetHandler::create(renderer::CmdListRender* cmdList, renderer::RenderTargetState* renderTarget)
{
    ASSERT(!m_ImGuiContext, "must be nullptr");
    ImGui::SetAllocatorFunctions(
        [](size_t size, void* userData) -> void*
        {
            return V3D_MALLOC(size, v3d::memory::MemoryLabel::MemorySystem);
        },
        [](void* ptr, void* userData)
        {
            V3D_FREE(ptr, v3d::memory::MemoryLabel::MemorySystem);
        }
    );

    m_ImGuiContext = ImGui::CreateContext();
    if (!m_ImGuiContext)
    {
        LOG_ERROR("ImGuiWigetLayout::create CreateContext is failed");
        destroy();

        return false;
    }

    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imguiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    if (!createFontTexture(cmdList))
    {
        LOG_ERROR("ImGuiWigetLayout::create createFontTexture is failed");
        destroy();

        return false;
    }

    if (!createPipeline(renderTarget))
    {
        LOG_ERROR("ImGuiWigetLayout::create createPipeline is failed");
        destroy();

        return false;
    }


    //TODO dynamic?
    u32 indexCount = 4096;
    u32 vertexCount = 4096;
    if (!createBuffers(indexCount, vertexCount))
    {
        LOG_ERROR("ImGuiWigetLayout::create createBuffers is failed");
        destroy();

        return false;
    }

    //TODO: style
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.0f);

    // Color scheme
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

    //ImGui::StyleColorsClassic();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    //ImGui::SetCurrentContext(m_IMGUIContext);

    return true;
}

void ImGuiWigetHandler::destroy()
{
    destroyBuffers();
    destroyPipeline();
    destroyFontTexture();

    if (m_ImGuiContext)
    {
        ImGui::DestroyContext(m_ImGuiContext);
        m_ImGuiContext = nullptr;
    }
}

WigetLayout* ImGuiWigetHandler::createWigetLayout(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos)
{
    ImGuiWigetLayout* layout = V3D_NEW(ImGuiWigetLayout, memory::MemoryLabel::MemorySystem)(this, title);
    layout->setSize(size);
    layout->setPosition(pos);

    m_wigetLayouts.push_back(layout);
    return layout;
}

void ImGuiWigetHandler::destroyWigetLayout(WigetLayout* layout)
{
    if (layout)
    {
        auto found = std::find(m_wigetLayouts.begin(), m_wigetLayouts.end(), layout);
        ASSERT(found != m_wigetLayouts.end(), "not found");
        m_wigetLayouts.erase(found);

        V3D_DELETE(layout, memory::MemoryLabel::MemorySystem);
    }
}

WigetLayout* ImGuiWigetHandler::createWigetMenuLayout()
{
    ImGuiWigetMenuLayout* layout = V3D_NEW(ImGuiWigetMenuLayout, memory::MemoryLabel::MemorySystem)(this);

    m_wigetLayouts.push_back(layout);
    return layout;
}

void ImGuiWigetHandler::createWigetMenuLayout(WigetLayout* layout)
{
    if (layout)
    {
        auto found = std::find(m_wigetLayouts.begin(), m_wigetLayouts.end(), layout);
        ASSERT(found != m_wigetLayouts.end(), "not found");
        m_wigetLayouts.erase(found);

        V3D_DELETE(layout, memory::MemoryLabel::MemorySystem);
    }
}

void ImGuiWigetHandler::handleMouseCallback(event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ASSERT(handler, "must be valid");

    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
    imguiIO.AddMousePosEvent(handler->getCursorPosition().m_x, handler->getCursorPosition().m_y);
    imguiIO.AddMouseButtonEvent(0, handler->isLeftMousePressed());
    imguiIO.AddMouseButtonEvent(1, handler->isRightMousePressed());
    imguiIO.AddMouseWheelEvent(0.0f, handler->getMouseWheel());
}

void ImGuiWigetHandler::handleKeyboardCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ASSERT(handler, "must be valid");

    ImGuiIO& imguiIO = ImGui::GetIO();
    //imguiIO.AddInputCharacter();
    //imguiIO.AddKeyEvent(); //map
}

void ImGuiWigetHandler::update(platform::Window* window, f32 dt)
{
    ASSERT(m_ImGuiContext, "must be valid");

    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.DeltaTime = (dt == 0.0f) ? 1.0f / 60.0f : dt;
    imguiIO.DisplaySize.x = window->getSize().m_width;
    imguiIO.DisplaySize.y = window->getSize().m_height;
    imguiIO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);


    ImGui::NewFrame();

    WigetHandler::update(window, dt);

    if (m_showDemo)
    {
        ImGui::ShowDemoWindow();
    }

    ImGui::Render();
}

bool ImGuiWigetHandler::drawButton(Wiget* wiget, f32 dt)
{
    ASSERT(m_ImGuiContext, "must be valid");
    WigetButton* button = static_cast<WigetButton*>(wiget);

    bool action = ImGui::Button(button->getTitle().c_str());
    if (button->isShowToolTip() && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(button->getToolTip().c_str());
    }

    if (button->m_onHoveredEvent && ImGui::IsItemHovered())
    {
        std::invoke(button->m_onHoveredEvent, wiget);
    }

    if (button->m_onClickedEvent && ImGui::IsItemClicked())
    {
        std::invoke(button->m_onClickedEvent, wiget);
    }

    return action;
}

bool ImGuiWigetHandler::beginDrawMenu(Wiget* wiget, f32 dt)
{
    ASSERT(m_ImGuiContext, "must be valid");
    WigetMenu* menu = static_cast<WigetMenu*>(wiget);

    bool action = ImGui::BeginMenu(menu->getTitle().c_str(), menu->isActive());
    if (menu->isShowToolTip() && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(menu->getToolTip().c_str());
    }

    if (menu->m_onClickedEvent && ImGui::IsItemClicked())
    {
        std::invoke(menu->m_onClickedEvent, wiget);
    }

    return action;
}

bool ImGuiWigetHandler::endDrawMenu(Wiget* menu, f32 dt)
{
    ASSERT(m_ImGuiContext, "must be valid");
    ImGui::EndMenu();

    return true;
}

bool ImGuiWigetHandler::drawMenuItem(Wiget* wiget, f32 dt)
{
    ASSERT(m_ImGuiContext, "must be valid");
    WigetMenu::MenuItem* item = static_cast<WigetMenu::MenuItem*>(wiget);

    bool clicked = false;
    bool action = ImGui::MenuItem(item->getTitle().c_str(), nullptr, &clicked, item->isActive());
    if (item->m_onClickedEvent && clicked)
    {
        std::invoke(item->m_onClickedEvent, wiget);
    }

    return action;
}

void ImGuiWigetHandler::showDemoUI()
{
    m_showDemo = true;
}

void ImGuiWigetHandler::hideDemoUI()
{
    m_showDemo = false;
}

bool ImGuiWigetHandler::createFontTexture(renderer::CmdListRender* cmdList)
{
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.Fonts->AddFontDefault(); //TODO
    imguiIO.FontGlobalScale = 1.0f;

    //Font texture
    s32 width, height;
    u8* pixels = nullptr;
    u32 pixelSize = 4; //RGBA8
    imguiIO.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    m_fontSampler = V3D_NEW(renderer::SamplerState, memory::MemoryLabel::MemoryRenderCore)(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    if (!m_fontSampler)
    {
        return false;
    }
    m_fontSampler->setWrap(renderer::SamplerWrap::TextureWrap_ClampToEdge);
    m_fontSampler->setBorderColor(math::Vector4D(1.0, 1.0, 1.0, 1.0)); //opaque white

    m_fontAtlas = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryRenderCore)(m_device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Write, renderer::Format::Format_R8G8B8A8_UNorm, math::Dimension2D(width, height), 1);
    if (!m_fontAtlas)
    {
        return false;
    }
    cmdList->uploadData(m_fontAtlas, width * height * pixelSize, pixels);
    m_device->submit(cmdList, true);

    return true;
}

void ImGuiWigetHandler::destroyFontTexture()
{
    if (m_fontSampler)
    {
        V3D_DELETE(m_fontSampler, memory::MemoryLabel::MemoryRenderCore);
        m_fontSampler = nullptr;
    }

    if (m_fontAtlas)
    {
        V3D_DELETE(m_fontAtlas, memory::MemoryLabel::MemoryRenderCore);
        m_fontAtlas = nullptr;
    }
}

bool ImGuiWigetHandler::createBuffers(u32 indexCount, u32 vertexCount)
{
    for (u32 i = 0; i < k_countSwapchaints; ++i)
    {
        m_UIIndexBuffer[i] = new renderer::IndexBuffer(m_device, renderer::BufferUsage::Buffer_GPUWriteCocherent, sizeof(ImDrawIdx) == 2 ? renderer::IndexBufferType::IndexType_16 : renderer::IndexBufferType::IndexType_32, indexCount, "UIIndexBuffer_" + std::to_string(i));
        m_UIVertexBuffer[i] = new renderer::VertexBuffer(m_device, renderer::BufferUsage::Buffer_GPUWriteCocherent, vertexCount, vertexCount * sizeof(ImDrawVert), "UIVertexBuffer_" + std::to_string(i));
        m_UIGeometryDesc[i] = renderer::GeometryBufferDesc(m_UIIndexBuffer[i], 0, m_UIVertexBuffer[i], 0, sizeof(ImDrawVert), 0);
    }

    return true;
}

void ImGuiWigetHandler::destroyBuffers()
{
    for (u32 i = 0; i < k_countSwapchaints; ++i)
    {
        delete m_UIIndexBuffer[i];
        delete m_UIVertexBuffer[i];
    }
}

bool ImGuiWigetHandler::createPipeline(v3d::renderer::RenderTargetState* renderTarget)
{
    //shaders
    {
        const renderer::VertexShader* vertShader = nullptr;
        {
            const std::string vertexSource("\
            struct VS_INPUT\n\
            {\n\
                [[vk::location(0)]] float2 Position : POSITION;\n\
                [[vk::location(1)]] float2 UV       : TEXTURE;\n\
                [[vk::location(2)]] float4 Color    : COLOR;\n\
            };\n\
            \n\
            struct PushConstants\n\
            {\n\
                float2 scale;\n\
                float2 translate;\n\
            };\n\
            \n\
            [[vk::push_constant]]\n\
            PushConstants pushConstants;\n\
            \n\
            struct VS_OUTPUT\n\
            {\n\
                float4 Pos                       : SV_POSITION;\n\
                [[vk::location(0)]] float2 UV    : TEXTURE;\n\
                [[vk::location(1)]] float4 Color : COLOR;\n\
            };\n\
            \n\
            VS_OUTPUT main(VS_INPUT Input)\n\
            {\n\
                VS_OUTPUT Out;\n\
                Out.Pos = float4(Input.Position * pushConstants.scale + pushConstants.translate, 0.0, 1.0);\n\
                Out.Pos.y = -Out.Pos.y;\n\
                Out.UV = Input.UV;\n\
                Out.Color = Input.Color;\n\
                return Out;\n\
            }");
            const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

            resource::ShaderDecoder::ShaderPolicy vertexPolicy;
            vertexPolicy._type = renderer::ShaderType::Vertex;
            vertexPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
            vertexPolicy._content = renderer::ShaderContent::Source;
            vertexPolicy._entryPoint = "main";

            vertShader = resource::ShaderCompiler::compileShader<renderer::VertexShader>(m_device, "UIVertex", vertexPolicy, vertexStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
            stream::StreamManager::destroyStream(vertexStream);
        }

        const renderer::FragmentShader* fragShader = nullptr;
        {
            const std::string fragmentSource("\
            struct PS_INPUT\n\
            {\n\
                [[vk::location(0)]] float2 UV    : TEXTURE;\n\
                [[vk::location(1)]] float4 Color : COLOR;\n\
            };\n\
            \n\
            [[vk::binding(0, 0)]] Texture2D fontTexture    : register(t0);\n\
            [[vk::binding(1, 0)]] SamplerState fontSampler : register(s0);\n\
            \n\
            float4 main(PS_INPUT Input) : SV_TARGET0\n\
            {\n\
                return Input.Color * fontTexture.Sample(fontSampler, Input.UV);\n\
            }");
            const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

            resource::ShaderDecoder::ShaderPolicy fragmentPolicy;
            fragmentPolicy._type = renderer::ShaderType::Fragment;
            fragmentPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
            fragmentPolicy._content = renderer::ShaderContent::Source;
            fragmentPolicy._entryPoint = "main";

            fragShader = resource::ShaderCompiler::compileShader<renderer::FragmentShader>(m_device, "UIFragment", fragmentPolicy, fragmentStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
            stream::StreamManager::destroyStream(fragmentStream);
        }

        ASSERT(vertShader && fragShader, "nullptr");
        m_UIProgram = V3D_NEW(renderer::ShaderProgram, memory::MemoryLabel::MemoryRenderCore)(m_device, vertShader, fragShader);
        if (!m_UIProgram)
        {
            return false;
        }
    }

    //pipeline
    {
        renderer::VertexInputAttributeDesc vertexDesc(
            {
                renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(ImDrawVert)),
            },
            {
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, offsetof(ImDrawVert, pos)),
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, offsetof(ImDrawVert, uv)),
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R8G8B8A8_UNorm, offsetof(ImDrawVert, col)),
            }
        );

        m_UIPipeline = V3D_NEW(renderer::GraphicsPipelineState, memory::MemoryLabel::MemoryRenderCore)(m_device, vertexDesc, m_UIProgram, renderTarget, "UIPipeline");
        if (!m_UIPipeline)
        {
            return false;
        }

        m_UIPipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        m_UIPipeline->setFrontFace(renderer::FrontFace::FrontFace_CounterClockwise);
        m_UIPipeline->setCullMode(renderer::CullMode::CullMode_None);
        m_UIPipeline->setPolygonMode(renderer::PolygonMode::PolygonMode_Fill);
        m_UIPipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        m_UIPipeline->setBlendEnable(true);
        m_UIPipeline->setColorBlendFactor(renderer::BlendFactor::BlendFactor_SrcAlpha, renderer::BlendFactor::BlendFactor_OneMinusSrcAlpha);
        m_UIPipeline->setColorBlendOp(renderer::BlendOperation::BlendOp_Add);
        m_UIPipeline->setAlphaBlendFactor(renderer::BlendFactor::BlendFactor_OneMinusSrcAlpha, renderer::BlendFactor::BlendFactor_Zero);
        m_UIPipeline->setAlphaBlendOp(renderer::BlendOperation::BlendOp_Add);
        m_UIPipeline->setDepthWrite(false);
        m_UIPipeline->setDepthTest(false);
    }

    return true;
}

void ImGuiWigetHandler::destroyPipeline()
{
    if (m_UIPipeline)
    {
        V3D_DELETE(m_UIPipeline, memory::MemoryLabel::MemoryRenderCore);
        m_UIPipeline = nullptr;
    }

    if (m_UIProgram)
    {
        V3D_DELETE(m_UIProgram, memory::MemoryLabel::MemoryRenderCore);
        m_UIProgram = nullptr;
    }
}

void ImGuiWigetHandler::render(renderer::CmdListRender* cmdList)
{
    ++m_frameCounter;

    ImDrawData* imDrawData = ImGui::GetDrawData();
    ImGuiIO& imguiIO = ImGui::GetIO();

    u32 currIndex = m_frameCounter % k_countSwapchaints;

    u32 vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    u32 indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    if ((vertexBufferSize == 0) && (indexBufferSize == 0))
    {
        return;
    }

    u32 indexCount = m_UIIndexBuffer[currIndex]->getIndicesCount();
    u32 vertexCount = m_UIVertexBuffer[currIndex]->getVerticesCount();
    if (imDrawData->TotalIdxCount > indexCount || imDrawData->TotalVtxCount > vertexCount)
    {
        destroyBuffers();
        createBuffers(std::max<u32>(indexCount, imDrawData->TotalIdxCount), std::max<u32>(vertexCount, imDrawData->TotalVtxCount));
    }
    ASSERT(vertexBufferSize <= m_UIVertexBuffer[currIndex]->getSize(), "out of size");
    ASSERT(indexBufferSize <= m_UIIndexBuffer[currIndex]->getIndicesCount() * sizeof(u16), "out of size");

    ImDrawIdx* idxDst = m_UIIndexBuffer[currIndex]->map<ImDrawIdx>();
    ImDrawVert* vtxDst = m_UIVertexBuffer[currIndex]->map<ImDrawVert>();
    for (int n = 0; n < imDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = imDrawData->CmdLists[n];

        memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        idxDst += cmd_list->IdxBuffer.Size;
        vtxDst += cmd_list->VtxBuffer.Size;
    }

    m_UIIndexBuffer[currIndex]->unmap();
    m_UIVertexBuffer[currIndex]->unmap();


    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;
    if (imDrawData->CmdListsCount > 0) 
    {
        cmdList->setViewport(math::Rect32(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));

        cmdList->setPipelineState(*m_UIPipeline);
        cmdList->bindTexture(0, 0, m_fontAtlas);
        cmdList->bindSampler(0, 1, *m_fontSampler);

        struct PushConstant
        {
            math::Vector2D _scale;
            math::Vector2D _translate;
        } pushConstBlock;
        pushConstBlock._scale = math::Vector2D(2.0f / imguiIO.DisplaySize.x, 2.0f / imguiIO.DisplaySize.y);
        pushConstBlock._translate = math::Vector2D(-1.0f);

        cmdList->bindPushConstant(renderer::ShaderType::Vertex, sizeof(PushConstant), &pushConstBlock);

        for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                u32 x = std::max((int32_t)(pcmd->ClipRect.x), 0);
                u32 y = std::max((int32_t)(pcmd->ClipRect.y), 0);
                u32 width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                u32 height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                cmdList->setScissor(math::Rect32(x, y, x + width, y + height));

                m_UIGeometryDesc[currIndex]._indexOffset = m_offsetIB;
                m_UIGeometryDesc[currIndex]._offsets[0] = m_offsetVB;
                cmdList->drawIndexed(m_UIGeometryDesc[currIndex], indexOffset, pcmd->ElemCount, vertexOffset, 0, 1);

                indexOffset += pcmd->ElemCount;
            }
            vertexOffset += cmd_list->VtxBuffer.Size;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

ImGuiWigetLayout::ImGuiWigetLayout(WigetHandler* handler, const std::string& title) noexcept
    : WigetLayout(handler, title)
{
}

void ImGuiWigetLayout::update(f32 dt)
{
    ImGui::SetNextWindowPos(ImVec2(static_cast<f32>(m_position.m_x), static_cast<f32>(m_position.m_y)), ImGuiCond_None);
    ImGui::SetNextWindowSize(ImVec2(static_cast<f32>(m_size.m_width), static_cast<f32>(m_size.m_height)), ImGuiCond_None);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin(m_title.c_str(), 0, flags))
    {
        WigetLayout::update(dt);
        ImGui::End();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

ImGuiWigetMenuLayout::ImGuiWigetMenuLayout(WigetHandler* handler) noexcept
    : WigetLayout(handler)
{
}

void ImGuiWigetMenuLayout::update(f32 dt)
{
    if (ImGui::BeginMainMenuBar())
    {
        WigetLayout::update(dt);

        //if (ImGui::BeginMenu("File"))
        //{
        //    ImGui::EndMenu();
        //}

        ////TODO
        //if (ImGui::BeginMenu("File"))
        //{
        //    if (ImGui::MenuItem("New")) {}
        //    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
        //    if (ImGui::BeginMenu("Open Recent"))
        //    {
        //        ImGui::MenuItem("fish_hat.c");
        //        ImGui::MenuItem("fish_hat.inl");
        //        ImGui::MenuItem("fish_hat.h");
        //        if (ImGui::BeginMenu("More.."))
        //        {
        //            ImGui::MenuItem("Hello");
        //            ImGui::MenuItem("Sailor");
        //            if (ImGui::BeginMenu("Recurse.."))
        //            {
        //                ImGui::EndMenu();
        //            }
        //            ImGui::EndMenu();
        //        }
        //        ImGui::EndMenu();
        //    }
        //    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
        //    if (ImGui::MenuItem("Save As..")) {}


        //    ImGui::EndMenu();
        //}
        //if (ImGui::BeginMenu("Edit"))
        //{
        //    if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
        //    if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
        //    ImGui::Separator();
        //    if (ImGui::MenuItem("Cut", "CTRL+X")) {}
        //    if (ImGui::MenuItem("Copy", "CTRL+C")) {}
        //    if (ImGui::MenuItem("Paste", "CTRL+V")) {}
        //    ImGui::EndMenu();
        //}

        ImGui::EndMainMenuBar();
    }
}

} //namespace ui
} //namespace v3d

#endif //USE_IMGUI