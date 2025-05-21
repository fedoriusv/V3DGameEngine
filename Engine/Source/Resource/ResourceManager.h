#pragma once

#include "Common.h"
#include "Resource.h"
#include "Utils/Singleton.h"
#include "Renderer/Shader.h"
#include "ShaderDecoder.h"

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
        * @brief loadShader
        * Create shader from the file. Suppotrs ShaderSourceFileLoader
        *
        * @param renderer::Device* device [required]
        * @param const std::string& filename [required]
        * @param renderer::ShaderType type [required]
        * @param const std::string& entrypoint [optional]
        * @param const renderer::Shader::DefineList& defines [optional]
        * @param const std::vector<std::string>& includes [optional]
        * @param u32 flags [optional]
        * @return Shader resource, nullptr if failed
        */
        template<class TResource = renderer::Shader, class TResourceLoader>
        [[nodiscard]] const TResource* loadShader(renderer::Device* device, const std::string& filename, renderer::ShaderType type,
            const std::string& entrypoint = "main", const renderer::Shader::DefineList& defines = {}, const std::vector<std::string>& includes = {}, ShaderCompileFlags flags = 0);

        /**
        * @brief loadHLSLShaders
        * Create list of HLSL shaders from file. Supports ShaderSourceFileLoader
        *
        * @param renderer::Device* device [required]
        * @param const std::string filename [required]
        * @param const std::vector<std::pair<std::string, std::string>>& entryPoint/Type [required]
        * @param std::vector<std::pair<std::string, std::string>> defines [optional]
        * @param ShaderSourceBuildFlags flags [optional]
        * @return list of shader resources
        */
        template<class TResource = renderer::Shader, class TResourceLoader>
        [[nodiscard]] std::vector<const TResource*> loadHLSLShaders(renderer::Device* device, std::string filename, const std::vector<std::tuple<std::string, renderer::ShaderType>>& entryPoints,
            const renderer::Shader::DefineList& defines = {}, const std::vector<std::string>& includes = {}, ShaderCompileFlags flags = 0);

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
    inline const TResource* ResourceManager::loadShader(renderer::Device* device, const std::string& filename, renderer::ShaderType type,
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
            TResourceLoader loader(device, type, entrypoint, defines, includes, flags);
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
    inline std::vector<const TResource*> ResourceManager::loadHLSLShaders(renderer::Device* device, std::string filename, const std::vector<std::tuple<std::string, renderer::ShaderType>>& entryPoints,
        const renderer::Shader::DefineList& defines, const std::vector<std::string>& includes, ShaderCompileFlags flags)
    {
        static_assert(std::is_base_of<renderer::Shader, TResource>(), "wrong type");
        std::string innerName(filename);
        std::transform(filename.begin(), filename.end(), innerName.begin(), ::tolower);

        renderer::Shader::DefineList innerDefines(defines);
        std::sort(innerDefines.begin(), innerDefines.end(), [](const std::pair<std::string, std::string>& macros1, const std::pair<std::string, std::string>& macros2) -> bool
            {
                return macros1.first < macros2.first;
            });

        auto composeResourceName = [](const std::string& name, const std::vector<std::pair<std::string, std::string>>& defines) -> std::string
        {
            std::string outString = name;
            for (auto& define : defines)
            {
                outString.append("#");
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
                TResourceLoader loader(device, std::get<1>(entryPoint), std::get<0>(entryPoint), defines, includes, flags);
                Resource* res = loader.load(innerName);
                if (!res)
                {
                    m_resources.erase(resourceIter.first);
                    resources.push_back(nullptr);

                    continue;
                }

                resourceIter.first->second = res;
            }

            resources.push_back(static_cast<TResource*>(resourceIter.first->second));
        }

        return resources;
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

    inline void ResourceManager::clear()
    {
        for (auto& iter : m_resources)
        {
            Resource* res = iter.second;
            V3D_DELETE(res, memory::MemoryLabel::MemoryObject);
        }
        m_resources.clear();
    }

    inline bool ResourceManager::remove(Resource* resource)
    {
        for (auto& iter : m_resources)
        {
            Resource* res = iter.second;
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

    inline bool ResourceManager::remove(const Resource* resource)
    {
        for (auto& iter : m_resources)
        {
            const Resource* res = iter.second;
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
