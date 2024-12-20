#pragma once

#include "ResourceDecoder.h"

//namespace nlohmann
//{
//    class basic_json;
//} // json
namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

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

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        void parseScene();
        void parseTechnique();
        //void parseMaterial(const nlohmann::json& data);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d