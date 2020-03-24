#include "MyApplication.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Formats.h"
#include "Renderer/Shader.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ShaderSourceStreamLoader.h"
#include "Resource/ShaderBinaryFileLoader.h"

#include "Stream/StreamManager.h"

#include <random>


using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;
using namespace v3d::renderer;
using namespace v3d::resource;


MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
    , m_InputEventHandler(nullptr)

    , m_Context(nullptr)
    , m_CommandList(nullptr)
{
    m_Window = Window::createWindow({ 1024, 768 }, {800, 500}, false, new v3d::event::InputEventReceiver());
    ASSERT(m_Window, "windows is nullptr");

    m_InputEventHandler = new InputEventHandler();
    m_InputEventHandler->connect([this](const MouseInputEvent* event)
    {
        if (event->_event == MouseInputEvent::MousePressDown || event->_event == MouseInputEvent::MouseDoubleClick)
        {
            s32 rvalue = std::rand();
            f32 r = 1.0f / RAND_MAX * rvalue;

            s32 gvalue = std::rand();
            f32 g = 1.0f / RAND_MAX * gvalue;

            s32 bvalue = std::rand();
            f32 b = 1.0f / RAND_MAX * bvalue;

            m_clearColor = {r, g, b, 1.0f };

        }
    });

    std::srand(u32(std::time(0)));
    m_Window->getInputEventReceiver()->attach(InputEvent::InputEventType::MouseInputEvent, m_InputEventHandler);
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    bool running = true;
    while (running)
    {
        running = Window::updateWindow(m_Window);

        m_Window->getInputEventReceiver()->sendDeferredEvents();
        if (running)
        {
            MyApplication::Running(*m_CommandList);
        }
    }

    Exit();
    delete this;

    return 0;
}

void MyApplication::Initialize()
{
    Context::RenderType renderTypes[2] = { Context::RenderType::VulkanRender, Context::RenderType::DirectXRender };
    for (Context::RenderType renderType : renderTypes)
    {
        m_Context = Context::createContext(m_Window, renderType);
        ASSERT(m_Context, "context is nullptr");

        m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::DelayedCommandList);

        Test_ShaderLoader();

        delete m_CommandList;
        Context::destroyContext(m_Context);
    }
    //Test_MemoryPool();

    //Texture2D* texture = m_CommandList->createObject<Texture2D>(renderer::Format::Format_R8G8B8A8_UInt, core::Dimension2D(1024, 768), renderer::TextureSamples::TextureSamples_x1);
    //renderTarget0 = m_CommandList->createObject<RenderTarget>(texture->getDimension());
    //bool success = renderTarget0->setColorTexture(0, texture, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store);
    //m_CommandList->setRenderTarget(renderTarget0);

    ///*Texture2D* texture2 = m_CommandList->createObject<Texture2D>(renderer::Format::Format_R8G8B8A8_UInt, core::Dimension2D(80, 80), renderer::TextureSamples::TextureSamples_x1);
    //renderTarget1 = m_CommandList->createObject<RenderTarget>(core::Dimension2D(80, 80));
    //bool success1 = renderTarget1->setColorTexture(0, texture2, renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_Store);
    //m_CommandList->setRenderTarget(renderTarget1);*/

    //Shader* vertShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "Shaders/mrt.vert");
    //Shader* fragShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "Shaders/mrt.frag");
    ////Shader* fragShader = ResourceLoaderManager::getInstance()->loadResource<Shader, ShaderSourceFileLoader(m_Context, name, header, defines)>();
    //
    //ShaderProgram* program = m_CommandList->createObject<ShaderProgram>(std::vector<Shader*>{vertShader, fragShader});
    //renderer::VertexInputAttribDescription vertexDesc;

    //GraphicsPipelineState* pipeline = m_CommandList->createObject<GraphicsPipelineState>(vertexDesc, program, renderTarget0);

    //pipeline->setPrimitiveTopology(PrimitiveTopology::PrimitiveTopology_TriangleList);
    //m_CommandList->setPipelineState(pipeline);

    //core::Matrix4D projection;

    //program->bindTexture<Texture2D, ShaderType::ShaderType_Fragment>("samplerColor", texture);
    //program->bindUniform<core::Matrix4D, ShaderType::ShaderType_Fragment>("projection", projection);

    //std::vector<f32> vertexBuffer =
    //{
    //     1.0f,  1.0f, 0.0f,
    //     -1.0f,  1.0f, 0.0f,
    //      0.0f, -1.0f, 0.0f
    //};
    //u64 vertexBufferSize = vertexBuffer.size() * sizeof(f32);
    //VertexStreamBuffer* streamBuffer = m_CommandList->createObject<VertexStreamBuffer>(StreamBuffer_Write | StreamBuffer_Shared,  vertexBufferSize, (u8*)vertexBuffer.data());
    //
    ///*Geometry geometry;
    //Image image;
    //Pipeline pipe;
    //{
    //    texture.upload(image);
    //    geometry.upload(data);
    //}*/

    m_clearColor = { 1.0, 0.0, 0.0, 1.0 };
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    //commandList.beginFrame();
    //commandList.clearBackbuffer(m_clearColor);

    ////m_CommandList->draw(StreamBufferDescription(streamBuffer, 0, 3), 1);

    ////m_CommandList->setRenderTarget(renderTarget0);
    ////commandList.flushCommands();
    ////m_CommandList->setRenderTarget(renderTarget1);

    ///*commandList.set(pipe1);
    //commandList.set(texture1);
    //commandList.set(uniform1);
    //commandList.draw(geometry1);

    //commandList.set(pipe2);
    //commandList.set(texture2);
    //commandList.set(uniform2);
    //commandList.draw(geometry2);*/

    //commandList.endFrame();
    //commandList.presentFrame();
    //
    //commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
}

void MyApplication::Test_MemoryPool()
{
    {
        MemoryPool pool;

        char* a = (char*)pool.getMemory(253);
        memset(a, 'a', 253);
        u64 ofsa = pool.getOffsetInBlock(a);
        char* b = (char*)pool.getMemory(253);
        memset(b, 'b', 253);
        u64 ofsb = pool.getOffsetInBlock(b);
        char* c = (char*)pool.getMemory(253);
        memset(c, 'c', 253);
        u64 ofsc = pool.getOffsetInBlock(c);
        char* d = (char*)pool.getMemory(253);
        memset(d, 'd', 253);
        u64 ofsd = pool.getOffsetInBlock(d);
        char* e = (char*)pool.getMemory(253);
        memset(e, 'e', 253);
        u64 ofse = pool.getOffsetInBlock(e);
        char* f = (char*)pool.getMemory(253);
        memset(f, 'f', 10);
        u64 ofsf = pool.getOffsetInBlock(f);
        ////

        pool.freeMemory((void*)b);
        char* nb = (char*)pool.getMemory(253);
        memset(b, 'B', 253);
        u64 nofsb = pool.getOffsetInBlock(b);

        void* hugeData = pool.getMemory(1024 * 1024 * 4);
        u64 ofhugeData = pool.getOffsetInBlock(hugeData);

        void* hugeData1 = pool.getMemory(1024 * 1024 * 40);
        u64 ofhugeData1 = pool.getOffsetInBlock(hugeData1);

        pool.freeMemory(hugeData1);

        void* hugeData2 = pool.getMemory(1024 * 1024 * 400);
        u64 ofhugeData2 = pool.getOffsetInBlock(hugeData2);

        pool.clearPools();
    }

    {
        auto randomFunc = [](u32 min, u32 max) -> u32
        {
            std::srand(u32(std::time(nullptr)));
            s32 rnd = std::rand();
            return (u32)(rnd % (max - min + 1) + min);
         };


        MemoryPool pool(2048, 32, 32, utils::MemoryPool::getDefaultMemoryPoolAllocator());
        std::vector<std::pair<u32, void*>> sizes;

        std::random_device rd;
        std::mt19937 g(rd());

        for (u32 e = 0; e < 100; ++e)
        {
            std::uniform_int_distribution<u32> uid(1, 512);
            for (u32 i = 0; i < 1000; ++i)
            {
                u32 rendomSize = uid(g);
                void* ptr = pool.getMemory(rendomSize);
                sizes.push_back(std::make_pair(rendomSize, ptr));
            }

            std::shuffle(sizes.begin(), sizes.end(), g);

            for (u32 i = 0; i < 1000; ++i)
            {
                void* ptr = sizes[i].second;
                pool.freeMemory(ptr);
            }
        }

        //pool.clearPools();
    }

    int test;
}

void MyApplication::Test_ShaderLoader()
{
    auto checkVertexShaderReflection = [](const Shader::ReflectionInfo& info)
    {
        return;

        ASSERT(info._inputAttribute.size() == 3, "wrong");
        ASSERT(info._inputAttribute[0]._location == 0, "wrong");
        ASSERT(info._inputAttribute[0]._format == Format::Format_R32G32B32_SFloat, "wrong");
        ASSERT(info._inputAttribute[1]._location == 1, "wrong");
        ASSERT(info._inputAttribute[1]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._inputAttribute[2]._location == 2, "wrong");
        ASSERT(info._inputAttribute[2]._format == Format::Format_R32G32_SFloat, "wrong");

        ASSERT(info._outputAttribute.size() == 2, "wrong");
        ASSERT(info._outputAttribute[0]._location == 0, "wrong");
        ASSERT(info._outputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._outputAttribute[1]._location == 1, "wrong");
        ASSERT(info._outputAttribute[1]._format == Format::Format_R32G32_SFloat, "wrong");

        ASSERT(info._uniformBuffers.size() == 3, "wrong");
        ASSERT(info._uniformBuffers[0]._id == 1, "wrong");
        ASSERT(info._uniformBuffers[0]._set == 0, "wrong");
        ASSERT(info._uniformBuffers[0]._binding == 0, "wrong");
        ASSERT(info._uniformBuffers[0]._array == 1, "wrong");
        ASSERT(info._uniformBuffers[0]._size == 64, "wrong");
        ASSERT(info._uniformBuffers[0]._name == "ubo00_size64", "wrong");
        ASSERT(info._uniformBuffers[0]._uniforms.size() == 1, "wrong");
        {
            ASSERT(info._uniformBuffers[0]._uniforms[0]._bufferId == 1, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[0]._uniforms[0]._offset == 0, "wrong");
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == 0, "wrong");
        }
        ASSERT(info._uniformBuffers[1]._id == 0, "wrong");
        ASSERT(info._uniformBuffers[1]._set == 0, "wrong");
        ASSERT(info._uniformBuffers[1]._binding == 1, "wrong");
        ASSERT(info._uniformBuffers[1]._array == 1, "wrong");
        ASSERT(info._uniformBuffers[1]._size == 192, "wrong");
        ASSERT(info._uniformBuffers[1]._name == "ubo01_size192", "wrong");
        ASSERT(info._uniformBuffers[1]._uniforms.size() == 3, "wrong");
        {
            ASSERT(info._uniformBuffers[1]._uniforms[0]._bufferId == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[0]._offset == 0, "wrong");
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._bufferId == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._offset == 64, "wrong");
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._bufferId == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._offset == 128, "wrong");
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == 0, "wrong");
        }
        ASSERT(info._uniformBuffers[2]._id == 2, "wrong");
        ASSERT(info._uniformBuffers[2]._set == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._binding == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._array == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._size == 128 * 2, "wrong");
        ASSERT(info._uniformBuffers[2]._name == "ubo11_size128", "wrong");
        ASSERT(info._uniformBuffers[2]._uniforms.size() == 2, "wrong");
        {
            ASSERT(info._uniformBuffers[2]._uniforms[0]._bufferId == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._array == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._size == 64 * 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._offset == 0, "wrong");
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == 0, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._bufferId == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._array == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._size == 64 * 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._offset == 64 * 2, "wrong");
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == 0, "wrong");
        }
    };

    auto checkFragmentShaderReflection = [](const Shader::ReflectionInfo& info)
    {
        ASSERT(info._inputAttribute.size() == 2, "wrong");
        ASSERT(info._inputAttribute[0]._location == 0, "wrong");
        ASSERT(info._inputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._inputAttribute[1]._location == 1, "wrong");
        ASSERT(info._inputAttribute[1]._format == Format::Format_R32G32_SFloat, "wrong");

        ASSERT(info._outputAttribute.size() == 1, "wrong");
        ASSERT(info._outputAttribute[0]._location == 0, "wrong");
        ASSERT(info._outputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");

        ASSERT(info._samplers.size() == 1, "wrong");
        ASSERT(info._samplers[0]._set == 0, "wrong");
        ASSERT(info._samplers[0]._binding == 0, "wrong");
        ASSERT(info._samplers[0]._name == "samplerColor00", "wrong");

        ASSERT(info._images.size() == 3, "wrong");
        ASSERT(info._images[0]._set == 0, "wrong");
        ASSERT(info._images[0]._binding == 1, "wrong");
        ASSERT(info._images[0]._array == 1, "wrong");
        ASSERT(info._images[0]._target == TextureTarget::Texture2D, "wrong");
        ASSERT(info._images[0]._ms == false, "wrong");
        ASSERT(info._images[0]._depth == false, "wrong");
        ASSERT(info._images[0]._name == "textureColor01", "wrong");
        ASSERT(info._images[1]._set == 0, "wrong");
        ASSERT(info._images[1]._binding == 2, "wrong");
        ASSERT(info._images[1]._array == 1, "wrong");
        ASSERT(info._images[1]._target == TextureTarget::Texture2D, "wrong");
        ASSERT(info._images[1]._ms == false, "wrong");
        ASSERT(info._images[1]._depth == false, "wrong");
        ASSERT(info._images[1]._name == "textureColor02", "wrong");
        ASSERT(info._images[2]._set == 1, "wrong");
        ASSERT(info._images[2]._binding == 0, "wrong");
        ASSERT(info._images[2]._array == 1, "wrong");
        ASSERT(info._images[2]._target == TextureTarget::Texture2D, "wrong");
        ASSERT(info._images[2]._ms == false, "wrong");
        ASSERT(info._images[2]._depth == false, "wrong");
        ASSERT(info._images[2]._name == "textureColor10", "wrong");
    };

    //load source shaders from file
    if (m_Context->getRenderType() == Context::RenderType::VulkanRender)
    {
        /*
        #version 450

        layout (location = 0) in vec3 inAttribute0_vec3;
        layout (location = 1) in vec4 inAttribute1_vec4;
        layout (location = 2) in vec2 inAttribute2_vec2;

        layout (set = 0, binding = 1, std140) uniform UBO01_size192
        {
            mat4 projectionMatrix;
            mat4 modelMatrix;
            mat4 viewMatrix;
        } ubo01_size192;

        layout (set = 0, binding = 0, std140) uniform UBO00_size64
        {
            mat4 projectionMatrix;
        } ubo00_size64;

        layout (set = 1, binding = 1, std140) uniform UBO11_size128
        {
            mat4 modelMatrix[2];
            mat4 viewMatrix[2];
        } ubo11_size128;

        layout (location = 0) out vec4 outAttribute0_vec4;
        layout (location = 1) out vec2 outAttribute1_vec2;

        void main()
        {
            outAttribute1_vec2 = inAttribute2_vec2;
            vec4 vertex = ubo01_size192.modelMatrix * vec4(inAttribute0_vec3, 1.0);
            outAttribute0_vec4 = ubo00_size64.projectionMatrix * ubo11_size128.viewMatrix[1] * vertex;

            gl_Position = outAttribute0_vec4;
        }
        */
        Shader* glslVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfo.vert");
        ASSERT(glslVShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = glslVShader->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        /*
        #version 450

        layout (location = 0) in vec4 inAttribute0_vec4;
        layout (location = 1) in vec2 inAttribute1_vec2;

        layout (set = 0, binding = 0) uniform sampler samplerColor00;

        layout (set = 0, binding = 2) uniform texture2D textureColor02;
        layout (set = 1, binding = 0) uniform texture2D textureColor10;
        layout (set = 0, binding = 1) uniform texture2D textureColor01;

        layout (location = 0) out vec4 outFragColor;

        void main()
        {
            outFragColor.r = texture(sampler2D(textureColor02, samplerColor00), inAttribute1_vec2).r;
            outFragColor.g = texture(sampler2D(textureColor10, samplerColor00), inAttribute1_vec2).g;
            outFragColor.b = texture(sampler2D(textureColor01, samplerColor00), inAttribute1_vec2).b;
            outFragColor.a = 0.0;
        }
        */
        Shader* glslFShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfo.frag");
        ASSERT(glslFShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = glslFShader->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceLoaderManager::getInstance()->remove(glslVShader);
        ResourceLoaderManager::getInstance()->remove(glslFShader);

        //load spirv
        Shader* spirvVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderBinaryFileLoader>(m_Context, "examples/test/shaders/testReflectInfo.vspv");
        ASSERT(spirvVShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = spirvVShader->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        Shader* spirvFShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderBinaryFileLoader>(m_Context, "examples/test/shaders/testReflectInfo.fspv");
        ASSERT(spirvFShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = spirvFShader->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceLoaderManager::getInstance()->remove(spirvVShader);
        ResourceLoaderManager::getInstance()->remove(spirvFShader);
    }

    {
        //hlsl
        /*
        struct VS_INPUT
        {
            float3 inAttribute0_vec3 : IN_ATTRIBUTE0;
            float4 inAttribute1_vec4 : IN_ATTRIBUTE1;
            float2 inAttribute2_vec2 : IN_ATTRIBUTE2;
        };

        struct UBO01_size192
        {
            float4x4 projectionMatrix;
            float4x4 modelMatrix;
            float4x4 viewMatrix;
        };

        struct UBO00_size64
        {
            float4x4 projectionMatrix;
        };

        struct UBO11_size128
        {
            float4x4 modelMatrix[2];
            float4x4 viewMatrix[2];
        };

        ConstantBuffer<UBO01_size192> ubo01_size192   : register(b1, space0);
        ConstantBuffer<UBO00_size64>  ubo00_size64    : register(b0, space0);
        ConstantBuffer<UBO11_size128> ubo11_size128   : register(b1, space1);


        struct VS_OUTPUT 
        {
           float4 Position: SV_POSITION;
           float4 outAttribute0_vec4: OUT_ATTRIBUTE0;
           float2 outAttribute1_vec2: OUT_ATTRIBUTE1;
        };

        VS_OUTPUT main(VS_INPUT Input)
        {
           VS_OUTPUT Output;
   
           Output.outAttribute1_vec2 = Input.inAttribute2_vec2;
   
           float4 vertex = mul(ubo01_size192.modelMatrix, float4(Input.inAttribute0_vec3, 1.0));
           Output.outAttribute0_vec4 = mul(ubo00_size64.projectionMatrix, vertex);
           Output.outAttribute0_vec4 = mul(ubo11_size128.viewMatrix[1], Output.outAttribute0_vec4);

           Output.Position = Output.outAttribute0_vec4;

           return Output;
        }
        */
        Shader* hlslVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfo.vs");
        ASSERT(hlslVShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = hlslVShader->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        /*
        struct PS_INPUT
        {
           float4 inAttribute0_vec4: IN_ATTRIBUTE0;
           float2 inAttribute1_vec2: IN_ATTRIBUTE1;
        };

        SamplerState samplerColor00 : register(s0, space0);

        Texture2D textureColor02    : register(t2, space0);
        Texture2D textureColor10    : register(t0, space1);
        Texture2D textureColor01    : register(t1, space0);

        float4 main(PS_INPUT Input) : SV_TARGET0
        {
            float4 OutColor;
            OutColor.r = textureColor02.Sample(samplerColor00, Input.inAttribute1_vec2).r;
            OutColor.g = textureColor10.Sample(samplerColor00, Input.inAttribute1_vec2).g;
            OutColor.b = textureColor01.Sample(samplerColor00, Input.inAttribute1_vec2).b;
            OutColor.a = 0.0;

            return OutColor;
        }
        */
        Shader* hlslPShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfo.ps");
        ASSERT(hlslPShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = hlslPShader->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceLoaderManager::getInstance()->remove(hlslVShader);
        ResourceLoaderManager::getInstance()->remove(hlslPShader);
    }

    {
        renderer::Shader* vertShader = nullptr;
        {
            const std::string vertexSource("\
        struct VS_INPUT\n\
        {\n\
            float3 Position : POSITION;\n\
            float3 Color : COLOR;\n\
        };\n\
        \n\
        struct VS_OUTPUT\n\
        {\n\
            float4 Pos : SV_POSITION;\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        cbuffer ConstantBuffer\n\
        {\n\
            matrix projectionMatrix;\n\
            matrix modelMatrix;\n\
            matrix viewMatrix;\n\
        };\n\
        \n\
        VS_OUTPUT main(VS_INPUT Input)\n\
        {\n\
            VS_OUTPUT Out;\n\
            Out.Pos = mul(modelMatrix, float4(Input.Position, 1.0));\n\
            Out.Pos = mul(viewMatrix, Out.Pos);\n\
            Out.Pos = mul(projectionMatrix, Out.Pos);\n\
            Out.Col = float4(Input.Color, 1.0);\n\
            return Out;\n\
        }");
            const stream::Stream* vertexStream = stream::StreamManager::createMemoryStream(vertexSource);

            renderer::ShaderHeader vertexHeader(renderer::ShaderType::ShaderType_Vertex);
            vertexHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
            vertexHeader._shaderLang = renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL;
            vertexHeader._apiVersion = 50;

            vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_Context, "vertex", &vertexHeader, vertexStream);
            ASSERT(vertShader, "nullptr");
        }

        renderer::Shader* fragShader = nullptr;
        {
            const std::string fragmentSource("\
        struct PS_INPUT\n\
        {\n\
            float4 Pos : SV_POSITION;\n\
            float4 Col : COLOR;\n\
        };\n\
        \n\
        float4 main(PS_INPUT Input) : SV_TARGET0\n\
        {\n\
            return Input.Col;\n\
        }");
            const stream::Stream* fragmentStream = stream::StreamManager::createMemoryStream(fragmentSource);

            renderer::ShaderHeader fragmentHeader(renderer::ShaderType::ShaderType_Fragment);
            fragmentHeader._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
            fragmentHeader._shaderLang = renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL;
            fragmentHeader._apiVersion = 50;

            fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<renderer::Shader, resource::ShaderSourceStreamLoader>(m_Context, "fragment", &fragmentHeader, fragmentStream);
            ASSERT(fragShader, "nullptr");
        }

        ResourceLoaderManager::getInstance()->remove(vertShader);
        ResourceLoaderManager::getInstance()->remove(fragShader);
    }
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
