#include "CameraFPSHelper.h"
#include "Camera.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

CameraFPSHelper::CameraFPSHelper(Camera* camera, const core::Vector3D& position) noexcept
    : CameraHelper(camera, position)
{
}

CameraFPSHelper::~CameraFPSHelper()
{
}

void CameraFPSHelper::update(f32 deltaTime)
{
    if (m_needUpdate)
    {
        CameraHelper::update();

        core::Matrix4D rotateX;
        rotateX.makeIdentity();
        rotateX.setRotation(core::Vector3D(m_transform.getRotation().x, 0.0f, 0.0f));

        core::Matrix4D rotateY;
        rotateY.makeIdentity();
        rotateY.setRotation(core::Vector3D(0.0f, m_transform.getRotation().y, 0.0f));

        core::Matrix4D rotate = rotateX * rotateY;

        core::Matrix4D look = core::buildLookAtMatrix(m_transform.getPosition(), getCamera().getTarget(), getCamera().getUpVector());
        core::Matrix4D view = rotate * look;

        getCamera().setViewMatrix(view);
        m_needUpdate = false;
        
        m_needUpdate = false;
    }
}

void CameraFPSHelper::setRotation(const core::Vector3D & rotation)
{
    m_transform.setRotation(rotation);
    m_needUpdate = true;
}

const core::Vector3D& CameraFPSHelper::getRotation() const
{
    return m_transform.getRotation();
}


void CameraFPSHelper::rotateHandlerCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event)
{
    static core::Point2D position = event->_cursorPosition;

    if (handler->isLeftMousePressed() || true)
    {
        core::Point2D positionDelta = position - event->_cursorPosition;

        core::Vector3D rotation = CameraFPSHelper::getRotation();
        rotation.x += positionDelta.y * k_rotationSpeed;
        rotation.y -= positionDelta.x * k_rotationSpeed;
        CameraFPSHelper::setRotation(rotation);
    }

    position = event->_cursorPosition;
}

void CameraFPSHelper::moveHandlerCallback(v3d::event::InputEventHandler * handler, const event::KeyboardInputEvent * event)
{
    if (handler->isKeyPressed(event::KeyCode::KeyKey_W))
    {
        //CameraFPSHelper::setPosition()
    }


    if (handler->isKeyPressed(event::KeyCode::KeyKey_S))
    {

    }

    if (handler->isKeyPressed(event::KeyCode::KeyKey_A))
    {

    }

    if (handler->isKeyPressed(event::KeyCode::KeyKey_D))
    {

    }
}

} //namespace scene
} //namespace v3d

//#include "Engine.h"
//
//
//using namespace v3d;
//using namespace core;
//using namespace scene;
//
//const f32 k_lod = 10000.0f;
//
//CFPSCamera::CFPSCamera()
//    : m_speed(0.001f)
//
//    , m_max(k_lod, k_lod, k_lod)
//    , m_min(-k_lod, -k_lod, -k_lod)
//{
//    m_keys._forward         = EKeyCode::eKeyKey_W;
//    m_keys._back            = EKeyCode::eKeyKey_S;
//    m_keys._left            = EKeyCode::eKeyKey_A;
//    m_keys._right           = EKeyCode::eKeyKey_D;
//    m_keys._acceleration    = EKeyCode::eKeyShift;
//}
//
//CFPSCamera::~CFPSCamera()
//{
//}
//
//void CFPSCamera::move(const Vector3D& direction)
//{
//    Vector3D pos = CNode::getPosition();
//    Vector3D view = CCamera::getTarget();
//    Vector3D move = CCamera::getTarget() - CNode::getPosition();
//    Vector3D strafe = core::crossProduct(move, CCamera::getUpVector());
//
//    move.normalize();
//    strafe.normalize();
//
//    pos += (move * direction.z);
//    view += (move * direction.z);
//
//    pos += (strafe * direction.x);
//    view += (strafe * direction.x);
//
//    pos.y += direction.y;
//    view.y += direction.y;
//
//    if (!CFPSCamera::isPointOut(pos))
//    {
//        CNode::setPosition(pos);
//        CCamera::setTarget(view);
//    }
//}
//
//bool CFPSCamera::isPointOut(const Vector3D& point)
//{
//    if (point.x <= m_min.x || point.y <= m_min.y || point.z <= m_min.z ||
//        point.x >= m_max.x || point.y >= m_max.y || point.z >= m_max.z)
//    {
//        return true;
//    }
//
//    return false;
//}
//
//void CFPSCamera::rotateByMouse()
//{
//    Point2D position;
//    WINDOW->getCursorPosition(position);
//
//    const Dimension2D& size = WINDOW->getSize();
//    const Point2D& pos = WINDOW->getPosition();
//    Point2D middle = Point2D(pos.x + (size.width / 2), pos.y + (size.height / 2));
//    if (position == middle)
//    {
//        return;
//    }
//
//    WINDOW->setCursorPosition(middle);
//
//    static f32 currentRotX = 0.0f;
//    static f32 lastRotX = 0.0f;
//
//    Vector3D angle = Vector3D(f32(middle.x - position.x), f32(middle.y - position.y), 0.f);
//    angle /= 1000.0f;
//    lastRotX = -currentRotX;
//
//    Vector3D vAxis = crossProduct(Vector3D(CCamera::getTarget() - CNode::getPosition()), CCamera::getUpVector());
//    vAxis.normalize();
//
//    if (currentRotX > 1.0f)
//    {
//        currentRotX = 1.0f;
//        if (lastRotX != 1.0f)
//        {
//            CFPSCamera::rotate(1.0f - lastRotX, vAxis);
//        }
//    }
//    else if (currentRotX < -1.0f)
//    {
//        currentRotX = -1.0f;
//        if (lastRotX != -1.0f)
//        {
//            CFPSCamera::rotate(-1.0f - lastRotX, vAxis);
//        }
//    }
//    else
//    {
//        CFPSCamera::rotate(angle.y, vAxis);
//    }
//
//    CFPSCamera::rotate(angle.x, Vector3D(0.0f, 1.0f, 0.0f));
//}
//
//void CFPSCamera::rotate(f32 angle, const Vector3D& point)
//{
//    Vector3D newView;
//    Vector3D oldView;
//
//    oldView = CCamera::getTarget() - CNode::getPosition();
//
//    f32 cosTheta = cos(angle);
//    f32 sinTheta = sin(angle);
//
//    newView.x = (cosTheta + (1 - cosTheta) * point.x * point.x) * oldView.x;
//    newView.x = newView.x + ((1 - cosTheta) * point.x * point.y - point.z * sinTheta) * oldView.y;
//    newView.x = newView.x + ((1 - cosTheta) * point.x * point.z + point.y * sinTheta) * oldView.z;
//
//    newView.y = ((1 - cosTheta) * point.x * point.y + point.z * sinTheta) * oldView.x;
//    newView.y = newView.y + (cosTheta + (1 - cosTheta) * point.y * point.y) * oldView.y;
//    newView.y = newView.y + ((1 - cosTheta) * point.y * point.z - point.x * sinTheta) * oldView.z;
//
//    newView.z = ((1 - cosTheta) * point.x * point.z - point.y * sinTheta) * oldView.x;
//    newView.z = newView.z + ((1 - cosTheta) * point.y * point.z + point.x * sinTheta) * oldView.y;
//    newView.z = newView.z + (cosTheta + (1 - cosTheta) * point.z * point.z) * oldView.z;
//
//    CCamera::setTarget(CNode::getPosition() + newView);
//}
//
//void CFPSCamera::update(s32 dt)
//{
//    if (!CNode::isVisible() && !m_initialiazed)
//    {
//        return;
//    }
//
//    if (m_active)
//    {
//        f32 acceleration = 1.0f;
//        if (INPUT_EVENTS->isKeyPressed(m_keys._acceleration))
//        {
//            acceleration = 10.0f;
//        }
//
//        f32 s = m_speed * acceleration * static_cast<f32>(dt);
//        if (INPUT_EVENTS->isKeyPressed(m_keys._forward))
//        {
//            CFPSCamera::move(Vector3D(0.0f, 0.0f, s));
//        }
//        if (INPUT_EVENTS->isKeyPressed(m_keys._back))
//        {
//            CFPSCamera::move(Vector3D(0.0f, 0.0f, -s));
//        }
//        if (INPUT_EVENTS->isKeyPressed(m_keys._left))
//        {
//            CFPSCamera::move(Vector3D(-s, 0.0f, 0.0f));
//        }
//        if (INPUT_EVENTS->isKeyPressed(m_keys._right))
//        {
//            CFPSCamera::move(Vector3D(s, 0.0f, 0.0f));
//        }
//
//        CFPSCamera::rotateByMouse();
//        CCamera::update(dt);
//    }
//}
//
//void CFPSCamera::setSpeed(f32 speed)
//{
//    m_speed = speed;
//}
//
//f32 CFPSCamera::getSpeed() const
//{
//    return m_speed;
//}
//
//void CFPSCamera::setMoveKeys(const SMoveKeys& keys)
//{
//    m_keys._forward = keys._forward;
//    m_keys._back = keys._back;
//    m_keys._left = keys._left;
//    m_keys._right = keys._right;
//    m_keys._acceleration = keys._acceleration;
//}
