#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{
    class Device;
    class CmdListRender;
    class RenderTargetState;

} // namespace render
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Wiget;
    class WigetHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class WigetLayout
    {
    public:

        enum FontSize
        {
            SmallFont,
            MediumFont,
            LargeFont,

            FontSize_Count
        };

        explicit WigetLayout() noexcept;
        virtual ~WigetLayout();

        template<class TWiget>
        TWiget& addWiget(const TWiget& wiget)
        {
            TWiget* obj = V3D_NEW(TWiget, memory::MemoryLabel::MemoryUI)(wiget);
            m_wigets.push_back(obj);
            return *obj;
        }

    public:

        virtual void update(WigetHandler* handler, Wiget* parent, f32 dt);

        std::vector<Wiget*> m_wigets;
        //aligment and paddings
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d