#include "RenderPass.h"

#include "Context.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

RenderPass::RenderPass(const RenderPassDescription& desc) noexcept
    : m_desc(desc)
{
}

RenderPass::~RenderPass()
{
}

const RenderPassDescription::RenderPassDesc& RenderPass::getDescription() const
{
    return m_desc._desc;
}

RenderPassManager::RenderPassManager(Context* context) noexcept
    : m_context(context)
{
}

RenderPassManager::~RenderPassManager()
{
   RenderPassManager::clear();
}

RenderPass* RenderPassManager::acquireRenderPass(const RenderPassDescription& renderPassInfo)
{
    RenderPass* renderpass = nullptr;
    auto found = m_renderPassList.emplace(renderPassInfo, renderpass);
    if (found.second)
    {
        renderpass = m_context->createRenderPass(&renderPassInfo);
        if (!renderpass->create())
        {
            renderpass->destroy();
            m_renderPassList.erase(found.first);

            ASSERT(false, "can't create renderpass");
            return nullptr;
        }
        found.first->second = renderpass;
        renderpass->registerNotify(this);

        return renderpass;
    }

    return found.first->second;
}

bool RenderPassManager::removeRenderPass(const RenderPass* renderPass)
{
    auto iter = m_renderPassList.find(renderPass->m_desc);
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
        return false;
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
    LOG_DEBUG("RenderPassManager::handleNotify renderpass %llx has been deleted", ob);
}

} //namespace renderer
} //namespace v3d
