#pragma once

#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AssetDecoder decoder.
    */
    class AssetDecoder final : public ResourceDecoder
    {
    public:

        explicit AssetDecoder() noexcept;
        explicit AssetDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit AssetDecoder(std::vector<std::string>&& supportedExtensions) noexcept;

        ~AssetDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const override;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d