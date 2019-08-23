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
    Test_MemoryPool();

    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    ASSERT(m_Context, "context is nullptr");

    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::DelayedCommandList);

    Shader* shader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/5.test/data/composition.frag");

    Shader* glslVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/5.test/data/simple.vert");
    Shader* glslFSShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/5.test/data/simple.frag");
    Shader* hlslVShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/5.test/data/simple.vs");
    Shader* hlslPShader = ResourceLoaderManager::getInstance()->loadShader<Shader, ShaderSourceFileLoader>(m_Context, "examples/5.test/data/simple.ps");


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

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
