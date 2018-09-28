#include "Texture.h"
#include "Context.h"

#include "Utils/Logger.h"
#include "RenderTarget.h"

namespace v3d
{
    RenderTarget::RenderTarget(renderer::CommandList& cmdList, const core::Dimension2D& dimension)
        : m_cmdList(cmdList)
        , m_dimension(dimension)
    {
        m_depthStencilTexture.first = nullptr;
    }

    RenderTarget::~RenderTarget()
    {
    }

    Texture2D* RenderTarget::attachColorTexture(renderer::ImageFormat format, RenderTargetSamples samples, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp)
    {
        AttachmentDesc attachmentDesc = {};
        attachmentDesc._format = format;
        attachmentDesc._samples = samples;
        attachmentDesc._loadOp = loadOp;
        attachmentDesc._storeOp = storeOp;
        m_colorTextures.emplace_back(m_cmdList.createObject<Texture2D>(format, m_dimension), attachmentDesc);

        return m_colorTextures.back().first;
    }

    Texture2D* RenderTarget::attachDepthStencilTexture(renderer::ImageFormat format, RenderTargetSamples samples, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp)
    {
        ASSERT(m_depthStencilTexture.first, "attachDepthStencilTexture can create only one");
        if (!m_depthStencilTexture.first)
        {
            AttachmentDesc attachmentDesc = {};
            attachmentDesc._format = format;
            attachmentDesc._samples = samples;
            attachmentDesc._loadOp = loadOp;
            attachmentDesc._storeOp = storeOp;
            m_depthStencilTexture = std::make_pair(m_cmdList.createObject<Texture2D>(format, m_dimension), attachmentDesc);
        }
        return m_depthStencilTexture.first;
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



} //namespace v3d
