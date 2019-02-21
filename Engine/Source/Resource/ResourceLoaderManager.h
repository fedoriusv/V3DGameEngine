#pragma once

#include "Common.h"
#include "Utils/Singleton.h"

namespace v3d
{
namespace renderer
{
    class Context;
} //namespace renderer

namespace resource
{
    class Resource;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * ResourceLoaderManager
    */
    class ResourceLoaderManager : public utils::Singleton<ResourceLoaderManager>
    {
    public:

        ResourceLoaderManager() = default;

        template<class TResource, class TResourceLoader>
        TResource* loadShader(renderer::Context* context, std::string filename, std::vector<std::pair<std::string, std::string>> defines = {})
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
        TResource* load(renderer::Context* context, std::string filename)
        {
            std::string innerName(filename);
            std::transform(filename.begin(), filename.end(), innerName.begin(), ::tolower);

            auto resourceIter = m_resources.emplace(std::make_pair(innerName, nullptr));
            if (resourceIter.second)
            {
                TResourceLoader loader(context);
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

    private:

        std::map<std::string, Resource*> m_resources;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
