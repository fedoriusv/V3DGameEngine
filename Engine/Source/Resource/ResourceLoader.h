#pragma once

#include "Common.h"
#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Interface of ResourceLoader
    */
    template <class T>
    class ResourceLoader
    {
    public:

        ResourceLoader() noexcept;
        virtual ~ResourceLoader();

        virtual T load(const std::string& name, const std::string& alias = "") = 0;

        void registerDecoder(ResourceDecoder* decoder);
        void unregisterDecoder(ResourceDecoder* decoder);
        void unregisterAllDecoders();

        void registerPath(const std::string& path);
        void registerPathes(const std::vector<std::string> &pathes);

        void unregisterPath(const std::string& path);
        void unregisterPathes(const std::vector<std::string>& pathes);

    protected:

        ResourceDecoder* findDecoder(const std::string& extension);

        std::vector<ResourceDecoder*> m_decoders;
        std::vector<std::string>      m_pathes;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    inline ResourceLoader<T>::ResourceLoader() noexcept
    {
    }

    template<class T>
    inline ResourceLoader<T>::~ResourceLoader()
    {
        m_pathes.clear();
        ResourceLoader<T>::unregisterAllDecoders();
    }

    template<class T>
    inline void ResourceLoader<T>::registerDecoder(ResourceDecoder * decoder)
    {
        auto it = std::find(m_decoders.begin(), m_decoders.end(), decoder);
        if (it == m_decoders.end())
        {
            m_decoders.push_back(decoder);
        }
    }

    template<class T>
    inline void ResourceLoader<T>::unregisterDecoder(ResourceDecoder * decoder)
    {
        auto it = std::find(m_decoders.begin(), m_decoders.end(), decoder);
        if (it != m_decoders.end())
        {
            m_decoders.erase(std::remove(m_decoders.begin(), m_decoders.end(), *it), m_decoders.end());
            delete *it;
        }
    }

    template<class T>
    inline void ResourceLoader<T>::unregisterAllDecoders()
    {
        for (auto decoder : m_decoders)
        {
            delete decoder;
        }
        m_decoders.clear();
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

    template<class T>
    inline ResourceDecoder * ResourceLoader<T>::findDecoder(const std::string & extension)
    {
        auto predCanDecode = [extension](const ResourceDecoder* decoder) -> bool
        {
            return decoder->isExtensionSupported(extension);
        };

        auto iter = std::find_if(m_decoders.begin(), m_decoders.end(), predCanDecode);
        if (iter == m_decoders.end())
        {
            return nullptr;
        }

        return (*iter);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
