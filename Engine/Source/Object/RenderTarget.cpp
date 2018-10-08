#include "RenderTarget.h"
#include "Context.h"
#include "Texture.h"

#include "Renderer/Image.h"
#include "Renderer/RenderPass.h"
#include "Renderer/Framebuffer.h"

#include "Utils/Logger.h"


namespace v3d
{

    /*CreateRenderPassCommand*/
class CreateRenderPassCommand : public renderer::Command
{
public:

    CreateRenderPassCommand(renderer::RenderPass* renderpass)

        : m_renderPass(renderpass) //TODO: need use handle
    {
        LOG_DEBUG("CreateRenderPassCommand constructor");
    }

    ~CreateRenderPassCommand()
    {
        LOG_DEBUG("CreateRenderPassCommand destructor");
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        if (!m_renderPass->create())
        {
            m_renderPass->destroy();
            m_renderPass->notifyObservers();
            delete m_renderPass;

            return;
        }
    }

private:

    renderer::RenderPass* m_renderPass;
};

    /*CreateFramebufferCommand*/
class CreateFramebufferCommand : public renderer::Command, utils::Observer
{
public:

    CreateFramebufferCommand(renderer::Framebuffer* framebuffer, renderer::RenderPass* renderpass, const std::vector<renderer::Image*>& images)
        : m_framebuffer(framebuffer)
        , m_renderPass(renderpass)
        , m_images(images)
    {
        LOG_DEBUG("CreateFramebufferCommand constructor");
        m_renderPass->registerNotify(this);
    }

    ~CreateFramebufferCommand()
    {
        LOG_DEBUG("CreateFramebufferCommand destructor");
        m_renderPass->unregisterNotify(this);
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        if (!m_renderPass)
        {
            return;
        }

        if (!m_framebuffer->create(m_renderPass, m_images))
        {
            m_framebuffer->destroy();
            m_framebuffer->notifyObservers();
            delete m_framebuffer;

            return;
        }
    }

    void handleNotify(utils::Observable* ob) override
    {
        LOG_ERROR("CreateFramebufferCommand renderPass %llx was deleted", m_renderPass);
        m_renderPass = nullptr;
    }

private:

    renderer::Framebuffer* m_framebuffer;
    renderer::RenderPass*  m_renderPass;
    std::vector<renderer::Image*> m_images;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

renderer::RenderPassManager* RenderTarget::s_renderpassManager = nullptr;

RenderTarget::RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& size)
    : m_cmdList(cmdList)
    , m_size(size)

    , m_framebuffer(nullptr)
    , m_renderpass(nullptr)
{
    m_depthStencilTexture.first = nullptr;

    if (!s_renderpassManager)
    {
        s_renderpassManager = new renderer::RenderPassManager(m_cmdList.getContext());
    }
}

RenderTarget::~RenderTarget()
{
    if (m_renderpass)
    {
        m_renderpass->unregisterNotify(this);
    }
}

bool RenderTarget::setColorTexture(Texture2D* colorTexture, renderer::RenderTargetLoadOp loadOp, renderer::RenderTargetStoreOp storeOp)
{
    renderer::AttachmentDescription attachmentDesc = {};
    attachmentDesc._format = colorTexture->getFormat();
    attachmentDesc._samples = colorTexture->getSampleCount();
    attachmentDesc._loadOp = loadOp;
    attachmentDesc._storeOp = storeOp;
    m_colorTextures.emplace_back(colorTexture, attachmentDesc);

    //TODO check compatibility
    return true;
}

bool RenderTarget::setDepthStencilTexture(Texture2D* depthStencilTexture, renderer::RenderTargetLoadOp loadOp, renderer::RenderTargetStoreOp storeOp)
{
    ASSERT(m_depthStencilTexture.first, "attachDepthStencilTexture can create only one");
    if (!m_depthStencilTexture.first)
    {
        renderer::AttachmentDescription attachmentDesc = {};
        attachmentDesc._format = depthStencilTexture->getFormat();
        attachmentDesc._samples = depthStencilTexture->getSampleCount();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        m_depthStencilTexture = std::make_pair(depthStencilTexture, attachmentDesc);
    }

    //TODO check compatibility
    return true;
}

Texture2D * RenderTarget::getColorTexture(u32 attachment) const
{
    ASSERT(attachment < m_colorTextures.size(), "out of range");
    return m_colorTextures[attachment].first;
}

Texture2D * RenderTarget::getDepthStencilTexture() const
{
    return m_depthStencilTexture.first;
}

void RenderTarget::makeRenderTarget()
{
    if (!m_renderpass)
    {
        renderer::RenderPass::RenderPassDescription renderPassInfo;
        memset(renderPassInfo._info._attachments.data(), 0, renderPassInfo._info._attachments.size() * sizeof(renderer::AttachmentDescription));
        renderPassInfo._info._countColorAttachments = static_cast<u32>(m_colorTextures.size());
        for (u32 index = 0; index < renderPassInfo._info._countColorAttachments; ++index)
        {
            renderPassInfo._info._attachments[index] = m_colorTextures[index].second;
        }

        renderPassInfo._info._hasDepthStencilAttahment = false;
        if (m_depthStencilTexture.first)
        {
            renderPassInfo._info._hasDepthStencilAttahment = true;
            renderPassInfo._info._attachments.back() = m_depthStencilTexture.second;
        }

        m_renderpass = s_renderpassManager->acquireRenderPass(renderPassInfo);
        m_renderpass->registerNotify(this);

        if (m_cmdList.isImmediate())
        {
            if (!m_renderpass->create())
            {
                m_renderpass->destroy();
                delete m_renderpass;
                m_renderpass = nullptr;
            }
        }
        else
        {
            m_cmdList.pushCommand(new CreateRenderPassCommand(m_renderpass));
        }
    }

    if (!m_framebuffer)
    {
        std::vector<renderer::Image*> images;
        for (auto& texture : m_colorTextures)
        {
            images.push_back(texture.first->getImage());
        }

        if (m_depthStencilTexture.first)
        {
            images.push_back(m_depthStencilTexture.first->getImage());
        }

        m_framebuffer = m_cmdList.getContext()->createFramebuffer(images, m_size);
        m_framebuffer->registerNotify(this);

        if (m_cmdList.isImmediate())
        {
            if (!m_framebuffer->create(m_renderpass, images))
            {
                m_framebuffer->destroy();
                delete m_framebuffer;
                m_framebuffer = nullptr;
            }
        }
        else
        {
            m_cmdList.pushCommand(new CreateFramebufferCommand(m_framebuffer, m_renderpass, images));
        }
    }
}

void RenderTarget::handleNotify(utils::Observable* ob)
{
    if (m_renderpass == ob)
    {
        m_renderpass = nullptr;
    }

    if (m_framebuffer == ob)
    {
        m_framebuffer = nullptr;
    }
}



Backbuffer::Backbuffer(renderer::CommandList & cmdList, SwapchainTexture * texture)
    : m_cmdList(cmdList)
    , m_texture(texture)
{
}

} //namespace v3d
