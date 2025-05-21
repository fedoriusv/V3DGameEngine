#include "EditorViewport.h"

#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventKeyboard.h"

#include "Renderer/Shader.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderCompiler.h"
#include "Resource/ImageFileLoader.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Scene.h"

using namespace v3d;
using namespace v3d::renderer;
using namespace v3d::platform;
using namespace v3d::utils;
using namespace v3d::event;

namespace v3d
{
    extern scene::Transform g_modelTransform;
}

EditorViewport::EditorViewport()
    : m_Device(nullptr)

    , m_Camera(new scene::CameraEditorHandler(std::make_unique<scene::Camera>()))
    , m_VewiportTarget(nullptr)
{
    resource::ResourceManager::createInstance();
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/textures/");
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/models/");
    resource::ResourceManager::getInstance()->addPath("../../../../engine/data/shaders/");


    InputEventHandler::bind([this](const MouseInputEvent* event)
        {
            if (m_CurrentViewportRect.isPointInside({ (f32)this->getAbsoluteCursorPosition()._x, (f32)this->getAbsoluteCursorPosition()._y }))
            {
                m_Camera->handleInputEventCallback(this, event);
            }
        }
    );

    InputEventHandler::bind([this](const KeyboardInputEvent* event)
        {
            if (m_CurrentViewportRect.isPointInside({ (f32)this->getAbsoluteCursorPosition()._x, (f32)this->getAbsoluteCursorPosition()._y }))
            {
                if (event->_event == event::KeyboardInputEvent::KeyboardPressDown)
                {
                    if (event->_key == event::KeyCode::KeyKey_F) //focus on selected object
                    {
                        m_Camera->setTarget(g_modelTransform.getPosition()); //TODO
                    }
                }
                m_Camera->handleInputEventCallback(this, event);
            }
        }
    );
}

EditorViewport::~EditorViewport()
{
    //TODO
}

void EditorViewport::init(renderer::Device* device, const v3d::math::Dimension2D& viewportSize)
{
    m_Device = device;
    m_CurrentViewportRect = math::Rect(0, 0, viewportSize._width, viewportSize._height);
    m_Camera->setPerspective(m_VewportParams._fov, viewportSize, m_VewportParams._near, m_VewportParams._far);
    m_Camera->setMoveSpeed(2.f);
    m_Camera->setRotationSpeed(50.0f);
    m_Camera->setTarget({ 0.f, 0.f, 0.f });
    m_Camera->setPosition({ 0.f, 0.25f, -1.f });

    recreateViewport(viewportSize);
    m_drawState = Scene::loadCube(device, m_VewiportTarget->getRenderPassDesc());
}

void EditorViewport::recreateViewport(const v3d::math::Dimension2D& viewportSize)
{
    if (m_VewiportTarget)
    {
        renderer::Texture2D* colorAttachment = m_VewiportTarget->getColorTexture<renderer::Texture2D>(0);
        delete colorAttachment;

        renderer::Texture2D* depthAttachment = m_VewiportTarget->getDepthStencilTexture< renderer::Texture2D>();
        delete depthAttachment;

        delete m_VewiportTarget;
        m_VewiportTarget = nullptr;
    }


    m_VewiportTarget = new renderer::RenderTargetState(m_Device, viewportSize);

    renderer::Texture2D* colorAttachment = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Sampled | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_R8G8B8A8_UNorm, viewportSize, renderer::TextureSamples::TextureSamples_x1, "ViewportColorAttachment");
    renderer::Texture2D* depthAttachment = new renderer::Texture2D(m_Device, renderer::TextureUsage::TextureUsage_Attachment | renderer::TextureUsage::TextureUsage_Write,
        renderer::Format::Format_D32_SFloat_S8_UInt, viewportSize, renderer::TextureSamples::TextureSamples_x1, "ViewportDepthAttachment");

    m_VewiportTarget->setColorTexture(0, colorAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, color::Color(0.f, 0.f, 0.f, 1.f)
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_ColorAttachment
        }
    );

    m_VewiportTarget->setDepthStencilTexture(depthAttachment,
        {
            renderer::RenderTargetLoadOp::LoadOp_Clear, renderer::RenderTargetStoreOp::StoreOp_Store, 0.0f
        },
        {
            renderer::RenderTargetLoadOp::LoadOp_DontCare, renderer::RenderTargetStoreOp::StoreOp_DontCare, 0
        },
        {
            renderer::TransitionOp::TransitionOp_Undefined, renderer::TransitionOp::TransitionOp_DepthStencilAttachment
        }
    );
}

void EditorViewport::cleanup()
{
    //TODO
}

void EditorViewport::update(f32 dt)
{
    m_Camera->update(dt);
}

void EditorViewport::render(v3d::renderer::CmdListRender* cmdList)
{
    cmdList->beginRenderTarget(*m_VewiportTarget);
    cmdList->setViewport(math::Rect(0, 0, m_VewiportTarget->getRenderArea()._width, m_VewiportTarget->getRenderArea()._height));
    cmdList->setScissor(math::Rect(0, 0, m_VewiportTarget->getRenderArea()._width, m_VewiportTarget->getRenderArea()._height));

    Scene::drawCube(m_Device, cmdList, m_drawState, m_Camera);

    cmdList->endRenderTarget();
}

void EditorViewport::onChanged(const v3d::math::Rect& viewport)
{
    if (viewport != m_CurrentViewportRect)
    {
        if (m_CurrentViewportRect.getWidth() != viewport.getWidth() || m_CurrentViewportRect.getHeight() != viewport.getHeight())
        {
            recreateViewport({ (u32)viewport.getWidth(), (u32)viewport.getHeight() });
        }

        m_CurrentViewportRect = viewport;
    }
}

void EditorViewport::onChanged(const math::Matrix4D& view)
{
    m_Camera->setViewMatrix(view);
    m_Camera->setTarget(m_Camera->getPosition() + m_Camera->getCamera().getForwardVector() * 2.f);
    m_Camera->update(0.f);
}

const renderer::Texture2D* EditorViewport::getOutputTexture() const
{
    return m_VewiportTarget->getColorTexture<renderer::Texture2D>(0);
}

const math::Rect& EditorViewport::getViewportArea() const
{
    return m_CurrentViewportRect;
}

scene::Camera* EditorViewport::getCamera()
{
    return &m_Camera->getCamera();
}
