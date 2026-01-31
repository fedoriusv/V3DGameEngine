#pragma once

#include "Common.h"
#include "Resource.h"
#include "Utils/Singleton.h"
#include "Utils/Timer.h"
#include "Renderer/Shader.h"
#include "Resource/Decoder/ShaderDecoder.h"

namespace v3d
{
namespace renderer
{
    class Device;
} //namespace renderer
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * @brief ResourceManager, Singleton
    */
    class ResourceManager : public utils::Singleton<ResourceManager>
    {
    public:

        /**
        * @brief composeShader interface.
        * Create a shader from steam data. Supports: ShaderSourceStreamLoader
        *
        * @param renderer::Device* device [required]
        * @param const std::string name [required]
        * @param const ShaderDecoder::ShaderPolicy* policy [required]
        * @param const stream::Stream* stream [required]
        * @param u32 flags [optional]
        * @return Shader resource, nullptr if failed
        */
        template<class TResource = renderer::Shader, class TResourceLoader>
        [[nodiscard]] const TResource* composeShader(renderer::Device* device, const std::string& name, const ShaderDecoder::ShaderPolicy& policy, const stream::Stream* stream, ShaderCompileFlags flags);

        /**
        * @brief loadShader
        * Create shader from the file. Suppotrs ShaderSourceFileLoader
        *
        * @param renderer::Device* device [required]
        * @param const std::string& filename [required]
        * @param const std::string& entrypoint [optional]
        * @param const renderer::Shader::DefineList& defines [optional]
        * @param const std::vector<std::string>& includes [optional]
        * @param u32 flags [optional]
        * @return Shader resource, nullptr if failed
        */
        template<class TResource = renderer::Shader, class TResourceLoader>
        [[nodiscard]] const TResource* loadShader(renderer::Device* device, const std::string& filename,
            const std::string& entrypoint = "main", const renderer::Shader::DefineList& defines = {}, const std::vector<std::string>& includes = {}, ShaderCompileFlags flags = 0);

        /**
        * @brief load interface.
        * Create resource from file. Supports: AssetFileLoader, ImageFileLoader, ModelFileLoader, ShaderBinaryFileLoader
        * 
        * @param std::string filename [required]
        * @param u32 flags [optional]
        * @return current resource, nullptr if failed
        */
        template<class TResource, class TResourceLoader>
        [[nodiscard]] TResource* load(const std::string& filename, u32 flags = 0);

        /**
        * @brief load interface.
        * Create resource from file and upload data to GPU. Supports: AssetFileLoader, ModelFileLoader, TextureFileLoader
        *
        * @param renderer::Device* device [required]
        * @param sconst std::string& filename [required]
        * @param const TResourceLoader::PolicyType& policy [required]
        * @param u32 flags [optional]
        * @return current resource, nullptr if failed
        */
        template<class TResource, class TResourceLoader, typename TPolicy = TResourceLoader::PolicyType>
        [[nodiscard]] TResource* load(renderer::Device* device, const std::string& filename, const TPolicy& policy, u32 flags = 0);

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
        template<class TResource>
        bool remove(TResource* resource);

        template<class TResource>
        bool remove(const TResource* resource);

        void addPath(const std::string& path);
        void removePath(const std::string& path);
        const std::vector<std::string>& getPaths() const;

    private:

        friend utils::Singleton<ResourceManager>;

        /**
        * @brief ResourceManager constructor
        */
        ResourceManager() noexcept = default;

        std::map<std::string, Resource*> m_resources;
        std::vector<std::string>         m_paths;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TResource, class TResourceLoader>
    inline const TResource* ResourceManager::composeShader(renderer::Device* device, const std::string& name, const ShaderDecoder::ShaderPolicy& policy, const stream::Stream* stream, ShaderCompileFlags flags)
    {
        static_assert(std::is_base_of<renderer::Shader, TResource>(), "wrong type");
        std::string innerName(name);
        std::transform(name.cbegin(), name.cend(), innerName.begin(), ::tolower);

        renderer::Shader::DefineList innerDefines(policy._defines);
        std::sort(innerDefines.begin(), innerDefines.end(), [](const std::pair<std::string, std::string>& macros1, const std::pair<std::string, std::string>& macros2) -> bool
            {
                return macros1.first < macros2.first;
            });

        auto composeResourceName = [](const std::string& name, const std::string& entrypoint, const std::vector<std::pair<std::string, std::string>>& defines) -> std::string
        {
            std::string outString = name;
            outString.append("#");
            outString.append(entrypoint);

            for (auto& define : defines)
            {
                outString.append("#");
                outString.append(define.first);
                outString.append(define.second);
            }

            return outString;
        };
        const std::string resourceName = composeResourceName(innerName, policy._entryPoint, innerDefines);

        auto resourceIter = m_resources.emplace(std::make_pair(resourceName, nullptr));
        if (resourceIter.second)
        {
            TResourceLoader loader(device, policy, stream, flags);
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
    inline const TResource* ResourceManager::loadShader(renderer::Device* device, const std::string& filename,
        const std::string& entrypoint, const renderer::Shader::DefineList& defines, const std::vector<std::string>& includes, ShaderCompileFlags flags)
    {
        static_assert(std::is_base_of<renderer::Shader, TResource>(), "wrong type");
        std::string innerName(filename);
        std::transform(filename.begin(), filename.end(), innerName.begin(), ::tolower);

        renderer::Shader::DefineList innerDefines(defines);
        std::sort(innerDefines.begin(), innerDefines.end(), [](const std::pair<std::string, std::string>& macros1, const std::pair<std::string, std::string>& macros2) -> bool
        {
            return macros1.first < macros2.first;
        });

        auto composeResourceName = [](const std::string& name, const std::string& entrypoint, const std::vector<std::pair<std::string, std::string>>& defines) -> std::string
            {
                std::string outString = name;
                outString.append("#");
                outString.append(entrypoint);

                for (auto& define : defines)
                {
                    outString.append("#");
                    outString.append(define.first);
                    outString.append(define.second);
                }

                return outString;
            };
        const std::string resourceName = composeResourceName(innerName, entrypoint, innerDefines);

        auto resourceIter = m_resources.emplace(std::make_pair(resourceName, nullptr));
        if (resourceIter.second)
        {
            TResourceLoader loader(device, renderer::getShaderTypeByClass<TResource>(), entrypoint, defines, includes, flags);
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
    inline TResource* ResourceManager::load(const std::string& filename, u32 flags)
    {
        std::string innerName(filename);
        std::transform(filename.cbegin(), filename.cend(), innerName.begin(), ::tolower);

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

    template<class TResource, class TResourceLoader, typename TPolicy>
    inline TResource* ResourceManager::load(renderer::Device* device, const std::string& filename, const TPolicy& policy, u32 flags)
    {
        std::string innerName(filename);
        std::transform(filename.cbegin(), filename.cend(), innerName.begin(), ::tolower);

        auto found = m_resources.find(innerName);
        if (found != m_resources.end() && policy.unique)
        {
            return static_cast<TResource*>(found->second);
        }
        else
        {
            if (found != m_resources.end())
            {
                innerName = std::format("{}_copy_{}", innerName, utils::Timer::getCurrentTime());
            }

            TResourceLoader loader(device, policy, flags);
            Resource* resource = loader.load(filename);
            if (resource)
            {
                m_resources.insert(std::make_pair(innerName, resource));
                return static_cast<TResource*>(resource);
            }
        }

        return nullptr;
    }

    inline void ResourceManager::clear()
    {
        for (auto& iter : m_resources)
        {
            Resource* res = iter.second;
            V3D_DELETE(res, memory::MemoryLabel::MemoryObject);
        }
        m_resources.clear();
    }

    template<class TResource>
    inline bool ResourceManager::remove(TResource* resource)
    {
        for (auto& iter : m_resources)
        {
            TResource* res = static_cast<TResource*>(iter.second);
            if (resource == res)
            {
                ResourceReport report;
                report._event = ResourceReport::Event::Destroy;

                res->notify(report);

                V3D_DELETE(res, memory::MemoryLabel::MemoryObject);
                m_resources.erase(iter.first);

                return true;
            }
        }

        return false;
    }

    template<class TResource>
    inline bool ResourceManager::remove(const TResource* resource)
    {
        for (auto& iter : m_resources)
        {
            const TResource* res = static_cast<const TResource*>(iter.second);
            if (resource == res)
            {
                ResourceReport report;
                report._event = ResourceReport::Event::Destroy;

                res->notify(report);

                V3D_DELETE(res, memory::MemoryLabel::MemoryObject);
                m_resources.erase(iter.first);

                return true;
            }
        }

        return false;
    }

    inline void ResourceManager::addPath(const std::string& path)
    {
        auto it = std::find(m_paths.begin(), m_paths.end(), path);
        if (it == m_paths.end())
        {
            m_paths.push_back(path);
        }
    }

    inline void ResourceManager::removePath(const std::string& path)
    {
        auto it = std::find(m_paths.begin(), m_paths.end(), path);
        if (it != m_paths.end())
        {
            m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), *it), m_paths.end());
        }
    }

    inline const std::vector<std::string>& ResourceManager::getPaths() const
    {
        return m_paths;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
