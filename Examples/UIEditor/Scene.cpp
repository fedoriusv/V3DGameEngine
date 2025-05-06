#include "Scene.h"

#include "Resource/Model.h"
#include "Resource/Bitmap.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/ModelFileLoader.h"
#include "Resource/ShaderCompiler.h"

#include "Scene/Model.h"
#include "Scene/Mesh.h"

#include "Renderer/Render.h"
#include "Renderer/Device.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "Renderer/ShaderProgram.h"


#include "Stream/StreamManager.h"

namespace v3d
{

scene::Transform g_modelTransform;

void Scene::loadResources()
{
    //default
}

Scene::DrawData Scene::loadCube(renderer::Device* device, const renderer::RenderPassDesc& renderpassDesc)
{
    renderer::CmdListRender* cmdList = device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);

    //Material
    v3d::renderer::ShaderProgram* program = nullptr;
    {
        const renderer::VertexShader* vertShader = resource::ResourceManager::getInstance()->loadShader<renderer::VertexShader, resource::ShaderSourceFileLoader>(device, "lit_solid_color.hlsl", "main_vs");
        const renderer::FragmentShader* fragShader = resource::ResourceManager::getInstance()->loadShader<renderer::FragmentShader, resource::ShaderSourceFileLoader>(device, "lit_solid_color.hlsl", "main_ps");
        program = new renderer::ShaderProgram(device, vertShader, fragShader);
    }

    //resource::Bitmap* image = resource::ResourceManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>("basetex.jpg");

    //renderer::SamplerState* sampler = new renderer::SamplerState(device, renderer::SamplerFilter::SamplerFilter_Bilinear, renderer::SamplerAnisotropic::SamplerAnisotropic_4x);
    //sampler->setWrap(renderer::SamplerWrap::TextureWrap_Repeat);

    //renderer::Texture2D* texture = new renderer::Texture2D(device, renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
    //    image->getFormat(), math::Dimension2D(image->getDimension().m_width, image->getDimension().m_height), image->getMipmapsCount());
    //cmdList->uploadData(texture, image->getSize(), image->getBitmap());

    //Geometry
    scene::Model* model = nullptr;
    {
        resource::ModelResource* modelRes = resource::ResourceManager::getInstance()->load<resource::ModelResource, resource::ModelFileLoader>("cube.dae", resource::ModelFileLoader::ModelLoaderFlag::SkipTangentAndBitangent);
        model = scene::ModelHelper::createModel(device, cmdList, modelRes);
        resource::ResourceManager::getInstance()->remove(modelRes);
    }

    //Pipeline
    v3d::renderer::GraphicsPipelineState* pipeline = nullptr;
    {
        const v3d::renderer::VertexInputAttributeDesc& attrib = model->m_geometry[0]._LODs[0]->getVertexAttribDesc(); //load from json

        pipeline = new renderer::GraphicsPipelineState(device, attrib, renderpassDesc, program, "TextureRender");
        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_Back);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
        pipeline->setDepthWrite(true);
        pipeline->setDepthTest(true);
    }

    device->submit(cmdList, true);
    device->destroyCommandList(cmdList);

    return DrawData{
        program,
        pipeline,
        model->m_geometry[0]._LODs[0]->m_indexBuffer,
        model->m_geometry[0]._LODs[0]->m_vertexBuffer[0]
    };
}

void Scene::drawCube(renderer::Device* device, renderer::CmdListRender* cmdList, const DrawData& data, scene::CameraHandler* camera)
{
    cmdList->setPipelineState(*data.m_Pipeline);

    struct ViewportBuffer
    {
        v3d::math::Matrix4D projectionMatrix;
        v3d::math::Matrix4D viewMatrix;
        v3d::math::Vector4D cameraPosition;
        v3d::math::Vector4D viewportSize;
    };
    ViewportBuffer constantBuffer0;
    constantBuffer0.cameraPosition = camera->getPosition();
    constantBuffer0.projectionMatrix = camera->getCamera().getProjectionMatrix();
    constantBuffer0.viewMatrix = camera->getCamera().getViewMatrix();
    constantBuffer0.viewportSize = { 0, 0, 0.f, 0.f };
    cmdList->bindDescriptorSet(0, { renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer0, 0, sizeof(constantBuffer0)}, 0) });


    struct Light
    {
        v3d::math::Vector4D lightPos;
        v3d::math::Vector4D color;
    };
    Light constantBuffer1;
    constantBuffer1.lightPos = math::Vector4D(25.0f, 0.0f, 5.0f, 1.0f);
    constantBuffer1.color = math::Vector4D(1.0f);
    cmdList->bindDescriptorSet(0, { renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer1, 0, sizeof(constantBuffer1)}, 1) });


    struct UniformBuffer
    {
        v3d::math::Matrix4D modelMatrix;
        v3d::math::Matrix4D normalMatrix;
    };
    UniformBuffer constantBuffer2;
    constantBuffer2.modelMatrix = g_modelTransform.getTransform();
    constantBuffer2.normalMatrix = constantBuffer2.modelMatrix.getTransposed();
    cmdList->bindDescriptorSet(1, { renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer2, 0, sizeof(constantBuffer2)}, 2) });

    v3d::renderer::GeometryBufferDesc desc(data.m_IdxBuffer, 0, data.m_VtxBuffer, 0, 32, 0);
    cmdList->drawIndexed(desc, 0, 36, 0, 0, 1);
}

Scene::DrawData Scene::loadTriangle(renderer::Device* device, const renderer::RenderPassDesc& renderpassDesc)
{
    const renderer::VertexShader* vertShader = nullptr;
    {
        const std::string vertexSource("\
        struct VS_INPUT\n\
        {\n\
            float3 Position : POSITION;\n\
            float3 Color    : COLOR;\n\
        };\n\
        \n\
        struct VS_OUTPUT\n\
        {\n\
            float4 Pos   : SV_POSITION;\n\
            float4 Color : COLOR;\n\
        };\n\
        \n\
        struct CBuffer\n\
        {\n\
            matrix projectionMatrix;\n\
            matrix modelMatrix; \n\
            matrix viewMatrix; \n\
        }; \n\
        \n\
        ConstantBuffer<CBuffer> buffer;\n\
        \n\
        VS_OUTPUT main(VS_INPUT Input)\n\
        {\n\
            VS_OUTPUT Out;\n\
            Out.Pos = mul(buffer.modelMatrix, float4(Input.Position, 1.0));\n\
            Out.Pos = mul(buffer.viewMatrix, Out.Pos);\n\
            Out.Pos = mul(buffer.projectionMatrix, Out.Pos);\n\
            Out.Color = float4(Input.Color, 1.0);\n\
            return Out;\n\
        }");
        const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

        resource::ShaderDecoder::ShaderPolicy vertexPolicy;
        vertexPolicy._type = renderer::ShaderType::Vertex;
        vertexPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
        vertexPolicy._content = renderer::ShaderContent::Source;
        vertexPolicy._entryPoint = "main";

        vertShader = resource::ShaderCompiler::compileShader<renderer::VertexShader>(device, "vertex", vertexPolicy, vertexStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        stream::StreamManager::destroyStream(vertexStream);
    }

    const renderer::FragmentShader* fragShader = nullptr;
    {
        const std::string fragmentSource("\
        struct PS_INPUT\n\
        {\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        float4 main(PS_INPUT Input) : SV_TARGET0\n\
        {\n\
            return Input.Col;\n\
        }");
        const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

        resource::ShaderDecoder::ShaderPolicy fragmentPolicy;
        fragmentPolicy._type = renderer::ShaderType::Fragment;
        fragmentPolicy._shaderModel = renderer::ShaderModel::HLSL_6_2;
        fragmentPolicy._content = renderer::ShaderContent::Source;
        fragmentPolicy._entryPoint = "main";

        fragShader = resource::ShaderCompiler::compileShader<renderer::FragmentShader>(device, "fragment", fragmentPolicy, fragmentStream, resource::ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV);
        stream::StreamManager::destroyStream(fragmentStream);
    }

    ASSERT(vertShader && fragShader, "nullptr");
    v3d::renderer::ShaderProgram* program = new renderer::ShaderProgram(device, vertShader, fragShader);

    std::vector<math::Vector3D> geometryData =
    {
        {-1.0f,-1.0f, 0.0f },  { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },  { 0.0f, 1.0f, 0.0f },
        { 1.0f,-1.0f, 0.0f },  { 0.0f, 0.0f, 1.0f },
    };
    v3d::renderer::VertexBuffer* geometry = new renderer::VertexBuffer(device, renderer::Buffer_GPUOnly, static_cast<u32>(geometryData.size()), static_cast<u32>(geometryData.size() * sizeof(math::Vector3D)), "geometry");

    renderer::VertexInputAttributeDesc vertexDesc(
        {
            renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(math::Vector3D) + sizeof(math::Vector3D)),
        },
        {
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, 0),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, sizeof(math::Vector3D)),
        });

        v3d::renderer::GraphicsPipelineState* pipeline = new renderer::GraphicsPipelineState(device, vertexDesc, renderpassDesc, program);
        pipeline->setPrimitiveTopology(renderer::PrimitiveTopology::PrimitiveTopology_TriangleList);
        pipeline->setFrontFace(renderer::FrontFace::FrontFace_Clockwise);
        pipeline->setCullMode(renderer::CullMode::CullMode_None);
        pipeline->setColorMask(renderer::ColorMask::ColorMask_All);
        pipeline->setDepthCompareOp(renderer::CompareOperation::CompareOp_GreaterOrEqual);
        pipeline->setDepthWrite(true);
        pipeline->setDepthTest(true);

        renderer::CmdListRender* cmdList =device->createCommandList<renderer::CmdListRender>(renderer::Device::GraphicMask);
        cmdList->uploadData(geometry, 0, static_cast<u32>(geometryData.size() * sizeof(math::Vector3D)), geometryData.data());
        device->submit(cmdList, true);
        device->destroyCommandList(cmdList);

        return DrawData{
            program,
            pipeline,
            nullptr,
            geometry
        };
}

void Scene::drawTriangle(renderer::Device* device, renderer::CmdListRender* cmdList, const DrawData& data, scene::CameraHandler* camera)
{
    //update uniforms
    struct UBO
    {
        math::Matrix4D projectionMatrix;
        math::Matrix4D modelMatrix;
        math::Matrix4D viewMatrix;
    };

    cmdList->setPipelineState(*data.m_Pipeline);

    UBO ubo1;
    ubo1.projectionMatrix = camera->getProjectionMatrix();
    ubo1.modelMatrix.setTranslation(math::Vector3D(-1, 0, 0));
    ubo1.viewMatrix = camera->getViewMatrix();

    renderer::Descriptor desc1(renderer::Descriptor::Descriptor_ConstantBuffer);
    desc1._resource = renderer::Descriptor::ConstantBuffer{ &ubo1, 0, sizeof(UBO) };

    cmdList->bindDescriptorSet(0, { desc1 });
    cmdList->draw(renderer::GeometryBufferDesc(data.m_VtxBuffer, 0, sizeof(math::Vector3D) + sizeof(math::Vector3D)), 0, 3, 0, 1);

    UBO ubo2;
    ubo2.projectionMatrix = camera->getProjectionMatrix();
    ubo2.modelMatrix.setTranslation(math::Vector3D(1, 0, 0));
    ubo2.viewMatrix = camera->getViewMatrix();

    renderer::Descriptor desc2(renderer::Descriptor::Descriptor_ConstantBuffer);
    desc2._resource = renderer::Descriptor::ConstantBuffer{ &ubo2, 0, sizeof(UBO) };

    cmdList->bindDescriptorSet(0, { desc2 });
    cmdList->draw(renderer::GeometryBufferDesc(data.m_VtxBuffer, 0, sizeof(math::Vector3D) + sizeof(math::Vector3D)), 0, 3, 0, 1);
}

} //namespace v3d