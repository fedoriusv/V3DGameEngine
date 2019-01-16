#include "Framebuffer.h"

#include "Context.h"
#include "Utils/Logger.h"

#include "crc32c/crc32c.h"

namespace v3d
{
namespace renderer
{

Framebuffer::Framebuffer() noexcept 
    : m_key(0)
{
}

Framebuffer::~Framebuffer() 
{
}


FramebufferManager::FramebufferManager(Context * context) noexcept
    : m_context(context)
{
}

FramebufferManager::~FramebufferManager()
{
    FramebufferManager::clear();
}

Framebuffer * FramebufferManager::acquireFramebuffer(const RenderPass * renderpass, const std::vector<Image*>& images, const core::Dimension2D & size)
{
    u32 hash = crc32c::Crc32c((char*)images.data(), images.size() * sizeof(Image*));

    Framebuffer* framebuffer = nullptr;
    auto found = m_framebuffers.emplace(hash, framebuffer);
    if (found.second)
    {
        framebuffer = m_context->createFramebuffer(images, size);
        framebuffer->m_key = hash;

        if (!framebuffer->create(renderpass))
        {
            framebuffer->destroy();
            m_framebuffers.erase(hash);

            ASSERT(false, "can't create framebuffer");
            return nullptr;
        }
        found.first->second = framebuffer;
        framebuffer->registerNotify(this);

        return framebuffer;
    }

    return found.first->second;
}

bool FramebufferManager::removeFramebuffer(const std::vector<Image*>& images)
{
    u32 hash = crc32c::Crc32c((char*)images.data(), images.size() * sizeof(Image*));

    auto iter = m_framebuffers.find(hash);
    if (iter == m_framebuffers.cend())
    {
        LOG_DEBUG("FramebufferManager renderpass not found");
        ASSERT(false, "renderpass");
        return false;
    }

    Framebuffer* framebuffer = iter->second;
    framebuffer->notifyObservers();

    framebuffer->destroy();
    delete framebuffer;

    return true;
}

bool FramebufferManager::removeFramebuffer(Framebuffer * framebufer)
{
    auto iter = m_framebuffers.find(framebufer->m_key);
    if (iter == m_framebuffers.cend())
    {
        LOG_DEBUG("FramebufferManager renderpass not found");
        ASSERT(false, "renderpass");
        return false;
    }

    Framebuffer* framebuffer = iter->second;
    framebuffer->notifyObservers();

    framebuffer->destroy();
    delete framebuffer;

    return true;
}

void FramebufferManager::handleNotify(utils::Observable * ob)
{
    LOG_DEBUG("RenderPassManager renderpass %x has been deleted", ob);
    m_framebuffers.erase(static_cast<Framebuffer*>(ob)->m_key);
}

void FramebufferManager::clear()
{
    for (auto& renderpass : m_framebuffers)
    {
        renderpass.second->destroy();
        renderpass.second->notifyObservers();

        delete renderpass.second;
    }
    m_framebuffers.clear();
}

} //namespace renderer
} //namespace v3d
