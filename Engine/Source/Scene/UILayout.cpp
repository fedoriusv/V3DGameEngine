#include "UILayout.h"
#include "Utils/Logger.h"
#include "Platform/Window.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Events/InputEventHandler.h"
#include "Events/InputEventReceiver.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Stream/StreamManager.h"
#include "Resource/ShaderCompiler.h"

#include "ThirdParty/imgui/imgui.h"

namespace v3d
{
namespace scene
{

UILayout::UILayout(renderer::Device* device)
    : m_device(device)
    , m_IMGUIContext(nullptr)
{
}

UILayout::~UILayout()
{
    ImGui::DestroyContext(m_IMGUIContext);
}

void UILayout::init(v3d::renderer::RenderTargetState* renderTarget)
{
    v3d::renderer::CmdListRender* cmdList = m_device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    ImGui::SetAllocatorFunctions(
        [](size_t size, void* userData) -> void*
        {
            return V3D_MALLOC(size, v3d::memory::MemoryLabel::MemorySystem);
        },
        [](void* ptr, void* userData)
        {
            V3D_FREE(ptr, v3d::memory::MemoryLabel::MemorySystem);
        });
    m_IMGUIContext = ImGui::CreateContext();

    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imguiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    imguiIO.Fonts->AddFontDefault();

    //Font texture
    int width, height;
    unsigned char* pixels = nullptr;
    imguiIO.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    m_fontSampler = new renderer::SamplerState(m_device, renderer::SamplerFilter::SamplerFilter_Trilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_None);
    m_fontSampler->setWrap(renderer::SamplerWrap::TextureWrap_ClampToEdge);
    m_fontSampler->setBorderColor(math::Vector4D(1.0, 1.0, 1.0, 1.0)); //opaque white

    m_fontAtlas = new renderer::Texture2D(m_device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Write, renderer::Format::Format_R8G8B8A8_UNorm, math::Dimension2D(width, height), 1);
    cmdList->uploadData(m_fontAtlas, width * height * 4, pixels);
    m_device->submit(cmdList, true);


    // Color scheme
    /*ImGuiStyle& style = ImGui::GetStyle();
    style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImGui::StyleColorsClassic();
    ImGui::StyleColorsDark();*/
    ImGui::StyleColorsLight();


    createResources(renderTarget);

    //ImGui::SetCurrentContext(m_IMGUIContext);


}
void UILayout::render(renderer::CmdListRender* cmdList)
{
    ImDrawData* imDrawData = ImGui::GetDrawData();
    ImGuiIO& imguiIO = ImGui::GetIO();

    u32 vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    u32 indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    //TODO recreate
    ASSERT(vertexBufferSize <= m_UIVertexBuffer->getSize(), "out of size");
    ASSERT(indexBufferSize <= m_UIIndexBuffer->getIndicesCount() * sizeof(u16), "out of size");

    if ((vertexBufferSize != 0) && (indexBufferSize != 0)) 
    {
        u32 vtxDst = m_offsetVB;
        u32 idxDst = m_offsetIB;
        for (int n = 0; n < imDrawData->CmdListsCount; n++) 
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[n];

            cmdList->uploadData(m_UIIndexBuffer, idxDst, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), cmd_list->IdxBuffer.Data);
            cmdList->uploadData(m_UIVertexBuffer, vtxDst, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), cmd_list->VtxBuffer.Data);

            vtxDst += cmd_list->VtxBuffer.Size;
            idxDst += cmd_list->IdxBuffer.Size;

            //m_offsetVB = vtxDst;
            //m_offsetIB = idxDst;
        }
    }

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

                m_UIGeometryDesc._indexOffset = m_offsetIB;
                m_UIGeometryDesc._offsets[0] = m_offsetVB;
                cmdList->drawIndexed(m_UIGeometryDesc, indexOffset, pcmd->ElemCount, vertexOffset, 0, 1);

                indexOffset += pcmd->ElemCount;
            }
            vertexOffset += cmd_list->VtxBuffer.Size;
        }
    }
}
void UILayout::update(platform::Window* window, event::InputEventHandler* handler)
{
    //Imgui
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.DeltaTime = 1.0f / 60.0f;
    imguiIO.DisplaySize.x = window->getSize().m_width;
    imguiIO.DisplaySize.y = window->getSize().m_height;

    if (handler)
    {
        imguiIO.AddMousePosEvent(handler->getCursorPosition().m_x, handler->getCursorPosition().m_y);
        imguiIO.AddMouseButtonEvent(0, handler->isLeftMousePressed());
        imguiIO.AddMouseButtonEvent(1, handler->isRightMousePressed());
    }

    ImGui::NewFrame();

/*    ImGui::SetWindowPos(ImVec2(20 * 1, 20 * 1), ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2(300 * 1, 300 * 1), ImGuiCond_Always);
    ImGui::TextUnformatted("TEST");
    ImGui::Text("Test")*/;

    ImGui::ShowDemoWindow();
    ImGui::EndFrame();

    ImGui::Render();
}

void UILayout::createResources(v3d::renderer::RenderTargetState* renderTarget)
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
        m_UIProgram = new renderer::ShaderProgram(m_device, vertShader, fragShader);
    }

    //pipeline
    const u32 UIStride = sizeof(math::Vector2D) + sizeof(math::Vector2D) + sizeof(math::Color4D);
    {
        renderer::VertexInputAttributeDesc vertexDesc(
            {
                renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, UIStride),
            },
            {
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, 0),
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, sizeof(math::Vector2D)),
                renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R8G8B8A8_UNorm, sizeof(math::Color4D)),
            }
        );

        m_UIPipeline = new renderer::GraphicsPipelineState(m_device, vertexDesc, m_UIProgram, renderTarget, "UIPipeline");
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

    //geometry
    {
        const u32 maxIndexCount = 5000;
        const u32 maxVertexCount = 5000;
        m_UIIndexBuffer = new renderer::IndexBuffer(m_device, renderer::Buffer_GPUWrite /*| renderer::Buffer_Dynamic*/, renderer::IndexBufferType::IndexType_16, maxIndexCount, "UIIndexBuffer");
        m_UIVertexBuffer = new renderer::VertexBuffer(m_device, renderer::Buffer_GPUWrite /*| renderer::Buffer_Dynamic*/, maxVertexCount, maxVertexCount * UIStride, "UIVertexBuffer");

        m_UIGeometryDesc = renderer::GeometryBufferDesc(m_UIIndexBuffer, 0, m_UIVertexBuffer, 0, UIStride, 0);
    }
}

} //namespace scene
} //namespace v3d