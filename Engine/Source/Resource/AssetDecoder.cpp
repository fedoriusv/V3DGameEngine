#include "AssetDecoder.h"
#include "Resource.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"

#include "Bitmap.h"
#include "Scene/Model.h"
#include "Scene/Geometry/Mesh.h"

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

AssetDecoder::AssetDecoder(std::vector<std::string> supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
{
}

Resource* AssetDecoder::decode(const stream::Stream* stream, const std::string& name) const
{
    if (stream->size() > 0)
    {
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
        case ResourceType::ModelResource:
        {
            scene::ModelHeader* modleHeader = V3D_NEW(scene::ModelHeader, memory::MemoryLabel::MemoryResource);
            modleHeader->operator<<(stream);

            resource = V3D_NEW(scene::Model, memory::MemoryLabel::MemoryResource)(modleHeader);
        }
        break;

        case ResourceType::MeshResource:
        {
            scene::MeshHeader* meshHeader = V3D_NEW(scene::MeshHeader, memory::MemoryLabel::MemoryResource);
            meshHeader->operator<<(stream);

            resource = V3D_NEW(scene::Mesh, memory::MemoryLabel::MemoryResource)(meshHeader);
        }
        break;

        case ResourceType::BitmapResource:
        {
            resource::BitmapHeader* meshHeader = V3D_NEW(resource::BitmapHeader, memory::MemoryLabel::MemoryResource);
            meshHeader->operator<<(stream);

            resource = V3D_NEW(resource::Bitmap, memory::MemoryLabel::MemoryResource)(meshHeader);
        }
        break;

        case ResourceType::ShaderResource:
        case ResourceType::MaterialResource:
        default:
            ASSERT(false, "resource is not found");
        };
        static_assert(toEnumType(ResourceType::ResourceType_Count) == 6, "handle all resources and increment the value if is needed");

        ASSERT(resource, "nullptr");
        if (!resource->load(stream))
        {
            LOG_ERROR("AssetDecoder::decode: load is falied, %s", name.c_str());

            V3D_DELETE(resource, memory::MemoryLabel::MemoryResource);
            resource = nullptr;
        }

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_INFO("AssetDecoder::decode: the resource %s[%s], is loaded. Time %.4f sec", name.c_str(), generalHeader._name, static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return resource;
    }

    ASSERT(false, "empty");
    return nullptr;
}
} //namespace resource
} //namespace v3d