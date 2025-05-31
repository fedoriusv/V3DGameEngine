#pragma once

#include "Common.h"
#include "Wiget.h"
#include "Scene/Transform.h"

namespace v3d
{
namespace scene
{
    class Camera;
} //namespace scene
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetGizmo class
    */
    class WigetGizmo final : public WigetBase<WigetGizmo>
    {
    public:

        enum class Operation
        {
            Translate,
            Rotate,
            Scale
        };

        enum class Mode
        {
            Local,
            Word
        };

        explicit WigetGizmo(const scene::Camera* camera) noexcept;
        WigetGizmo(const WigetGizmo&) noexcept;
        WigetGizmo(WigetGizmo&&) noexcept;
        ~WigetGizmo();

        WigetGizmo& setCamera(const scene::Camera* camera);
        WigetGizmo& setTransform(const scene::Transform& transform);
        WigetGizmo& setOperation(Operation op);
        WigetGizmo& setMode(Mode mode);

        WigetGizmo& setOnHoveredEvent(const OnWigetEvent& event);
        WigetGizmo& setOnTransformChangedEvent(const OnWigetEventTransformParam& event);

        TypePtr getType() const final;

        struct StateGizmo : StateBase
        {
            OnWigetEvent                _onHoveredEvent;
            OnWigetEventTransformParam  _onTransformChangedEvent;
            Operation                   _operation  = Operation::Translate;
            Mode                        _mode       = Mode::Local;
            scene::Transform            _transform;
            const scene::Camera*        _camera     = nullptr;
        };

    private:

        using WigetType = WigetGizmo;
        using StateType = StateGizmo;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline WigetGizmo& WigetGizmo::setCamera(const scene::Camera* camera)
    {
        Wiget::cast_data<StateType>(m_data)._camera = camera;
        return *this;
    }

    inline WigetGizmo& WigetGizmo::setTransform(const scene::Transform& transform)
    {
        Wiget::cast_data<StateType>(m_data)._transform = transform;
        return *this;
    }

    inline WigetGizmo& WigetGizmo::setOperation(Operation op)
    {
        Wiget::cast_data<StateType>(m_data)._operation = op;
        return *this;
    }

    inline WigetGizmo& WigetGizmo::setMode(Mode mode)
    {
        Wiget::cast_data<StateType>(m_data)._mode = mode;
        return *this;
    }

    inline WigetGizmo& WigetGizmo::setOnTransformChangedEvent(const OnWigetEventTransformParam& event)
    {
        Wiget::cast_data<StateType>(m_data)._onTransformChangedEvent = event;
        return *this;
    }

    inline WigetGizmo& WigetGizmo::setOnHoveredEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<StateType>(m_data)._onHoveredEvent = event;
        return *this;
    }

    inline TypePtr WigetGizmo::getType() const
    {
        return typeOf<WigetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetViewManipulator : public WigetBase<WigetViewManipulator>
    {
    public:

        explicit WigetViewManipulator(const scene::Camera* camera) noexcept;
        WigetViewManipulator(const WigetViewManipulator&) noexcept;
        WigetViewManipulator(WigetViewManipulator&&) noexcept;
        ~WigetViewManipulator();

        WigetViewManipulator& setCamera(const scene::Camera* camera);

        WigetViewManipulator& setOnHoveredEvent(const OnWigetEvent& event);
        WigetViewManipulator& setOnViewChangedEvent(const OnWigetEventMatrix4x4Param& event);

        TypePtr getType() const final;

        struct StateViewManipulator : StateBase
        {
            OnWigetEvent                _onHoveredEvent;
            OnWigetEventMatrix4x4Param  _onViewChangedEvent;
            const scene::Camera*        _camera = nullptr;
            u32                         _size = 128;
        };

    private:

        using WigetType = WigetViewManipulator;
        using StateType = StateViewManipulator;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline WigetViewManipulator& WigetViewManipulator::setCamera(const scene::Camera* camera)
    {
        Wiget::cast_data<StateType>(m_data)._camera = camera;
        return *this;
    }

    inline WigetViewManipulator& WigetViewManipulator::setOnViewChangedEvent(const OnWigetEventMatrix4x4Param& event)
    {
        Wiget::cast_data<StateType>(m_data)._onViewChangedEvent = event;
        return *this;
    }

    inline WigetViewManipulator& WigetViewManipulator::setOnHoveredEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<StateType>(m_data)._onHoveredEvent = event;
        return *this;
    }

    inline TypePtr WigetViewManipulator::getType() const
    {
        return typeOf<WigetType>();
    }

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WigetGizmo>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WigetViewManipulator>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d