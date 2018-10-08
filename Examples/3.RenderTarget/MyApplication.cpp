#include "MyApplication.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include "Renderer/Context.h"
#include "Event/InputEventReceiver.h"
#include "Platform/Window.h"

#include "Object/Texture.h"
#include "Object/RenderTarget.h"
#include "Renderer/ImageFormats.h"


using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

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

        MyApplication::Running(*m_CommandList);
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



    //RenderTarget* target = m_CommandList->createObject<RenderTarget>(core::Dimension2D(0, 0));
    //Texture2D* colorTexture = target->attachColorTexture(renderer::ImageFormat::ImageFormat_R8G8B8A8_UInt, RenderTargetSamples::SampleCount_x1, RenderTargetLoadOp::LoadOp_Clear, RenderTargetStoreOp::StoreOp_Store);
    //m_CommandList.setRenderTarget(target);

    ////draw

    //m_CommandList.submitCommands(bool wait, callbackCompete);
    //colorTexture->read();


    ////////////////////////////////////

    Texture2D* texture = m_CommandList->createObject<Texture2D>(renderer::ImageFormat::ImageFormat_R8G8B8A8_UInt, core::Dimension2D(80, 80), renderer::TextureSamples::SampleCount_x1);
    RenderTarget* target0 = m_CommandList->createObject<RenderTarget>(core::Dimension2D(80, 80));
    bool success = target0->setColorTexture(texture, renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store);
    m_CommandList->setRenderTarget(target0);

    //draw

    
    //RenderTarget* target1 = m_CommandList->createObject<RenderTarget>(core::Dimension2D(0, 0));
    //bool success = target1->setColorTexture(texture, RenderTargetLoadOp::LoadOp_Load, RenderTargetStoreOp::StoreOp_Store);
    //m_CommandList.setRenderTarget(target1);

    //draw



    //Texture2D* texture = m_CommandList->createObject<Texture2D>(renderer::ImageFormat::ImageFormat_Undefined, core::Dimension2D(0, 0));
    //texture->update({ 0,0 }, {0, 0}, 1, nullptr);

    /*commandList.setPipeline(pipe1);
    commandList.setTexture(texture);
    commandList.setUnifrom(uniform1);
    commandList.draw(geometry1);*/

    /*Geometry geometry;
    Image image;
    Pipeline pipe;
    {
        texture.upload(image);
        geometry.upload(data);
    }*/

    m_clearColor = { 1.0, 0.0, 0.0, 1.0 };
}

bool MyApplication::Running(renderer::CommandList& commandList)
{
    //Frame
    commandList.beginFrame();
    commandList.clearBackbuffer(m_clearColor);

    /*commandList.set(pipe1);
    commandList.set(texture1);
    commandList.set(uniform1);
    commandList.draw(geometry1);

    commandList.set(pipe2);
    commandList.set(texture2);
    commandList.set(uniform2);
    commandList.draw(geometry2);*/

    commandList.endFrame();
    commandList.presentFrame();
    
    commandList.flushCommands();

    return true;
}

void MyApplication::Exit()
{
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
