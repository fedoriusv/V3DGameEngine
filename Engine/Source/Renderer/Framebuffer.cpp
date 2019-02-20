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

Framebuffer * FramebufferManager::acquireFramebuffer(const RenderPass* renderpass, const Framebuffer::FramebufferInfo& framebufferInfo)
{
    u32 hash = crc32c::Crc32c((char*)framebufferInfo._images.data(), framebufferInfo._images.size() * sizeof(Image*));

    Framebuffer* framebuffer = nullptr;
    auto found = m_framebufferList.emplace(hash, framebuffer);
    if (found.second)
    {
        framebuffer = m_context->createFramebuffer(framebufferInfo._images, framebufferInfo._clearInfo._size);
        framebuffer->m_key = hash;

        if (!framebuffer->create(renderpass))
        {
            framebuffer->destroy();
            m_framebufferList.erase(hash);

            ASSERT(false, "can't create framebuffer");
            return nullptr;
        }
        found.first->second = framebuffer;
        framebuffer->registerNotify(this);

        return framebuffer;
    }

    return found.first->second;
}

bool FramebufferManager::removeFramebuffer(Framebuffer * frameBuffer)
{

    auto iter = m_framebufferList.find(frameBuffer->m_key);
    if (iter == m_framebufferList.cend())
    {
        LOG_DEBUG("FramebufferManager framebuffer not found");
        ASSERT(false, "renderpass");
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

void FramebufferManager::handleNotify(utils::Observable * ob)
{
    LOG_DEBUG("FramebufferManager framebuffer %x has been deleted", ob);
}

} //namespace renderer
} //namespace v3d
