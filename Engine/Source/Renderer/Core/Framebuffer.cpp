#include "Framebuffer.h"
#include "Context.h"
#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

Framebuffer::FramebufferDescription::FramebufferDescription() noexcept
    : _hash(0)
{
}

Framebuffer::FramebufferDescription::FramebufferDescription(u64 size) noexcept
    : _hash(0)
{
    _imagesDesc.reserve(size);
}

bool Framebuffer::FramebufferDescription::operator==(const FramebufferDescription& desc) const
{
    if (&desc == this)
    {
        return true;
    }

    return _imagesDesc == desc._imagesDesc;
}

u64 Framebuffer::FramebufferDescription::Hash::operator()(const FramebufferDescription& desc) const
{
    ASSERT(desc._hash != 0, "empty hash");
    return desc._hash;
}

bool Framebuffer::FramebufferDescription::Compare::operator()(const FramebufferDescription& op1, const FramebufferDescription& op2) const
{
    ASSERT(op1._hash != 0 && op2._hash != 0, "must be not 0");
    if (op1._hash != op2._hash)
    {
        return false;
    }

    return op1._imagesDesc == op2._imagesDesc;
}

FramebufferManager::FramebufferManager(Context* context) noexcept
    : m_context(context)
{
}

FramebufferManager::~FramebufferManager()
{
    FramebufferManager::clear();
}

std::tuple<Framebuffer*, bool> FramebufferManager::acquireFramebuffer(const RenderPass* renderpass, const std::vector<Image*>& images, const math::Dimension2D& area)
{
    auto buildFramebufferDescription = [&](Framebuffer::FramebufferDescription& desc) -> void
    {
        ASSERT((renderpass->getDescription()._countColorAttachments + (renderpass->getDescription()._hasDepthStencilAttahment ? 1 : 0)) == (u32)images.size(), "diff size");
        std::vector<u64> indexes(images.size());
        std::vector<s32> layers(images.size());

        for (u32 index = 0; index < renderpass->getDescription()._countColorAttachments; ++index)
        {
            const Image* image = images[index];
            ASSERT(image, "nullptr");

            s32 layer = AttachmentDescription::uncompressLayer(renderpass->getDescription()._attachments[index]._layer);
            desc._imagesDesc.emplace_back(image, layer);

            indexes[index] = image->ID();
            layers[index] = layer;
        }

        if (renderpass->getDescription()._hasDepthStencilAttahment)
        {
            const Image* image = images[static_cast<u32>(images.size() - 1)];
            ASSERT(image, "nullptr");

            s32 layer = AttachmentDescription::uncompressLayer(renderpass->getDescription()._attachments[k_maxColorAttachments]._layer);
            desc._imagesDesc.emplace_back(image, layer);
        }

        desc._hash = (u64)(static_cast<u32>(desc._imagesDesc.size())) << 32;

        u32 hash = crc32c::Crc32c((c8*)indexes.data(), indexes.size() * sizeof(u64));
        desc._hash |= crc32c::Extend(hash, (u8*)layers.data(), layers.size() * sizeof(s32));
    };

    Framebuffer::FramebufferDescription desc(images.size());
    buildFramebufferDescription(desc);

    Framebuffer* framebuffer = nullptr;
    auto found = m_framebufferList.emplace(desc, framebuffer);
    if (found.second)
    {
        framebuffer = m_context->createFramebuffer(images, area);
        framebuffer->m_desc = std::move(desc);

        if (!framebuffer->create(renderpass))
        {
            framebuffer->destroy();
            m_framebufferList.erase(found.first);

            ASSERT(false, "can't create framebuffer");
            return std::make_tuple(nullptr, false);
        }
        found.first->second = framebuffer;
        framebuffer->registerNotify(this);

        return std::make_tuple(framebuffer, true);
    }

    return std::make_tuple(found.first->second, false);
}

bool FramebufferManager::removeFramebuffer(Framebuffer* frameBuffer)
{

    auto iter = m_framebufferList.find(frameBuffer->m_desc);
    if (iter == m_framebufferList.cend())
    {
        LOG_DEBUG("FramebufferManager framebuffer not found");
        ASSERT(false, "frameBuffer");
        return false;
    }

    Framebuffer* framebuffer = iter->second;
    ASSERT(framebuffer == frameBuffer, "Different pointers");
    if (framebuffer->linked())
    {
        LOG_WARNING("FramebufferManager::removeFramebuffer framebufer still linked, but reqested to delete");
        ASSERT(false, "framebuffer");
        //return false;
    }
    m_framebufferList.erase(iter);

    framebuffer->notifyObservers();

    framebuffer->destroy();
    delete framebuffer;

    return true;
}

void FramebufferManager::clear()
{
    for (auto& iter : m_framebufferList)
    {
        Framebuffer* framebuffer = iter.second;
        if (framebuffer->linked())
        {
            LOG_WARNING("FramebufferManager::removeFramebuffer framebufer still linked, but reqested to delete");
            ASSERT(false, "framebuffer");
            //return false;
        }
        framebuffer->notifyObservers();

        framebuffer->destroy();
        delete framebuffer;
    }
    m_framebufferList.clear();
}

void FramebufferManager::handleNotify(const utils::Observable* object, void* msg)
{
    LOG_DEBUG("FramebufferManager framebuffer %x has been deleted", object);
}

} //namespace renderer
} //namespace v3d
