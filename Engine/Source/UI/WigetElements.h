#pragma once

#include "Common.h"
#include "Wiget.h"

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
    * @brief WigetText class
    */
    class WigetText final : public WigetBase<WigetText>
    {
    public:

        explicit WigetText(const std::string& text) noexcept;
        WigetText(const WigetText&) noexcept;
        WigetText(WigetText&&) noexcept;
        ~WigetText();

        const std::string& getText() const;

        WigetText& setText(const std::string& text);
        WigetText& setColor(const color::ColorRGBAF& color);

        struct StateText : StateBase
        {
            std::string         _text;
            color::ColorRGBAF   _color;
        };

    private:

        using WigetType = WigetText;
        using StateType = StateText;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        math::Vector2D calculateSize(WigetHandler* handler, Wiget* parent, Wiget* layout) final;
        Wiget* copy() const final;
    };

    inline const std::string& WigetText::getText() const
    {
        return Wiget::cast_data<StateType>(m_data)._text;
    }

    inline WigetText& WigetText::setText(const std::string& text)
    {
        Wiget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WigetText& WigetText::setColor(const color::ColorRGBAF& color)
    {
        Wiget::cast_data<StateType>(m_data)._color = color;
        Wiget::cast_data<StateType>(m_data)._stateMask |= Wiget::State::StateMask::Color;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetButton class
    */
    class WigetButton final : public WigetBase<WigetButton>
    {
    public:

        explicit WigetButton(const std::string& text, const math::Dimension2D& size = {0, 0}) noexcept;
        WigetButton(const WigetButton&) noexcept;
        ~WigetButton();

        const std::string& getText() const;
        const math::Dimension2D getSize() const;

        WigetButton& setText(const std::string& text);
        WigetButton& setColor(const color::ColorRGBAF& color);
        WigetButton& setColorHovered(const color::ColorRGBAF& color);
        WigetButton& setColorActive(const color::ColorRGBAF& color);
        WigetButton& setSize(const math::Dimension2D& size);

        WigetButton& setOnClickedEvent(const OnWigetEvent& event);
        WigetButton& setOnHoveredEvent(const OnWigetEvent& event);

        struct StateButton : StateBase
        {
            std::string         _text;
            color::ColorRGBAF   _color;
            color::ColorRGBAF   _colorHovered;
            color::ColorRGBAF   _colorActive;
            math::Dimension2D   _size;
            OnWigetEvent        _onClickedEvent;
            OnWigetEvent        _onPressedEvent;
            OnWigetEvent        _onReleasedEvent;
            OnWigetEvent        _onHoveredEvent;
            OnWigetEvent        _onUnhoveredEvent;
        };

    private:

        using WigetType = WigetButton;
        using StateType = StateButton;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        math::Vector2D calculateSize(WigetHandler* handler, Wiget* parent, Wiget* layout) final;
        Wiget* copy() const final;
    };

    inline const std::string& WigetButton::getText() const
    {
        return Wiget::cast_data<StateType>(m_data)._text;
    }

    inline const math::Dimension2D WigetButton::getSize() const
    {
        return Wiget::cast_data<StateType>(m_data)._size;
    }

    inline WigetButton& WigetButton::setText(const std::string& text)
    {
        Wiget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WigetButton& WigetButton::setColor(const color::ColorRGBAF& color)
    {
        Wiget::cast_data<StateType>(m_data)._color = color;
        Wiget::cast_data<StateType>(m_data)._stateMask |= Wiget::State::StateMask::Color;
        return *this;
    }

    inline WigetButton& WigetButton::setColorHovered(const color::ColorRGBAF& color)
    {
        Wiget::cast_data<StateType>(m_data)._colorHovered = color;
        Wiget::cast_data<StateType>(m_data)._stateMask |= Wiget::State::StateMask::HoveredColor;
        return *this;
    }

    inline WigetButton& WigetButton::setColorActive(const color::ColorRGBAF& color)
    {
        Wiget::cast_data<StateType>(m_data)._colorActive = color;
        Wiget::cast_data<StateType>(m_data)._stateMask |= Wiget::State::StateMask::ClickedColor;
        return *this;
    }

    inline WigetButton& WigetButton::setSize(const math::Dimension2D& size)
    {
        Wiget::cast_data<StateType>(m_data)._size = size;
        return *this;
    }

    inline WigetButton& WigetButton::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<StateType>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline WigetButton& WigetButton::setOnHoveredEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<StateType>(m_data)._onHoveredEvent = event;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetImage class
    */
    class WigetImage final : public WigetBase<WigetImage>
    {
    public:

        WigetImage(const math::Dimension2D& size, const math::RectF32& uv = { 0.0f, 0.0f, 1.0f, 1.0f }) noexcept;
        WigetImage(const WigetImage&) noexcept;
        ~WigetImage();

        WigetImage& setTexture(const renderer::Texture2D* texture);
        WigetImage& setSize(const math::Dimension2D& size);
        WigetImage& setUVs(const math::RectF32& uv);

        WigetImage& setOnDrawRectChanged(const OnWigetEventRect32Param& event);

        struct StateImage : StateBase
        {
            const renderer::Texture2D*  _texture;
            math::Dimension2D           _size;
            math::RectF32               _uv;
            OnWigetEvent                _onClickedEvent;
            OnWigetEvent                _onPressedEvent;
            OnWigetEvent                _onReleasedEvent;
            OnWigetEvent                _onHoveredEvent;
            OnWigetEvent                _onUnhoveredEvent;
            OnWigetEventRect32Param     _onDrawRectChanged;

            math::Rect32                _drawRectState;
        };

    private:

        using WigetType = WigetImage;
        using StateType = StateImage;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline WigetImage& WigetImage::setTexture(const renderer::Texture2D* texture)
    {
        Wiget::cast_data<StateType>(m_data)._texture = texture;
        return *this;
    }

    inline WigetImage& WigetImage::setSize(const math::Dimension2D& size)
    {
        Wiget::cast_data<StateType>(m_data)._size = size;
        return *this;
    }

    inline WigetImage& WigetImage::setUVs(const math::RectF32& uv)
    {
        Wiget::cast_data<StateType>(m_data)._uv = uv;
        return *this;
    }

    inline WigetImage& WigetImage::setOnDrawRectChanged(const OnWigetEventRect32Param& event)
    {
        cast_data<StateType>(m_data)._onDrawRectChanged = event;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetCheckBox class
    */
    class WigetCheckBox final : public WigetBase<WigetCheckBox>
    {
    public:

        explicit WigetCheckBox(const std::string& text, bool value) noexcept;
        WigetCheckBox(const WigetCheckBox&) noexcept;
        ~WigetCheckBox();

        const std::string& getText() const;
        bool getValue() const;

        WigetCheckBox& setText(const std::string& text);
        WigetCheckBox& setValue(bool value);

        WigetCheckBox& setOnChangedValueEvent(const OnWigetEventBoolParam& event);

        struct StateCheckBox : StateBase
        {
            std::string             _text;
            OnWigetEventBoolParam   _onChangedValueEvent;
            bool                    _value;
        };

    private:

        using WigetType = WigetCheckBox;
        using StateType = StateCheckBox;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline const std::string& WigetCheckBox::getText() const
    {
        return Wiget::cast_data<StateType>(m_data)._text;
    }

    inline bool WigetCheckBox::getValue() const
    {
        return Wiget::cast_data<StateType>(m_data)._value;
    }

    inline WigetCheckBox& WigetCheckBox::setText(const std::string& text)
    {
        Wiget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WigetCheckBox& WigetCheckBox::setValue(bool value)
    {
        Wiget::cast_data<StateType>(m_data)._value = value;
        return *this;
    }

    inline WigetCheckBox& WigetCheckBox::setOnChangedValueEvent(const OnWigetEventBoolParam& event)
    {
        Wiget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    //TODO
    class WigetInputField final : public WigetBase<WigetInputField>
    {
    public:

        explicit WigetInputField() noexcept;
        WigetInputField(const WigetInputField&) noexcept;
        ~WigetInputField();

        struct StateInputField : StateBase
        {
        };

    private:

        using WigetType = WigetInputField;
        using StateType = StateInputField;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

        //TODO
    class WigetInputSlider final : public WigetBase<WigetInputSlider>
    {
    public:

        explicit WigetInputSlider() noexcept;
        WigetInputSlider(const WigetInputSlider&) noexcept;
        ~WigetInputSlider();

        struct StateInputSlider : StateBase
        {
        };

    private:

        using WigetType = WigetInputSlider;
        using StateType = StateInputSlider;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace ui
} // namespace v3d