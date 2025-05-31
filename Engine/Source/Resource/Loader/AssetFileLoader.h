#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AssetFileLoader class. Loader an asset from the file
    * @see Resource. Supports internal format ".v3dasset"
    */
    class AssetFileLoader : public ResourceLoader<Resource*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief AssetLoaderFlag enum
        */
        enum AssetLoaderFlag : u32
        {
            AssetLoaderEmpty = 0,
        };
        typedef u32 AssetLoaderFlags;

        /**
        * @brief AssetFileLoader constructor
        */
        explicit AssetFileLoader(AssetLoaderFlags flags = 0) noexcept;

        /**
        * @brief AssetFileLoader destructor
        */
        ~AssetFileLoader() = default;

        /**
        * @brief Load a resource by name from the file
        * @see Resource
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Resource pointer
        */
        [[nodiscard]] Resource* load(const std::string& name, const std::string& alias = "") override;

    private:

        AssetFileLoader(const AssetFileLoader&) = delete;
        AssetFileLoader& operator=(const AssetFileLoader&) = delete;

        AssetLoaderFlags m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
