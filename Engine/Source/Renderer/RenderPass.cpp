#include "RenderPass.h"

#include "Context.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

RenderPass::RenderPass() noexcept
    : m_key(0)
{
}

RenderPass::~RenderPass()
{
}


RenderPassManager::RenderPassManager(Context * context) noexcept
    : m_context(context)
{
}

RenderPassManager::~RenderPassManager()
{
   RenderPassManager::clear();
}

RenderPass* RenderPassManager::acquireRenderPass(const RenderPass::RenderPassInfo& desc)
{
    RenderPassDescription pDesc;
    pDesc._info = desc;

    RenderPass* renderpass = nullptr;
    auto found = m_renderpasses.emplace(pDesc._hash, renderpass);
    if (found.second)
    {
        renderpass = m_context->createRenderPass(&pDesc._info);
        renderpass->m_key = pDesc._hash;

        if (!renderpass->create())
        {
            renderpass->destroy();
            m_renderpasses.erase(pDesc._hash);

            ASSERT(false, "can't create renderpass");
            return nullptr;
        }
        found.first->second = renderpass;
        renderpass->registerNotify(this);

        return renderpass;
    }

    return found.first->second;
}

bool RenderPassManager::removeRenderPass(const RenderPass::RenderPassInfo& desc)
{
    RenderPassDescription pDesc;
    pDesc._info = desc;

    auto iter = m_renderpasses.find(pDesc._hash);
    if (iter == m_renderpasses.cend())
    {
        LOG_DEBUG("RenderPassManager renderpass not found");
        ASSERT(false, "renderpass");
        return false;
    }

    RenderPass* renderpass = iter->second;
    renderpass->notifyObservers();

    renderpass->destroy();
    delete renderpass;

    return true;
}

bool RenderPassManager::removeRenderPass(const RenderPass * renderPass)
{
    auto iter = m_renderpasses.find(renderPass->m_key);
    if (iter == m_renderpasses.cend())
    {
        LOG_DEBUG("RenderPassManager renderpass not found");
        ASSERT(false, "renderpass");
        return false;
    }

    RenderPass* renderpass = iter->second;
    renderpass->notifyObservers();

    renderpass->destroy();
    delete renderpass;

    return true;
}

void RenderPassManager::handleNotify(utils::Observable* ob)
{
    LOG_DEBUG("RenderPassManager renderpass %x has been deleted", ob);
    m_renderpasses.erase(static_cast<RenderPass*>(ob)->m_key);
}

void RenderPassManager::clear()
{
    for (auto& renderpass : m_renderpasses)
    {
        renderpass.second->notifyObservers();

        renderpass.second->destroy();
        delete renderpass.second;
    }
    m_renderpasses.clear();
}

} //namespace renderer
} //namespace v3d
