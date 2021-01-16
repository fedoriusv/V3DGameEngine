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
    template <class T>
    class ResourceLoader
    {
    public:

        ResourceLoader() noexcept;
        virtual ~ResourceLoader();

        /**
        * @brief load interface
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return T
        */
        virtual T load(const std::string& name, const std::string& alias = "") = 0;

        void registerRoot(const std::string& path);
        void unregisterRoot(const std::string& path);

        void registerPath(const std::string& path);
        void registerPathes(const std::vector<std::string> &pathes);

        void unregisterPath(const std::string& path);
        void unregisterPathes(const std::vector<std::string>& pathes);

    protected:

        std::vector<std::string> m_roots;
        std::vector<std::string> m_pathes;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    inline ResourceLoader<T>::ResourceLoader() noexcept
    {
    }

    template<class T>
    inline ResourceLoader<T>::~ResourceLoader()
    {
        m_roots.clear();
        m_pathes.clear();
    }

    template<class T>
    inline void ResourceLoader<T>::registerRoot(const std::string& path)
    {
        std::string innerRoot(path);
        std::transform(innerRoot.begin(), innerRoot.end(), innerRoot.begin(), ::tolower);

        auto it = std::find(m_roots.begin(), m_roots.end(), innerRoot);
        if (it == m_roots.end())
        {
            m_roots.push_back(innerRoot);
        }
    }

    template<class T>
    inline void ResourceLoader<T>::unregisterRoot(const std::string& path)
    {
        auto it = std::find(m_roots.begin(), m_roots.end(), path);
        if (it != m_roots.end())
        {
            m_roots.erase(std::remove(m_roots.begin(), m_roots.end(), *it), m_roots.end());
        }
    }

    template<class T>
    inline void ResourceLoader<T>::registerPath(const std::string & path)
    {
        std::string innerPath(path);
        std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(), ::tolower);

        auto it = std::find(m_pathes.begin(), m_pathes.end(), innerPath);
        if (it == m_pathes.end())
        {
            m_pathes.push_back(innerPath);
        }
    }

    template<class T>
    inline void ResourceLoader<T>::registerPathes(const std::vector<std::string>& pathes)
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

    template<class T>
    inline void ResourceLoader<T>::unregisterPath(const std::string & path)
    {
        auto it = std::find(m_pathes.begin(), m_pathes.end(), path);
        if (it != m_pathes.end())
        {
            m_pathes.erase(std::remove(m_pathes.begin(), m_pathes.end(), *it), m_pathes.end());
        }
    }

    template<class T>
    inline void ResourceLoader<T>::unregisterPathes(const std::vector<std::string>& pathes)
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
