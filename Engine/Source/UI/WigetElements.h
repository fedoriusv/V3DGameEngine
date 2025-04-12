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
    * @brief WigetButton class
    */
    class WigetButton : public WigetBase<WigetButton>
    {
    public:

        explicit WigetButton(const std::string& text) noexcept;
        WigetButton(const WigetButton&) noexcept;
        ~WigetButton();

        const std::string& getText() const;

        WigetButton& setText(const std::string& text);
        WigetButton& setColor(const color::ColorRGBAF& color);
        WigetButton& setColorHovered(const color::ColorRGBAF& color);
        WigetButton& setColorActive(const color::ColorRGBAF& color);

        WigetButton& setOnClickedEvent(const OnWigetEvent& event);
        WigetButton& setOnHoveredEvent(const OnWigetEvent& event);

        struct ContextButton : ContextBase
        {
            std::string         _text;
            color::ColorRGBAF   _color;
            color::ColorRGBAF   _colorHovered;
            color::ColorRGBAF   _colorActive;
            OnWigetEvent        _onClickedEvent;
            OnWigetEvent        _onPressedEvent;
            OnWigetEvent        _onReleasedEvent;
            OnWigetEvent        _onHoveredEvent;
            OnWigetEvent        _onUnhoveredEvent;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    inline const std::string& WigetButton::getText() const
    {
        return Wiget::cast_data<ContextButton>(m_data)._text;
    }

    inline WigetButton& WigetButton::setText(const std::string& text)
    {
        Wiget::cast_data<ContextButton>(m_data)._text = text;
        return *this;
    }

    inline WigetButton& WigetButton::setColor(const color::ColorRGBAF& color)
    {
        Wiget::cast_data<ContextButton>(m_data)._color = color;
        Wiget::cast_data<ContextButton>(m_data)._stateMask |= 0x04;
        return *this;
    }

    inline WigetButton& WigetButton::setColorHovered(const color::ColorRGBAF& color)
    {
        Wiget::cast_data<ContextButton>(m_data)._colorHovered = color;
        Wiget::cast_data<ContextButton>(m_data)._stateMask |= 0x08;
        return *this;
    }

    inline WigetButton& WigetButton::setColorActive(const color::ColorRGBAF& color)
    {
        Wiget::cast_data<ContextButton>(m_data)._colorActive = color;
        Wiget::cast_data<ContextButton>(m_data)._stateMask |= 0x10;
        return *this;
    }

    inline WigetButton& WigetButton::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<ContextButton>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline WigetButton& WigetButton::setOnHoveredEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<ContextButton>(m_data)._onHoveredEvent = event;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetImage class
    */
    class WigetImage : public WigetBase<WigetImage>
    {
    public:

        WigetImage(const math::Dimension2D& size, const math::RectF32& uv = { 0.0f, 0.0f, 1.0f, 1.0f }) noexcept;
        WigetImage(const WigetImage&) noexcept;
        ~WigetImage();

        WigetImage& setTexture(const renderer::Texture2D* texture);
        WigetImage& setSize(const math::Dimension2D& size);
        WigetImage& setUVs(const math::RectF32& uv);

        struct ContextImage : ContextBase
        {
            OnWigetEvent                _onClickedEvent;
            OnWigetEvent                _onPressedEvent;
            OnWigetEvent                _onReleasedEvent;
            OnWigetEvent                _onHoveredEvent;
            OnWigetEvent                _onUnhoveredEvent;
            const renderer::Texture2D*  _texture;
            math::Dimension2D           _size;
            math::RectF32               _uv;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    inline WigetImage& WigetImage::setTexture(const renderer::Texture2D* texture)
    {
        Wiget::cast_data<ContextImage>(m_data)._texture = texture;
        return *this;
    }

    inline WigetImage& WigetImage::setSize(const math::Dimension2D& size)
    {
        Wiget::cast_data<ContextImage>(m_data)._size = size;
        return *this;
    }

    inline WigetImage& WigetImage::setUVs(const math::RectF32& uv)
    {
        Wiget::cast_data<ContextImage>(m_data)._uv = uv;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetCheckBox class
    */
    class WigetCheckBox : public WigetBase<WigetCheckBox>
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

        struct ContextCheckBox : ContextBase
        {
            std::string             _text;
            OnWigetEventBoolParam   _onChangedValueEvent;
            bool                    _value;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    inline const std::string& WigetCheckBox::getText() const
    {
        return Wiget::cast_data<ContextCheckBox>(m_data)._text;
    }

    inline bool WigetCheckBox::getValue() const
    {
        return Wiget::cast_data<ContextCheckBox>(m_data)._value;
    }

    inline WigetCheckBox& WigetCheckBox::setText(const std::string& text)
    {
        Wiget::cast_data<ContextCheckBox>(m_data)._text = text;
        return *this;
    }

    inline WigetCheckBox& WigetCheckBox::setValue(bool value)
    {
        Wiget::cast_data<ContextCheckBox>(m_data)._value = value;
        return *this;
    }

    inline WigetCheckBox& WigetCheckBox::setOnChangedValueEvent(const OnWigetEventBoolParam& event)
    {
        Wiget::cast_data<ContextCheckBox>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetInputField : public WigetBase<WigetInputField>
    {
    public:

        explicit WigetInputField() noexcept;
        WigetInputField(const WigetInputField&) noexcept;
        ~WigetInputField();

        struct ContextInputField : ContextBase
        {
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetInputSlider : public WigetBase<WigetInputSlider>
    {
    public:

        explicit WigetInputSlider() noexcept;
        WigetInputSlider(const WigetInputSlider&) noexcept;
        ~WigetInputSlider();

        struct ContextInputSlider : ContextBase
        {
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace ui
} // namespace v3d