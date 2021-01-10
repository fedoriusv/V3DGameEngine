#include "MyApplication.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"
#include "Utils/Timer.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/SamplerState.h"
#include "Renderer/Formats.h"
#include "Renderer/Shader.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/ShaderSourceStreamLoader.h"
#include "Resource/ShaderBinaryFileLoader.h"

#include "Stream/StreamManager.h"

#include "crc32c/crc32c.h"

#include <random>
#include <thread>


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
    m_Window = Window::createWindow({ 1024, 768 }, { 800, 500 }, false, new v3d::event::InputEventReceiver());
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
    /*std::thread test_thread([this]() -> void
        {
            Test_Timer();
        });

    test_thread.join();*/

    //Render test
    Context::RenderType renderTypes[2] = { Context::RenderType::VulkanRender, Context::RenderType::DirectXRender };
    for (Context::RenderType renderType : renderTypes)
    {
        m_Context = Context::createContext(m_Window, renderType);
        ASSERT(m_Context, "context is nullptr");

        m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::DelayedCommandList);

        Test_ShaderLoader();
        Test_CreateShaderProgram();
        Test_ShaderParam();
        Test_CreatePipeline();

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

    LOG_DEBUG("Tests have finished");
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

void MyApplication::Test_Timer()
{
    LOG_DEBUG("Test_Timer");

    {
        LOG_DEBUG("Test_Timer test 1");

        utils::Timer timer;
        ASSERT(timer.getTime<utils::Timer::Duration_NanoSeconds>() == 0, "Must be 0");

        timer.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.stop();

        u64 value0 = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        ASSERT(value0 == 10, "Must be 10");

        timer.reset();
        ASSERT(timer.getTime<utils::Timer::Duration_NanoSeconds>() == 0, "Must be 0");

        timer.start();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        timer.stop();

        u64 value1 = timer.getTime<utils::Timer::Duration_Seconds>();
        ASSERT(value1 == 2, "Must be 2");

        timer.start();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        timer.stop();

        u64 value2 = timer.getTime<utils::Timer::Duration_Seconds>();
        ASSERT(value2 == value1 + 3, "Must be 5");
    }

    {
        LOG_DEBUG("Test_Timer test 2");

        const u64 resetTime = 1000; //1 sec

        {
            auto start_time_w = std::chrono::high_resolution_clock::now();

            auto start_time = std::chrono::high_resolution_clock::now();
            auto end_time = start_time;

            u64 miliseconds = 0;
            while (true)
            {
                end_time = std::chrono::high_resolution_clock::now();
                u64 duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                if (resetTime <= duration)
                {
                    miliseconds += duration;
                    start_time = std::chrono::high_resolution_clock::now();
                    LOG_DEBUG("Test_Timer chrono %d miliseconds are passed. Summary %d", duration, miliseconds);
                }

                if (miliseconds >= resetTime * 10)
                {
                    break;
                }
            }

            auto end_time_w = std::chrono::high_resolution_clock::now();
            u64 duration_w = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_w - start_time_w).count();

            u64 error = miliseconds - (resetTime * 10);
            LOG_WARNING("Test_Timer chrono duration %d = 10 sec. Error %d ms", miliseconds, error);
        }

        {
            auto start_time_w = std::chrono::high_resolution_clock::now();

            auto start_time = std::chrono::high_resolution_clock::now();
            auto end_time = start_time;

            utils::Timer timer;
            timer.start();

            u64 miliseconds = 0;
            [[maybe_unused]] u32 loopsCount = 0;
            while (true)
            {
                u64 duration = timer.getTime<utils::Timer::Duration_MilliSeconds>();
                if (resetTime <= duration)
                {
                    miliseconds += duration;
                    timer.reset();
                    LOG_DEBUG("Test_Timer timer %d miliseconds are passed. Summary %d", duration, miliseconds);
                }

                end_time = std::chrono::high_resolution_clock::now();
                if (miliseconds >= resetTime * 10)
                {
                    break;
                }

                ++loopsCount;
            }

            timer.stop();

            auto end_time_w = std::chrono::high_resolution_clock::now();
            u64 duration_w = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_w - start_time_w).count();

            u64 error = miliseconds - (resetTime * 10);
            LOG_WARNING("Test_Timer timer duration %d = 10 sec. Error %d ms", miliseconds, error);
            ASSERT(miliseconds == resetTime * 10, "Must be 10 sec");
        }
    }

    [[maybe_unused]] u32 endTest = 0;
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
        ASSERT(info._inputAttribute.size() == 3, "wrong");
        ASSERT(info._inputAttribute[0]._location == 0, "wrong");
        ASSERT(info._inputAttribute[0]._format == Format::Format_R32G32B32_SFloat, "wrong");
        ASSERT(info._inputAttribute[1]._location == 1, "wrong");
        ASSERT(info._inputAttribute[1]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._inputAttribute[2]._location == 2, "wrong");
        ASSERT(info._inputAttribute[2]._format == Format::Format_R32G32_SFloat, "wrong");

        ASSERT(info._outputAttribute.size() == 3, "wrong");
        ASSERT(info._outputAttribute[0]._location == 0, "wrong");
        ASSERT(info._outputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._outputAttribute[1]._location == 1, "wrong");
        ASSERT(info._outputAttribute[1]._format == Format::Format_R32G32_SFloat, "wrong");
        ASSERT(info._outputAttribute[2]._location == 2, "wrong");
        ASSERT(info._outputAttribute[2]._format == Format::Format_R32G32B32_SFloat, "wrong");

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
            //ASSERT(info._uniformBuffers[0]._uniforms[0]._name == "matrix0_00", "wrong");
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
            //ASSERT(info._uniformBuffers[1]._uniforms[0]._name == "matrix0_01", "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._bufferId == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[1]._offset == 64, "wrong");
            //ASSERT(info._uniformBuffers[1]._uniforms[1]._name == "matrix1_01", "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._bufferId == 0, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._array == 1, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._size == 64, "wrong");
            ASSERT(info._uniformBuffers[1]._uniforms[2]._offset == 128, "wrong");
            //ASSERT(info._uniformBuffers[1]._uniforms[2]._name == "matrix2_01", "wrong");
        }
        ASSERT(info._uniformBuffers[2]._id == 2, "wrong");
        ASSERT(info._uniformBuffers[2]._set == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._binding == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._array == 1, "wrong");
        ASSERT(info._uniformBuffers[2]._size == 256, "wrong");
        ASSERT(info._uniformBuffers[2]._name == "ubo11_size256", "wrong");
        ASSERT(info._uniformBuffers[2]._uniforms.size() == 2, "wrong");
        {
            ASSERT(info._uniformBuffers[2]._uniforms[0]._bufferId == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._array == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._size == 64 * 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[0]._offset == 0, "wrong");
            //ASSERT(info._uniformBuffers[2]._uniforms[0]._name == "matrix0_11", "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._bufferId == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._array == 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._type == DataType::DataType_Matrix4, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._size == 64 * 2, "wrong");
            ASSERT(info._uniformBuffers[2]._uniforms[1]._offset == 64 * 2, "wrong");
            //ASSERT(info._uniformBuffers[2]._uniforms[1]._name == "matrix1_11", "wrong");
        }
    };

    auto checkFragmentShaderReflection = [](const Shader::ReflectionInfo& info)
    {
        ASSERT(info._inputAttribute.size() == 3, "wrong");
        ASSERT(info._inputAttribute[0]._location == 0, "wrong");
        ASSERT(info._inputAttribute[0]._format == Format::Format_R32G32B32A32_SFloat, "wrong");
        ASSERT(info._inputAttribute[1]._location == 1, "wrong");
        ASSERT(info._inputAttribute[1]._format == Format::Format_R32G32_SFloat, "wrong");
        ASSERT(info._inputAttribute[2]._location == 2, "wrong");
        ASSERT(info._inputAttribute[2]._format == Format::Format_R32G32B32_SFloat, "wrong");

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

    //vulkan only
    if (m_Context->getRenderType() == Context::RenderType::VulkanRender)
    {
        //load source shaders from file
        Shader* glslVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.vert");
        ASSERT(glslVShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = glslVShader->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        Shader* glslFShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.frag");
        ASSERT(glslFShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = glslFShader->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceLoaderManager::getInstance()->remove(glslVShader);
        ResourceLoaderManager::getInstance()->remove(glslFShader);

        //load spirv
        Shader* spirvVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderBinaryFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.vspv");
        ASSERT(spirvVShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = spirvVShader->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        Shader* spirvFShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderBinaryFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.fspv");
        ASSERT(spirvFShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = spirvFShader->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceLoaderManager::getInstance()->remove(spirvVShader);
        ResourceLoaderManager::getInstance()->remove(spirvFShader);
    }

    {
        Shader* hlslVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.vs");
        ASSERT(hlslVShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = hlslVShader->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        Shader* hlslPShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.ps");
        ASSERT(hlslPShader != nullptr, "wrong");
        {
            const Shader::ReflectionInfo& info = hlslPShader->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceLoaderManager::getInstance()->remove(hlslVShader);
        ResourceLoaderManager::getInstance()->remove(hlslPShader);
    }

    {
        std::vector<Shader*> hlslShaders = ResourceLoaderManager::getInstance()->loadHLSLShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.hlsl", { {"mainVS", ShaderType::ShaderType_Vertex}, {"mainPS", ShaderType::ShaderType_Fragment} });
        ASSERT(!hlslShaders.empty(), "wrong");

        {
            ASSERT(hlslShaders[0] != nullptr, "wrong");
            const Shader::ReflectionInfo& info = hlslShaders[0]->getReflectionInfo();
            checkVertexShaderReflection(info);
        }

        {
            ASSERT(hlslShaders[1] != nullptr, "wrong");
            const Shader::ReflectionInfo& info = hlslShaders[1]->getReflectionInfo();
            checkFragmentShaderReflection(info);
        }

        ResourceLoaderManager::getInstance()->remove(hlslShaders[0]);
        ResourceLoaderManager::getInstance()->remove(hlslShaders[1]);
        hlslShaders.clear();
    }

    [[maybe_unused]] u32 endTest = 0;
}

void MyApplication::Test_CreateShaderProgram()
{
    {
        Shader* vertShader = nullptr;
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

            ShaderHeader vertexHeader(ShaderType::ShaderType_Vertex);
            vertexHeader._contentType = ShaderHeader::ShaderResource::ShaderResource_Source;
            vertexHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0;

            vertShader = resource::ResourceLoaderManager::getInstance()->composeShader<Shader, resource::ShaderSourceStreamLoader>(m_Context, "vertex", &vertexHeader, vertexStream);
            ASSERT(vertShader, "nullptr");
        }

        Shader* fragShader = nullptr;
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

            ShaderHeader fragmentHeader(ShaderType::ShaderType_Fragment);
            fragmentHeader._contentType = ShaderHeader::ShaderResource::ShaderResource_Source;
            fragmentHeader._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0;

            fragShader = resource::ResourceLoaderManager::getInstance()->composeShader<Shader, resource::ShaderSourceStreamLoader>(m_Context, "fragment", &fragmentHeader, fragmentStream);
            ASSERT(fragShader, "nullptr");
        }


        CommandList* commandList = new renderer::CommandList(m_Context, CommandList::CommandListType::ImmediateCommandList);
        ASSERT(commandList, "nullptr");

        ShaderProgram* singleProgram = commandList->createObject<renderer::ShaderProgram>(std::vector<const Shader*>({ vertShader }));
        ASSERT(singleProgram, "wrong");
        renderer::ShaderProgram* twoShaderProgram = commandList->createObject<ShaderProgram, std::vector<const Shader*>>({ {vertShader, fragShader} });
        ASSERT(twoShaderProgram, "wrong");

        delete singleProgram;
        delete twoShaderProgram;

        delete commandList;

        ResourceLoaderManager::getInstance()->remove(vertShader);
        ResourceLoaderManager::getInstance()->remove(fragShader);
    }
}

void MyApplication::Test_ShaderParam()
{
    {
        Shader* hlslVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.vs");
        ASSERT(hlslVShader != nullptr, "wrong");

        Shader* hlslPShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/test/shaders/testReflectInfoWithNames.ps");
        ASSERT(hlslPShader != nullptr, "wrong");

        CommandList* commandList = new CommandList(m_Context, CommandList::CommandListType::ImmediateCommandList);
        ASSERT(commandList, "nullptr");

        ShaderProgram* shaderProgram = commandList->createObject<ShaderProgram, std::vector<const Shader*>>({ {hlslVShader, hlslPShader} });
        ASSERT(shaderProgram, "shaderProgram");

        {
            struct UBO11_size256
            {
                core::Matrix4D matrix0_11[2];
                core::Matrix4D matrix1_11[2];
            } ubo11_size256;
            ASSERT(sizeof(ubo11_size256) == 256, "wrong");

            ShaderParam param256("ubo11_size256");
            ASSERT(param256._id == crc32c::Crc32c(std::string("ubo11_size256")), "wrong");
            shaderProgram->bindUniformsBuffer<ShaderType::ShaderType_Vertex>(param256, 0, sizeof(UBO11_size256), & ubo11_size256);

            struct UBO01_size192
            {
                core::Matrix4D matrix0_01;
                core::Matrix4D matrix1_01;
                core::Matrix4D matrix2_01;
            } ubo01_size192;
            ASSERT(sizeof(ubo01_size192) == 192, "wrong");

            ShaderParam param192("ubo01_size192");
            ASSERT(param192._id == crc32c::Crc32c(std::string("ubo01_size192")), "wrong");
            shaderProgram->bindUniformsBuffer<ShaderType::ShaderType_Vertex>(param192, 0, sizeof(UBO01_size192), &ubo01_size192);
        }

        {
            Texture2D* texture = commandList->createObject<Texture2D>(TextureUsage::TextureUsage_Sampled, Format::Format_R32G32B32A32_SFloat, core::Dimension2D(1,1));
            ASSERT(texture, "nullptr");

            SamplerState* sampler = commandList->createObject<SamplerState>();

            ShaderParam textureColor01("textureColor01");
            ASSERT(textureColor01._id == crc32c::Crc32c(std::string("textureColor01")), "wrong");
            ShaderParam textureColor10("textureColor10");
            ASSERT(textureColor10._id == crc32c::Crc32c(std::string("textureColor10")), "wrong");
            ShaderParam samplerColor00("samplerColor00");
            ASSERT(samplerColor00._id == crc32c::Crc32c(std::string("samplerColor00")), "wrong");

            shaderProgram->bindTexture<ShaderType::ShaderType_Fragment>(textureColor01, texture);
            shaderProgram->bindTexture<ShaderType::ShaderType_Fragment>(textureColor10, texture);
            shaderProgram->bindSampler<ShaderType::ShaderType_Fragment>(samplerColor00, sampler);

            delete sampler;
            delete texture;
        }
        delete shaderProgram;

        delete commandList;

        ResourceLoaderManager::getInstance()->remove(hlslVShader);
        ResourceLoaderManager::getInstance()->remove(hlslPShader);
    }
}

void MyApplication::Test_CreatePipeline()
{
    //TODO
}

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
