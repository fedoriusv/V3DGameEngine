#pragma once

#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AssetJSONDecoder decoder.
    */
    class AssetJSONDecoder final : public ResourceDecoder
    {
    public:

        explicit AssetJSONDecoder() noexcept;
        explicit AssetJSONDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit AssetJSONDecoder(std::vector<std::string>&& supportedExtensions) noexcept;

        ~AssetJSONDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags = 0, const std::string& name = "") const override;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d