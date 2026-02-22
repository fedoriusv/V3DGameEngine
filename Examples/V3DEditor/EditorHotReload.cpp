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
        std::vector<WatchedFile> list;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirname))
        {
            if (stream::FileStream::isExists(entry.path().string()))
            {
                WatchedFile file;
                file._path = std::filesystem::absolute(entry.path().string());
                file._modifiedTime = std::filesystem::last_write_time(file._path);

                list.push_back(file);
                ++result;
            }
        }

        if (!list.empty())
        {
            m_folders.emplace_back(dirname, std::move(list));
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

    if (m_folders.empty())
    {
        return;
    }

    for (auto& [folderName, files] : m_folders)
    {
        for (auto& file : files)
        {
            std::error_code error;
            auto time = std::filesystem::last_write_time(file._path, error);
            if (error)
            {
                //Skip file system errors
                continue;
            }

            if (time > file._modifiedTime + std::chrono::milliseconds(500))
            {
                file._modifiedTime = time;
                m_gameEventRecevier->pushEvent(new event::ShaderHotReload(folderName, file._path.string()));
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
