#include "EditorHotReload.h"
#include "Stream/FileStream.h"

#include "EditorScene.h"

EditorHotReload::EditorHotReload(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_gameEventRecevier(gameEventRecevier)
{
    m_thread.run(&EditorHotReload::hotReloadLoop, this);
    m_thread.setPriority(thread::Thread::Priority::Lowest);
    m_thread.setName("HotReloadThread");
}

EditorHotReload::~EditorHotReload()
{
    m_thread.terminate();
}

bool EditorHotReload::addFile(const std::string& filename)
{
    if (stream::FileStream::isExists(filename))
    {
        WatchedFile file;
        file._path = std::filesystem::absolute(filename);
        file._modifiedTime = std::filesystem::last_write_time(file._path);

        m_files.push_back(file);
        return true;
    }

    return false;
}

u32 EditorHotReload::addFolder(const std::string& dirname)
{
    std::scoped_lock lock(m_mutex);

    u32 result = 0;
    if (stream::FileStream::isDirectory(dirname))
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirname))
        {
            if (addFile(entry.path().string()))
            {
                ++result;
            }
        }
    }

    return result;
}

void EditorHotReload::hotReloadLoop(EditorHotReload* hotReload)
{
    while (hotReload->m_thread.isRunning())
    {
        hotReload->trackFiles();
    }
}

void EditorHotReload::trackFiles()
{
    std::scoped_lock lock(m_mutex);

    for (auto& file : m_files)
    {
        std::error_code error;
        auto time = std::filesystem::last_write_time(file._path, error);
        if (error)
        {
            continue;
        }

        if (time > file._modifiedTime + std::chrono::milliseconds(500))
        {
            file._modifiedTime = time;

            m_gameEventRecevier->pushEvent(new event::ShaderHotReload(file._path.string()));
        }
    }
}
