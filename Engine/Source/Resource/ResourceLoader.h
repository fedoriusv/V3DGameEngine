#pragma once

#include "Common.h"
#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Interface of ResourceLoader
    */
    template <class TResource>
    class ResourceLoader
    {
    public:

        /**
        * @brief ResourceLoader constructor
        */
        ResourceLoader() noexcept;

        /**
        * @brief ResourceLoader destructor
        */
        virtual ~ResourceLoader();

        /**
        * @brief load interface.
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return TResource
        */
        [[nodiscard]] virtual TResource load(const std::string& name, const std::string& alias = "") = 0;

        void registerRoot(const std::string& path);
        void unregisterRoot(const std::string& path);

        /**
        * @brief registerPath interface.
        * Register a new path to the resource
        * @param const std::string& path [required]
        */
        void registerPath(const std::string& path);

        /**
        * @brief registerPathes interface.
        * Register new pathes to the resources
        * @param const std::vector<std::string> &pathes [required]
        */
        void registerPathes(const std::vector<std::string> &pathes);

        /**
        * @brief unregisterPath interface.
        * Unregister the path, remove from list
        * @param const std::string& path [required]
        */
        void unregisterPath(const std::string& path);

        /**
        * @brief unregisterPathes interface.
        * Unregister pathes, remove from list
        * @param const std::vector<std::string> &pathes [required]
        */
        void unregisterPathes(const std::vector<std::string>& pathes);

    protected:

        std::vector<std::string> m_roots;
        std::vector<std::string> m_pathes;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TResource>
    inline ResourceLoader<TResource>::ResourceLoader() noexcept
    {
    }

    template<class TResource>
    inline ResourceLoader<TResource>::~ResourceLoader()
    {
        m_roots.clear();
        m_pathes.clear();
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::registerRoot(const std::string& path)
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
    inline void ResourceLoader<TResource>::unregisterRoot(const std::string& path)
    {
        auto it = std::find(m_roots.begin(), m_roots.end(), path);
        if (it != m_roots.end())
        {
            m_roots.erase(std::remove(m_roots.begin(), m_roots.end(), *it), m_roots.end());
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::registerPath(const std::string & path)
    {
        std::string innerPath(path);
        std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(), ::tolower);

        auto it = std::find(m_pathes.begin(), m_pathes.end(), innerPath);
        if (it == m_pathes.end())
        {
            m_pathes.push_back(innerPath);
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::registerPathes(const std::vector<std::string>& pathes)
    {
        for (const std::string& path : pathes)
        {
            std::string innerPath(path);
            std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(), ::tolower);

            auto it = std::find(m_pathes.begin(), m_pathes.end(), innerPath);
            if (it == m_pathes.end())
            {
                m_pathes.push_back(innerPath);
            }
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::unregisterPath(const std::string & path)
    {
        auto it = std::find(m_pathes.begin(), m_pathes.end(), path);
        if (it != m_pathes.end())
        {
            m_pathes.erase(std::remove(m_pathes.begin(), m_pathes.end(), *it), m_pathes.end());
        }
    }

    template<class TResource>
    inline void ResourceLoader<TResource>::unregisterPathes(const std::vector<std::string>& pathes)
    {
        for (const std::string& path : m_pathes)
        {
            auto it = std::find(m_pathes.begin(), m_pathes.end(), path);
            if (it != m_pathes.end())
            {
                m_pathes.erase(std::remove(m_pathes.begin(), m_pathes.end(), *it), m_pathes.end());
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
