#pragma once

#include "Common.h"
#include "Widget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetTable class
    */
    class WidgetTable final : public WidgetBase<WidgetTable>
    {
    public:

        explicit WidgetTable() noexcept;
        WidgetTable(const WidgetTable&) noexcept;
        WidgetTable(WidgetTable&&) noexcept;
        ~WidgetTable();

        TypePtr getType() const final;

        struct StateTable : StateBase
        {
        };

    private:

        using WidgetType = WidgetTable;
        using StateType = StateTable;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;
    };

    inline TypePtr WidgetTable::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetTable>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d