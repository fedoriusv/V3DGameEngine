#pragma once

#include "Common.h"
#include "Widget.h"

namespace v3d
{
namespace renderer
{
    class Texture2D;
} //namespace renderer
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetText class
    */
    class WidgetText final : public WidgetBase<WidgetText>
    {
    public:

        explicit WidgetText(const std::string& text) noexcept;
        WidgetText(const WidgetText&) noexcept;
        WidgetText(WidgetText&&) noexcept;
        ~WidgetText();

        const std::string& getText() const;

        WidgetText& setText(const std::string& text);
        WidgetText& setColor(const color::ColorRGBAF& color);

        TypePtr getType() const final;

        struct StateText : StateBase
        {
            std::string         _text;
            color::ColorRGBAF   _color;
        };

    private:

        using WidgetType = WidgetText;
        using StateType = StateText;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;
    };

    inline const std::string& WidgetText::getText() const
    {
        return Widget::cast_data<StateType>(m_data)._text;
    }

    inline WidgetText& WidgetText::setText(const std::string& text)
    {
        Widget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WidgetText& WidgetText::setColor(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(m_data)._color = color;
        Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::Color;
        return *this;
    }

    inline TypePtr WidgetText::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetButton class
    */
    class WidgetButton final : public WidgetBase<WidgetButton>
    {
    public:

        explicit WidgetButton(const std::string& text, const math::Dimension2D& size = {0, 0}) noexcept;
        WidgetButton(const WidgetButton&) noexcept;
        WidgetButton(WidgetButton&&) noexcept;
        ~WidgetButton();

        const std::string& getText() const;
        const math::Dimension2D getSize() const;

        WidgetButton& setText(const std::string& text);
        WidgetButton& setColor(const color::ColorRGBAF& color);
        WidgetButton& setColorHovered(const color::ColorRGBAF& color);
        WidgetButton& setColorActive(const color::ColorRGBAF& color);
        WidgetButton& setSize(const math::Dimension2D& size);

        WidgetButton& setOnClickedEvent(const OnWidgetEvent& event);
        WidgetButton& setOnHoveredEvent(const OnWidgetEvent& event);

        TypePtr getType() const final;

        struct StateButton : StateBase
        {
            std::string         _text;
            color::ColorRGBAF   _color;
            color::ColorRGBAF   _colorHovered;
            color::ColorRGBAF   _colorActive;
            math::Dimension2D   _size;
            OnWidgetEvent       _onClickedEvent;
            OnWidgetEvent       _onPressedEvent;
            OnWidgetEvent       _onReleasedEvent;
            OnWidgetEvent       _onHoveredEvent;
            OnWidgetEvent       _onUnhoveredEvent;
        };

    private:

        using WidgetType = WidgetButton;
        using StateType = StateButton;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;
    };

    inline const std::string& WidgetButton::getText() const
    {
        return Widget::cast_data<StateType>(m_data)._text;
    }

    inline const math::Dimension2D WidgetButton::getSize() const
    {
        return Widget::cast_data<StateType>(m_data)._size;
    }

    inline WidgetButton& WidgetButton::setText(const std::string& text)
    {
        Widget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WidgetButton& WidgetButton::setColor(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(m_data)._color = color;
        Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::Color;
        return *this;
    }

    inline WidgetButton& WidgetButton::setColorHovered(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(m_data)._colorHovered = color;
        Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::HoveredColor;
        return *this;
    }

    inline WidgetButton& WidgetButton::setColorActive(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(m_data)._colorActive = color;
        Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::ClickedColor;
        return *this;
    }

    inline WidgetButton& WidgetButton::setSize(const math::Dimension2D& size)
    {
        Widget::cast_data<StateType>(m_data)._size = size;
        return *this;
    }

    inline WidgetButton& WidgetButton::setOnClickedEvent(const OnWidgetEvent& event)
    {
        Widget::cast_data<StateType>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline WidgetButton& WidgetButton::setOnHoveredEvent(const OnWidgetEvent& event)
    {
        Widget::cast_data<StateType>(m_data)._onHoveredEvent = event;
        return *this;
    }

    inline TypePtr WidgetButton::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetImage class
    */
    class WidgetImage final : public WidgetBase<WidgetImage>
    {
    public:

        explicit WidgetImage(const renderer::Texture2D* texture, const math::Dimension2D& size, const math::Rect& uv = { 0.0f, 0.0f, 1.0f, 1.0f }) noexcept;
        WidgetImage(const WidgetImage&) noexcept;
        WidgetImage(WidgetImage&&) noexcept;
        ~WidgetImage();

        WidgetImage& setTexture(const renderer::Texture2D* texture);
        WidgetImage& setSize(const math::Dimension2D& size);
        WidgetImage& setUVs(const math::Rect& uv);

        WidgetImage& setOnDrawRectChanged(const OnWidgetEventRectParam& event);

        TypePtr getType() const final;

        struct StateImage : StateBase
        {
            const renderer::Texture2D*  _texture;
            math::Dimension2D           _size;
            math::Rect                  _uv;
            OnWidgetEvent               _onClickedEvent;
            OnWidgetEvent               _onPressedEvent;
            OnWidgetEvent               _onReleasedEvent;
            OnWidgetEvent               _onHoveredEvent;
            OnWidgetEvent               _onUnhoveredEvent;
            OnWidgetEventRectParam      _onDrawRectChanged;

            math::Rect                  _drawRectState;
        };

    private:

        using WidgetType = WidgetImage;
        using StateType = StateImage;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;
    };

    inline WidgetImage& WidgetImage::setTexture(const renderer::Texture2D* texture)
    {
        Widget::cast_data<StateType>(m_data)._texture = texture;
        return *this;
    }

    inline WidgetImage& WidgetImage::setSize(const math::Dimension2D& size)
    {
        Widget::cast_data<StateType>(m_data)._size = size;
        return *this;
    }

    inline WidgetImage& WidgetImage::setUVs(const math::Rect& uv)
    {
        Widget::cast_data<StateType>(m_data)._uv = uv;
        return *this;
    }

    inline WidgetImage& WidgetImage::setOnDrawRectChanged(const OnWidgetEventRectParam& event)
    {
        cast_data<StateType>(m_data)._onDrawRectChanged = event;
        return *this;
    }

    inline TypePtr WidgetImage::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetCheckBox class
    */
    class WidgetCheckBox final : public WidgetBase<WidgetCheckBox>
    {
    public:

        explicit WidgetCheckBox(const std::string& text, bool value) noexcept;
        WidgetCheckBox(const WidgetCheckBox&) noexcept;
        WidgetCheckBox(WidgetCheckBox&&) noexcept;
        ~WidgetCheckBox();

        const std::string& getText() const;
        bool getValue() const;

        WidgetCheckBox& setText(const std::string& text);
        WidgetCheckBox& setValue(bool value);

        WidgetCheckBox& setOnChangedValueEvent(const OnWidgetEventBoolParam& event);

        TypePtr getType() const final;

        struct StateCheckBox : StateBase
        {
            std::string             _text;
            OnWidgetEventBoolParam  _onChangedValueEvent;
            bool                    _value;
        };

    private:

        using WidgetType = WidgetCheckBox;
        using StateType = StateCheckBox;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline const std::string& WidgetCheckBox::getText() const
    {
        return Widget::cast_data<StateType>(m_data)._text;
    }

    inline bool WidgetCheckBox::getValue() const
    {
        return Widget::cast_data<StateType>(m_data)._value;
    }

    inline WidgetCheckBox& WidgetCheckBox::setText(const std::string& text)
    {
        Widget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WidgetCheckBox& WidgetCheckBox::setValue(bool value)
    {
        Widget::cast_data<StateType>(m_data)._value = value;
        return *this;
    }

    inline WidgetCheckBox& WidgetCheckBox::setOnChangedValueEvent(const OnWidgetEventBoolParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    inline TypePtr WidgetCheckBox::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    //TODO
    class WidgetInputSlider final : public WidgetBase<WidgetInputSlider>
    {
    public:

        explicit WidgetInputSlider() noexcept;
        WidgetInputSlider(const WidgetInputSlider&) noexcept;
        ~WidgetInputSlider();

        TypePtr getType() const final;

        struct StateInputSlider : StateBase
        {
        };

    private:

        using WidgetType = WidgetInputSlider;
        using StateType = StateInputSlider;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline TypePtr WidgetInputSlider::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    template<>
    struct TypeOf<ui::WidgetText>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetButton>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetImage>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetCheckBox>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetInputSlider>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d