#include "AssetJSONDecoder.h"

#include "Utils/Timer.h"
#include "Utils/Logger.h"

#include "Resource/Resource.h"

#include "json/include/nlohmann/json.hpp"
using json = nlohmann::json;

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

static std::map<std::string, resource::ResourceType> k_resourceType =
{
    { "technique", ResourceType::Technique },
    //{ "material", ResourceType::Material },
};

static void parseTechnique(const json& data);
static void parseMaterial(const json& data);

AssetJSONDecoder::AssetJSONDecoder() noexcept
{
}

AssetJSONDecoder::AssetJSONDecoder(const std::vector<std::string>& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
{
}

AssetJSONDecoder::AssetJSONDecoder(std::vector<std::string>&& supportedExtensions) noexcept
    : ResourceDecoder(std::move(supportedExtensions))
{
}

Resource* AssetJSONDecoder::decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags, const std::string& name) const
{
    if (!stream || stream->size() == 0)
    {

        ASSERT(false, "stream is invalid");
        return nullptr;
    }

#if LOG_LOADIMG_TIME
    utils::Timer timer;
    timer.start();
#endif //LOG_LOADIMG_TIME

    stream->seekBeg(0);
    std::string stringData;
    stringData.resize(stream->size());
    stream->read(stringData.data(), stream->size());

    json data = json::parse(stringData);

    u32 verion = data["version"].get<u32>();
    std::string type = data["type"].get<std::string>();
    std::string resourceName = data["name"].get<std::string>();

    auto res = k_resourceType.find(type);
    ASSERT(res != k_resourceType.cend(), "not found");
    switch (res->second)
    {
    case ResourceType::Technique:
        parseTechnique(data);
        break;

    //case ResourceType::Material:
    //    parseMaterial(data);
    //    break;
    }

    std::string s = data.dump();


#if LOG_LOADIMG_TIME
    timer.stop();
    u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
    LOG_INFO("AssetDecoder::decode: the resource %s is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

    return nullptr;
}

void parseMaterial(const json& data)
{
    ASSERT(data["textures"].is_array(), "must be array");
    for (auto& texture : data["textures"])
    {
        std::string name = texture["name"].get<std::string>();
        std::string defaultTexturePath = texture["default"].get<std::string>();
    }

    json shader = data["shader"];
    ASSERT(!shader.is_null(), "must be filled");
    std::string shaderName = shader["name"].get<std::string>();
    std::string shaderPath = shader["path"].get<std::string>();
    std::string shaderFormat = shader["format"].get<std::string>();
    std::string shaderContent = shader["content"].get<std::string>();
    std::string shaderModel = shader["model"].get<std::string>();

    if (!shader["vs"].is_null())
    {
        std::string shaderVs = shader["vs"].get<std::string>();
    }

    if (!shader["ps"].is_null())
    {
        std::string shaderPs = shader["ps"].get<std::string>();
    }

    if (!shader["cs"].is_null())
    {
        std::string shaderCs = shader["cs"].get<std::string>();
    }

    ASSERT(shader["defines"].is_array(), "must be array");
    for (auto& define : shader["defines"])
    {
        std::string def = define.get<std::string>();
        size_t pos = def.find_first_of(' ');
        if (pos != std::string::npos)
        {
            std::string_view strDefine = def.substr(0, pos);
            std::string_view strValue = def.substr(pos, def.size() - pos);
        }

        int text;
    }
}

void parseTechnique(const json& data)
{
    
}

} //namespace resource
} //namespace v3d