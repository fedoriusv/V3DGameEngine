#pragma once

#include "Common.h"
#include "Utils/Singleton.h"
#include "Resource.h"

namespace v3d
{
namespace renderer
{
    class Context;
} //namespace renderer

namespace resource
{
    struct ResourceHeader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * ResourceLoaderManager
    */
    class ResourceLoaderManager : public utils::Singleton<ResourceLoaderManager>
    {
    public:

        ResourceLoaderManager() = default;

        template<class TResource, class TResourceLoader>
        TResource* loadShader(renderer::Context* context, std::string filename, std::vector<std::pair<std::string, std::string>> defines = {});

        template<class TResource, class TResourceLoader>
        TResource* load(std::string filename, u32 flags = 0);

        template<class TResource, class TResourceLoader>
        TResource* load(std::string filename, const ResourceHeader* header, u32 flags = 0);

        void clear();
        bool remove(Resource* resource);

        void addPath(const std::string& path);
        void removePath(const std::string& path);
        const std::vector<std::string>& getPathes() const;

    private:

        std::map<std::string, Resource*> m_resources;
        std::vector<std::string>         m_pathes;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TResource, class TResourceLoader>
    TResource* ResourceLoaderManager::loadShader(renderer::Context* context, std::string filename, std::vector<std::pair<std::string, std::string>> defines)
    {
        std::string innerName(filename);
        std::transform(filename.begin(), filename.end(), innerName.begin(), ::tolower);

        std::sort(defines.begin(), defines.end(), [](const std::pair<std::string, std::string>& macros1, const std::pair<std::string, std::string>& macros2) -> bool
        {
            return macros1.first < macros2.first;
        });

        auto composeResourceName = [](const std::string& name, const std::vector<std::pair<std::string, std::string>>& defines) -> std::string
        {
            std::string outString = name;
            for (auto& define : defines)
            {
                outString.append("_");
                outString.append(define.first);
                outString.append(define.second);
            }

            return outString;
        };
        const std::string resourceName = composeResourceName(innerName, defines);

        auto resourceIter = m_resources.emplace(std::make_pair(resourceName, nullptr));
        if (resourceIter.second)
        {
            TResourceLoader loader(context, defines);
            Resource* res = loader.load(innerName);
            if (!res)
            {
                m_resources.erase(resourceName);
                return nullptr;
            }

            resourceIter.first->second = res;
            return static_cast<TResource*>(res);
        }

        return static_cast<TResource*>(resourceIter.first->second);
    }

    template<class TResource, class TResourceLoader>
    TResource* ResourceLoaderManager::load(std::string filename, u32 flags)
    {
        std::string innerName(filename);
        std::transform(filename.begin(), filename.end(), innerName.begin(), ::tolower);

        auto resourceIter = m_resources.emplace(std::make_pair(innerName, nullptr));
        if (resourceIter.second)
        {
            TResourceLoader loader(flags);
            Resource* res = loader.load(innerName);
            if (!res)
            {
                m_resources.erase(innerName);
                return nullptr;
            }

            resourceIter.first->second = res;
            return static_cast<TResource*>(res);
        }

        return static_cast<TResource*>(resourceIter.first->second);
    }

    template<class TResource, class TResourceLoader>
    TResource* ResourceLoaderManager::load(std::string filename, const ResourceHeader* header, u32 flags)
    {
        std::string innerName(filename);
        std::transform(filename.begin(), filename.end(), innerName.begin(), ::tolower);

        auto resourceIter = m_resources.emplace(std::make_pair(innerName, nullptr));
        if (resourceIter.second)
        {
            TResourceLoader loader(header, flags);
            Resource* res = loader.load(innerName);
            if (!res)
            {
                m_resources.erase(innerName);
                return nullptr;
            }

            resourceIter.first->second = res;
            return static_cast<TResource*>(res);
        }

        return static_cast<TResource*>(resourceIter.first->second);
    }

    inline void ResourceLoaderManager::clear()
    {
        for (auto& iter : m_resources)
        {
            Resource* res = iter.second;
            delete res;
        }
        m_resources.clear();
    }

    inline bool ResourceLoaderManager::remove(Resource* resource)
    {
        for (auto& iter : m_resources)
        {
            Resource* res = iter.second;
            if (resource == res)
            {
                delete res;
                m_resources.erase(iter.first);

                return true;
            }
        }

        return false;
    }

    inline void ResourceLoaderManager::addPath(const std::string & path)
    {
        auto it = std::find(m_pathes.begin(), m_pathes.end(), path);
        if (it == m_pathes.end())
        {
            m_pathes.push_back(path);
        }
    }

    inline void ResourceLoaderManager::removePath(const std::string & path)
    {
        auto it = std::find(m_pathes.begin(), m_pathes.end(), path);
        if (it != m_pathes.end())
        {
            m_pathes.erase(std::remove(m_pathes.begin(), m_pathes.end(), *it), m_pathes.end());
        }
    }

    inline const std::vector<std::string>& ResourceLoaderManager::getPathes() const
    {
        return m_pathes;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
