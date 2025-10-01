#include "AssetDecoder.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#include "Resource/Resource.h"

#include "Resource/Bitmap.h"
//#include "Scene/Model.h"
//#include "Scene/Material.h"
//#include "Scene/StaticMesh.h"
#include "Renderer/Shader.h"

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

 AssetDecoder::AssetDecoder() noexcept
 {
 }

 AssetDecoder::AssetDecoder(const std::vector<std::string>& supportedExtensions) noexcept
     : ResourceDecoder(supportedExtensions)
 {
 }

 AssetDecoder::AssetDecoder(std::vector<std::string>&& supportedExtensions) noexcept
     : ResourceDecoder(std::move(supportedExtensions))
 {
 }

Resource* AssetDecoder::decode(const stream::Stream* stream, const Policy* policy, u32 flags, const std::string& name) const
{
    if (!stream || stream->size() == 0)
    {

        ASSERT(false, "stream is invalid");
        return nullptr;
    }

    stream->seekBeg(0);

#if LOG_LOADIMG_TIME
    utils::Timer timer;
    timer.start();
#endif //LOG_LOADIMG_TIME

    ResourceHeader generalHeader;
    generalHeader << stream;
    stream->seekBeg(0);

    Resource* resource = nullptr;
    switch (generalHeader.getResourceType())
    {
    case ResourceType::Technique:
    {
        //scene::ModelHeader* modleHeader = V3D_NEW(scene::ModelHeader, memory::MemoryLabel::MemoryObject);
        //modleHeader->operator<<(stream);

        //resource = ::V3D_NEW(scene::Model, memory::MemoryLabel::MemoryObject)(modleHeader);
    }
    break;

    case ResourceType::Model:
    {
        //scene::ModelHeader* modleHeader = V3D_NEW(scene::ModelHeader, memory::MemoryLabel::MemoryObject);
        //modleHeader->operator<<(stream);

        //resource = ::V3D_NEW(scene::Model, memory::MemoryLabel::MemoryObject)(modleHeader);
    }
    break;

    case ResourceType::Bitmap:
    {
        //resource::BitmapHeader* meshHeader = V3D_NEW(resource::BitmapHeader, memory::MemoryLabel::MemoryObject);
        //meshHeader->operator<<(stream);

        //resource = ::V3D_NEW(resource::Bitmap, memory::MemoryLabel::MemoryObject)(meshHeader);
    }
    break;

    case ResourceType::Shader:
    {
        //renderer::Shader::ShaderHeader shaderHeader;
        //shaderHeader<<(stream);

        //resource = V3D_NEW(renderer::Shader, memory::MemoryLabel::MemoryObject)(shaderHeader);
    }
    break;

    default:
        ASSERT(false, "resource is not found");
    };
    static_assert(toEnumType(ResourceType::Count) == 11, "handle all resources and increment the value if is needed");

    ASSERT(resource, "nullptr");
    if (!resource->load(stream))
    {
        LOG_ERROR("AssetDecoder::decode: load is falied, %s", name.c_str());

        V3D_DELETE(resource, memory::MemoryLabel::MemoryObject);
        resource = nullptr;
    }

#if LOG_LOADIMG_TIME
    timer.stop();
    u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
    LOG_INFO("AssetDecoder::decode: the resource %s[%s], is loaded. Time %.4f sec", name.c_str(), generalHeader._name, static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

    return resource;
}

} //namespace resource
} //namespace v3d