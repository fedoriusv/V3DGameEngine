#pragma once

#include "Common.h"
#include "Widget.h"
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
    * @brief WidgetGizmo class
    */
    class WidgetGizmo final : public WidgetBase<WidgetGizmo>
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
            World
        };

        explicit WidgetGizmo(const scene::Camera* camera) noexcept;
        WidgetGizmo(const WidgetGizmo&) noexcept;
        WidgetGizmo(WidgetGizmo&&) noexcept;
        ~WidgetGizmo();

        WidgetGizmo& setCamera(const scene::Camera* camera);
        WidgetGizmo& setTransform(const math::Matrix4D& transform);
        WidgetGizmo& setOperation(Operation op);
        WidgetGizmo& setMode(Mode mode);

        WidgetGizmo& setOnHoveredEvent(const OnWidgetEvent& event);
        WidgetGizmo& setOnTransformChangedEvent(const OnWidgetEventMatrix4x4Param& event);

        TypePtr getType() const final;

        struct StateGizmo : StateBase
        {
            OnWidgetEvent               _onHoveredEvent;
            OnWidgetEventMatrix4x4Param _onTransformChangedEvent;
            Operation                   _operation  = Operation::Translate;
            Mode                        _mode       = Mode::World;
            math::Matrix4D              _transform;
            const scene::Camera*        _camera     = nullptr;
        };

    private:

        using WidgetType = WidgetGizmo;
        using StateType = StateGizmo;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline WidgetGizmo& WidgetGizmo::setCamera(const scene::Camera* camera)
    {
        Widget::cast_data<StateType>(m_data)._camera = camera;
        return *this;
    }

    inline WidgetGizmo& WidgetGizmo::setTransform(const math::Matrix4D& transform)
    {
        Widget::cast_data<StateType>(m_data)._transform = transform;
        return *this;
    }

    inline WidgetGizmo& WidgetGizmo::setOperation(Operation op)
    {
        Widget::cast_data<StateType>(m_data)._operation = op;
        return *this;
    }

    inline WidgetGizmo& WidgetGizmo::setMode(Mode mode)
    {
        Widget::cast_data<StateType>(m_data)._mode = mode;
        return *this;
    }

    inline WidgetGizmo& WidgetGizmo::setOnTransformChangedEvent(const OnWidgetEventMatrix4x4Param& event)
    {
        Widget::cast_data<StateType>(m_data)._onTransformChangedEvent = event;
        return *this;
    }

    inline WidgetGizmo& WidgetGizmo::setOnHoveredEvent(const OnWidgetEvent& event)
    {
        Widget::cast_data<StateType>(m_data)._onHoveredEvent = event;
        return *this;
    }

    inline TypePtr WidgetGizmo::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WidgetViewManipulator : public WidgetBase<WidgetViewManipulator>
    {
    public:

        explicit WidgetViewManipulator(const scene::Camera* camera) noexcept;
        WidgetViewManipulator(const WidgetViewManipulator&) noexcept;
        WidgetViewManipulator(WidgetViewManipulator&&) noexcept;
        ~WidgetViewManipulator();

        WidgetViewManipulator& setCamera(const scene::Camera* camera);

        WidgetViewManipulator& setOnHoveredEvent(const OnWidgetEvent& event);
        WidgetViewManipulator& setOnViewChangedEvent(const OnWidgetEventMatrix4x4Param& event);

        TypePtr getType() const final;

        struct StateViewManipulator : StateBase
        {
            OnWidgetEvent                _onHoveredEvent;
            OnWidgetEventMatrix4x4Param  _onViewChangedEvent;
            const scene::Camera*        _camera = nullptr;
            u32                         _size = 128;
        };

    private:

        using WidgetType = WidgetViewManipulator;
        using StateType = StateViewManipulator;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline WidgetViewManipulator& WidgetViewManipulator::setCamera(const scene::Camera* camera)
    {
        Widget::cast_data<StateType>(m_data)._camera = camera;
        return *this;
    }

    inline WidgetViewManipulator& WidgetViewManipulator::setOnViewChangedEvent(const OnWidgetEventMatrix4x4Param& event)
    {
        Widget::cast_data<StateType>(m_data)._onViewChangedEvent = event;
        return *this;
    }

    inline WidgetViewManipulator& WidgetViewManipulator::setOnHoveredEvent(const OnWidgetEvent& event)
    {
        Widget::cast_data<StateType>(m_data)._onHoveredEvent = event;
        return *this;
    }

    inline TypePtr WidgetViewManipulator::getType() const
    {
        return typeOf<WidgetType>();
    }

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetGizmo>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WidgetViewManipulator>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d