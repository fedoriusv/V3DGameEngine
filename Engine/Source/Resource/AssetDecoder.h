#pragma once

#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AssetDecoder decoder.
    */
    class AssetDecoder final : public ResourceDecoder
    {
    public:

        explicit AssetDecoder(std::vector<std::string> supportedExtensions) noexcept;
        ~AssetDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d