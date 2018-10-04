#include "Texture.h"
#include "Context.h"

#include "Utils/Logger.h"
#include "RenderTarget.h"

namespace v3d
{
    RenderTarget::RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& dimension)
        : m_cmdList(cmdList)
        , m_dimension(dimension)

        , m_framebuffer(nullptr)
        , m_renderpass(nullptr)
    {
        m_depthStencilTexture.first = nullptr;
    }

    RenderTarget::~RenderTarget()
    {
    }

    bool RenderTarget::setColorTexture(Texture2D* colorTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp)
    {
        AttachmentDesc attachmentDesc = {};
        attachmentDesc._format = colorTexture->getFormat();
        attachmentDesc._samples = colorTexture->getSampleCount();
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        m_colorTextures.emplace_back(colorTexture, attachmentDesc);

        //TODO check compatibility
        return true;
    }

    bool RenderTarget::setDepthStencilTexture(Texture2D* depthStencilTexture, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp)
    {
        ASSERT(m_depthStencilTexture.first, "attachDepthStencilTexture can create only one");
        if (!m_depthStencilTexture.first)
        {
            AttachmentDesc attachmentDesc = {};
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



    Backbuffer::Backbuffer(renderer::CommandList & cmdList, SwapchainTexture * texture)
        : m_cmdList(cmdList)
        , m_texture(texture)
    {
    }

} //namespace v3d
