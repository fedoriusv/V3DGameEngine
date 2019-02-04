#include "MyApplication.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/RenderTarget.h"
#include "Renderer/Object/StreamBuffer.h"
#include "Renderer/Formats.h"

#include "Resource/ResourceLoaderManager.h"

#include "Resource/ShaderSourceFileLoader.h"
#include "Resource/Shader.h"




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

            m_ClearColor = {r, g, b, 1.0f };

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
    //Test_MemoryPool();

    m_Context = renderer::Context::createContext(m_Window, renderer::Context::RenderType::VulkanRender);
    ASSERT(m_Context, "context is nullptr");
    m_CommandList = new renderer::CommandList(m_Context, renderer::CommandList::CommandListType::DelayedCommandList);

    std::vector<f32> vertexBuffer =
    {
        1.0f,  1.0f, 0.0f ,     1.0f, 0.0f, 0.0f,
       -1.0f,  1.0f, 0.0f ,     0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f ,     0.0f, 0.0f, 1.0f
    };

    VertexInputAttribDescription::InputBinding binding(0, VertexInputAttribDescription::InputRate_Vertex, u32(sizeof(f32) * vertexBuffer.size()));
    renderer::VertexInputAttribDescription vertexDesc({ binding },
        {
            { binding._index, 0, Format::Format_R32G32B32_SFloat, 0 },                  //pos
            { binding._index, 0, Format::Format_R32G32B32_SFloat, sizeof(f32) * 3 }     //color
        }
    );

    m_Render = new renderer::SimpleRender(*m_CommandList, vertexDesc, vertexBuffer);
    m_ClearColor = { 1.0, 0.0, 0.0, 1.0 };
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.beginFrame();

    m_Render->render(commandList);

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
    delete m_Render;
    delete m_CommandList;
    Context::destroyContext(m_Context);

    m_Window->getInputEventReceiver()->dettach(InputEvent::InputEventType::MouseInputEvent);
}

void MyApplication::Test_MemoryPool()
{
    MemoryPool pool;
    ////TEST
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

MyApplication::~MyApplication()
{
    delete m_InputEventHandler;
    m_InputEventHandler = nullptr;

    Window::detroyWindow(m_Window);
}
