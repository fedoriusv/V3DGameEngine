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

RenderPass* RenderPassManager::acquireRenderPass(const RenderPassDescription& renderPassInfo)
{
    RenderPass::RenderPassInfo::RenderPassDesc info;
    info._desc = renderPassInfo;

    RenderPass* renderpass = nullptr;
    auto found = m_renderPassList.emplace(info._hash, renderpass);
    if (found.second)
    {
        renderpass = m_context->createRenderPass(&info._desc);
        renderpass->m_key = info._hash;

        if (!renderpass->create())
        {
            renderpass->destroy();
            m_renderPassList.erase(info._hash);

            ASSERT(false, "can't create renderpass");
            return nullptr;
        }
        found.first->second = renderpass;
        renderpass->registerNotify(this);

        return renderpass;
    }

    return found.first->second;
}

bool RenderPassManager::removeRenderPass(const RenderPass * renderPass)
{
    auto iter = m_renderPassList.find(renderPass->m_key);
    if (iter == m_renderPassList.cend())
    {
        LOG_DEBUG("RenderPassManager renderpass not found");
        ASSERT(false, "renderpass");
        return false;
    }

    RenderPass* renderpass = iter->second;
    ASSERT(renderpass == renderPass, "Different pointers");
    if (renderPass->linked())
    {
        LOG_WARNING("RenderPassManager::removeRenderPass renderPass still linked, but reqested to delete");
        ASSERT(false, "renderpass");
        //return false;
    }
    m_renderPassList.erase(iter);

    renderpass->notifyObservers();

    renderpass->destroy();
    delete renderpass;

    return true;
}

void RenderPassManager::clear()
{
    for (auto& iter : m_renderPassList)
    {
        RenderPass* renderpass = iter.second;
        if (renderpass->linked())
        {
            LOG_WARNING("RenderPassManager::removeRenderPass renderPass still linked, but reqested to delete");
            ASSERT(false, "renderpass");
            //return false;
        }
        renderpass->notifyObservers();

        renderpass->destroy();
        delete renderpass;
    }
    m_renderPassList.clear();
}

void RenderPassManager::handleNotify(utils::Observable* ob)
{
    LOG_DEBUG("RenderPassManager renderpass %x has been deleted", ob);
}

} //namespace renderer
} //namespace v3d
