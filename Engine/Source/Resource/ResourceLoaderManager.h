#pragma once

#include "Common.h"
#include "Utils/Singleton.h"
#include "Resource.h"
#include "Renderer/Shader.h"

namespace v3d
{
namespace renderer
{
    class Context;
} //namespace renderer

namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ResourceHeader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * @brief ResourceLoaderManager, Singleton
    */
    class ResourceLoaderManager : public utils::Singleton<ResourceLoaderManager>
    {
    public:

        ResourceLoaderManager() = default;

        /**
        * @brief composeShader
        * Create shader from steam data
        *
        * @param renderer::Context* context [required]
        * @param const std::string name [required]
        * @param const renderer::ShaderHeader* header name [required]
        * @param const stream::Stream* stream [required]
        * @return Shader resource, nullptr if failed
        */
        template<class TResource = renderer::Shader, class TResourceLoader>
        const TResource* composeShader(renderer::Context* context, const std::string name, const renderer::ShaderHeader* header, const stream::Stream* stream);

        /**
        * @brief loadShader
        * Create shader from file
        *
        * @param renderer::Context* context [required]
        * @param const std::string filename [required]
        * @param std::vector<std::pair<std::string, std::string>> defines [optional]
        * @param u32 flags [optional]
        * @return Shader resource, nullptr if failed
        */
        template<class TResource = renderer::Shader, class TResourceLoader>
        const TResource* loadShader(renderer::Context* context, std::string filename, std::vector<std::pair<std::string, std::string>> defines = {}, u32 flags = 0);

        /**
        * @brief loadHLSLShader
        * Create list of HLSL shaders from file
        *
        * @param renderer::Context* context [required]
        * @param const std::string filename [required]
        * @param const std::vector<std::pair<std::string, std::string>>& entryPoint/Type [required]
        * @param std::vector<std::pair<std::string, std::string>> defines [optional]
        * @param u32 flags [optional]
        * @return list of shader resources
        */
        template<class TResource = renderer::Shader, class TResourceLoader>
        std::vector<const TResource*> loadHLSLShader(renderer::Context* context, std::string filename, const std::vector<std::tuple<std::string, renderer::ShaderType>>& entryPoints, std::vector<std::pair<std::string, std::string>> defines = {}, u32 flags = 0);

        /**
        * @brief load
        * Create resource from file
        *
        * @param std::string filename [required]
        * @param u32 flags [optional]
        * @return current resource, nullptr if failed
        */
        template<class TResource, class TResourceLoader>
        TResource* load(std::string filename, u32 flags = 0);

        /**
        * @brief load
        * Create resource from file by header
        *
        * @param std::string filename [required]
        * @param const ResourceHeader* header [required]
        * @param u32 flags [optional]
        * @return current resource, nullptr if failed
        */
        template<class TResource, class TResourceLoader>
        TResource* load(std::string filename, const ResourceHeader* header, u32 flags = 0);

        /**
        * @brief clear
        * Remove all created resources
        */
        void clear();

        /**
        * @brief clear
        * Remove single Resource
        * @param Resource* resource [required]
        * @retrun true if resurce has bound and deleted
        */
        bool remove(Resource* resource);
        bool remove(const Resource* resource);

        void addPath(const std::string& path);
        void removePath(const std::string& path);
        const std::vector<std::string>& getPathes() const;

    private:

        std::map<std::string, Resource*> m_resources;
        std::vector<std::string>         m_pathes;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TResource, class TResourceLoader>
    inline const TResource* ResourceLoaderManager::composeShader(renderer::Context* context, const std::string name, const renderer::ShaderHeader* header, const stream::Stream* stream)
    {
        static_assert(std::is_same<TResource, renderer::Shader>(), "wrong type");
        std::string innerName(name);
        std::transform(name.begin(), name.end(), innerName.begin(), ::tolower);

        std::vector<std::pair<std::string, std::string>> innerDefines(header->_defines);
        std::sort(innerDefines.begin(), innerDefines.end(), [](const std::pair<std::string, std::string>& macros1, const std::pair<std::string, std::string>& macros2) -> bool
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
        const std::string resourceName = composeResourceName(innerName, innerDefines);

        auto resourceIter = m_resources.emplace(std::make_pair(resourceName, nullptr));
        if (resourceIter.second)
        {
            TResourceLoader loader(context, header, stream, true);
            Resource* res = loader.load(innerName);
            if (!res)
            {
                m_resources.erase(resourceIter.first);
                return nullptr;
            }

            resourceIter.first->second = res;
            return static_cast<TResource*>(res);
        }

        return static_cast<TResource*>(resourceIter.first->second);
    }

    template<class TResource, class TResourceLoader>
    inline const TResource* ResourceLoaderManager::loadShader(renderer::Context* context, std::string filename, std::vector<std::pair<std::string, std::string>> defines, u32 flags)
    {
        static_assert(std::is_same<TResource, renderer::Shader>(), "wrong type");
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
            TResourceLoader loader(context, defines, flags);
            Resource* res = loader.load(innerName);
            if (!res)
            {
                m_resources.erase(resourceIter.first);
                return nullptr;
            }

            resourceIter.first->second = res;
            return static_cast<TResource*>(res);
        }

        return static_cast<TResource*>(resourceIter.first->second);
    }

    template<class TResource, class TResourceLoader>
    inline std::vector<const TResource*> ResourceLoaderManager::loadHLSLShader(renderer::Context* context, std::string filename, const std::vector<std::tuple<std::string, renderer::ShaderType>>& entryPoints, std::vector<std::pair<std::string, std::string>> defines, u32 flags)
    {
        static_assert(std::is_same<TResource, renderer::Shader>(), "wrong type");
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

        std::vector<const TResource*> resources;
        for (auto& entryPoint : entryPoints)
        {
            const std::string resourceName = composeResourceName(innerName, defines) + "_" + renderer::ShaderTypeString(std::get<1>(entryPoint));
            auto resourceIter = m_resources.emplace(std::make_pair(resourceName, nullptr));
            if (resourceIter.second)
            {
                TResourceLoader loader(context, std::get<1>(entryPoint), std::get<0>(entryPoint), defines, flags);
                Resource* res = loader.load(innerName);
                if (!res)
                {
                    m_resources.erase(resourceIter.first);
                    resources.push_back(nullptr);

                    continue;
                }

                resourceIter.first->second = res;
                resources.push_back(static_cast<TResource*>(res));
            }
        }

        return resources;
    }

    template<class TResource, class TResourceLoader>
    inline TResource* ResourceLoaderManager::load(std::string filename, u32 flags)
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
                m_resources.erase(resourceIter.first);
                return nullptr;
            }

            resourceIter.first->second = res;
            return static_cast<TResource*>(res);
        }

        return static_cast<TResource*>(resourceIter.first->second);
    }

    template<class TResource, class TResourceLoader>
    inline TResource* ResourceLoaderManager::load(std::string filename, const ResourceHeader* header, u32 flags)
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
                m_resources.erase(resourceIter.first);
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
                res->notifyObservers();
                delete res;

                m_resources.erase(iter.first);

                return true;
            }
        }

        return false;
    }

    inline bool ResourceLoaderManager::remove(const Resource* resource)
    {
        for (auto& iter : m_resources)
        {
            const Resource* res = iter.second;
            if (resource == res)
            {
                res->notifyObservers();
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
