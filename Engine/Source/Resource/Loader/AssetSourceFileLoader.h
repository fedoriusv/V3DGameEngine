#pragma once

#include "ResourceLoader.h"
#include "Resource/Decoder/ResourceDecoderRegistration.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Asset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AssetSourceFileLoader class. Loader an asset from the file
    * @see Resource. Supports internal format ".v3dasset"
    */
    class AssetSourceFileLoader : public ResourceLoader<Asset>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief AssetSourceFileLoader constructor
        */
        AssetSourceFileLoader() noexcept;

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
        [[nodiscard]] Asset* load(const std::string& name, const resource::Resource::LoadPolicy& policy, u32 flags = 0) override;

    private:

        AssetSourceFileLoader(const AssetSourceFileLoader&) = delete;
        AssetSourceFileLoader& operator=(const AssetSourceFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
