#include "MyApplication.h"
#include "Utils/Logger.h"

using namespace v3d;
using namespace v3d::platform;
using namespace v3d::utils;

MyApplication::MyApplication(int& argc, char** argv)
    : m_Window(nullptr)
{
    m_Window = Window::createWindow({ 1024, 768 }, {800, 500});
    ASSERT(m_Window, "windows is nullptr");
}

MyApplication::~MyApplication()
{
    if (m_Window)
    {
        Window::detroyWindow(m_Window);
        m_Window = nullptr;
    }
}

int MyApplication::Execute()
{
    MyApplication::Initialize();
    bool running = true;
    while (running)
    {
        running = Window::updateWindow(m_Window);

        MyApplication::Running();
    }

    delete this;

    return 0;
}

void MyApplication::Initialize()
{
    //TEST
    LOG_DEBUG("Debug Test");
    LOG("Test");
    LOG_INFO("Info Test");
    LOG_WARNING("Warrnig Test");
    LOG_ERROR("Error Test");
    LOG_FATAL("Fatal Test");

    char cvar[] = "CHAR";
    int ivar = 0;

    LOG_DEBUG("Debug Test, %s, %d", cvar, ivar++);
    LOG("Test, %s, %d", cvar, ivar++);
    LOG_INFO("Info Test, %s, %d", cvar, ivar++);
    LOG_WARNING("Warrnig Test, %s, %d", cvar, ivar++);
    LOG_ERROR("Error Test, %s, %d", cvar, ivar++);
    LOG_FATAL("Fatal Test, %s, %d", cvar, ivar++);

    int b = 0;
}

bool MyApplication::Running()
{
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //m_Window->minimize();
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //m_Window->restore();

    return true;
}
