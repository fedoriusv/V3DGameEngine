#pragma once

#include "Common.h"

#include "Thread/Thread.h"
#include "Thread/ThreadSafeContainers.h"
#include "Events/Game/GameEventHandler.h"

using namespace v3d;

class EditorHotReload final
{
public:

    EditorHotReload(event::GameEventReceiver* gameEventRecevier) noexcept;
    ~EditorHotReload();

    u32 addFolder(const std::string& dirname);

    static void hotReloadLoop(EditorHotReload* hotReload);

private:

    event::GameEventReceiver* const m_gameEventRecevier;
    thread::Thread                  m_thread;
    std::mutex                      m_mutex;

    bool addFile(const std::string& filename);
    void trackFiles();

    struct WatchedFile
    {
        std::filesystem::path           _path;
        std::filesystem::file_time_type _modifiedTime;
    };
    std::vector<std::tuple<std::string, std::vector<WatchedFile>>> m_folders;

};