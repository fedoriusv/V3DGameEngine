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
    * @brief AssetFileLoader class. Loader an asset from the file
    * @see Resource. Supports internal format ".v3dasset"
    */
    class AssetFileLoader : public ResourceLoader<Asset>, public ResourceDecoderRegistration
    {
    public:

        /**
        * @brief AssetFileLoader constructor
        */
        explicit AssetFileLoader() noexcept;

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
        [[nodiscard]] Asset* load(const std::string& name, const resource::Resource::LoadPolicy& policy, u32 flags = 0) override;

    private:

        AssetFileLoader(const AssetFileLoader&) = delete;
        AssetFileLoader& operator=(const AssetFileLoader&) = delete;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
