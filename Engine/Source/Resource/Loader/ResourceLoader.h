#pragma once

#include "Common.h"
#include "Utils/ResourceID.h"
#include "Resource/Resource.h"
#include "Resource/Decoder/ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class BaseLoader
    {
    public:

        BaseLoader() noexcept = default;
        virtual ~BaseLoader() = default;
    };

    /**
    * @brief Interface of ResourceLoader
    */
    template <class TResource>
    class ResourceLoader : public BaseLoader
    {
    public:

        using ResourceType = TResource;
        using PolicyType = Resource::LoadPolicy;

        /**
        * @brief ResourceLoader constructor
        */
        ResourceLoader() noexcept;

        /**
        * @brief ResourceLoader destructor
        */
        virtual ~ResourceLoader();

        /**
        * @brief Load resource by name
        * @param const std::string& name [required]
        * @param const PolicyType& policy [required]
        * @param  u32 flags [optional]
        * @return Texture pointer
        */
        [[nodiscard]] virtual TResource* load(const std::string& name, const Resource::LoadPolicy& policy, u32 flags = 0) = 0;

        /**
        * @brief addRoot interface.
        * Register a new root foder
        * @param const std::string& path [required]
        */
        void addRoot(const std::string& path);

        /**
        * @brief removeRoot interface.
        * Unregister root folder, remove from list
        * @param const std::string& path [required]
        */
        void removeRoot(const std::string& path);

        /**
        * @brief addPath interface.
        * Register a new path to the resource
        * @param const std::string& path [required]
        */
        void addPath(const std::string& path);

        /**
        * @brief addPaths interface.
        * Register new pathes to the resources
        * @param const std::vector<std::string> &paths [required]
        */
        void addPaths(const std::vector<std::string> &paths);

        /**
        * @brief removePath interface.
        * Unregister the path, remove from list
        * @param const std::string& path [required]
        */
        void removePath(const std::string& path);

        /**
        * @brief removePathes interface.
        * Unregister pathes, remove from list
        * @param const std::vector<std::string> &pathes [required]
        */
        void removePathes(const std::vector<std::string>& pathes);

    protected:

        std::vector<std::string> m_roots;
        std::vector<std::string> m_paths;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TResource>
    inline ResourceLoader<TResource>::ResourceLoader() noexcept
    {
        ResourceLoader<TResource>::addRoot("");
        ResourceLoader<TResource>::addPath("");
    }

    template<class TResource>
    inline ResourceLoader<TResource>::~ResourceLoader()
    {
        m_roots.clear();
        m_paths.clear();
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::addRoot(const std::string& path)
    {
        std::string innerRoot(path);
        std::transform(innerRoot.begin(), innerRoot.end(), innerRoot.begin(), ::tolower);

        auto it = std::find(m_roots.begin(), m_roots.end(), innerRoot);
        if (it == m_roots.end())
        {
            m_roots.push_back(innerRoot);
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::removeRoot(const std::string& path)
    {
        auto it = std::find(m_roots.begin(), m_roots.end(), path);
        if (it != m_roots.end())
        {
            m_roots.erase(std::remove(m_roots.begin(), m_roots.end(), *it), m_roots.end());
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::addPath(const std::string & path)
    {
        std::string innerPath(path);
        std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(), ::tolower);

        auto it = std::find(m_paths.begin(), m_paths.end(), innerPath);
        if (it == m_paths.end())
        {
            m_paths.push_back(innerPath);
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::addPaths(const std::vector<std::string>& paths)
    {
        for (const std::string& path : paths)
        {
            std::string innerPath(path);
            std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(), ::tolower);

            auto it = std::find(m_paths.begin(), m_paths.end(), innerPath);
            if (it == m_paths.end())
            {
                m_paths.push_back(innerPath);
            }
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::removePath(const std::string & path)
    {
        auto it = std::find(m_paths.begin(), m_paths.end(), path);
        if (it != m_paths.end())
        {
            m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), *it), m_paths.end());
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::removePathes(const std::vector<std::string>& pathes)
    {
        for (const std::string& path : m_paths)
        {
            auto it = std::find(m_paths.begin(), m_paths.end(), path);
            if (it != m_paths.end())
            {
                m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), *it), m_paths.end());
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
