#pragma once

#include "ResourceLoader.h"
#include "ResourceDecoderRegistration.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AssetSourceFileLoader class. Loader an asset from the file
    * @see Resource. Supports internal format ".v3dasset"
    */
    class AssetSourceFileLoader : public ResourceLoader<Resource*>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief AssetSourceFileLoader constructor
        */
        explicit AssetSourceFileLoader(u32 flags = 0) noexcept;

        /**
        * @brief AssetSourceFileLoader destructor
        */
        ~AssetSourceFileLoader() = default;

        /**
        * @brief Load a resource by name from the file
        * @see Resource
        * @param const std::string& name [required]
        * @param const std::string& alias [optional]
        * @return Resource pointer
        */
        [[nodiscard]] Resource* load(const std::string& name, const std::string& alias = "") override;

    private:

        AssetSourceFileLoader(const AssetSourceFileLoader&) = delete;
        AssetSourceFileLoader& operator=(const AssetSourceFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
