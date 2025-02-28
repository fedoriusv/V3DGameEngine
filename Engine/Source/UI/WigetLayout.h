#pragma once

#include "Common.h"
#include "Wiget.h"

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

    class WigetHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class WigetLayout
    {
    public:

        explicit WigetLayout(WigetHandler* handler, const std::string& title = "") noexcept;
        virtual ~WigetLayout();

        const math::Dimension2D& getSize() const;
        const math::Point2D& getPosition() const;
        const std::string& getTitle() const;

        void setSize(const math::Dimension2D& size);
        void setPosition(const math::Point2D& position);
        void setTitle(const std::string& title);

        template<class TWiget>
        TWiget* addWiget(TWiget* wiget)
        {
            static_assert(std::is_base_of<Wiget, TWiget>(), "wrong type");
            m_wigets.push_back(wiget);

            return wiget;
        }

    protected:

        friend WigetHandler;

        virtual void update(f32 dt);
        virtual void render(renderer::CmdListRender* cmdList);

        WigetHandler* const m_handler;
        std::vector<Wiget*> m_wigets;

        math::Dimension2D   m_size;
        math::Point2D       m_position;
        std::string         m_title;

    };

    inline const math::Dimension2D& WigetLayout::getSize() const
    {
        return m_size;
    }

    inline const math::Point2D& WigetLayout::getPosition() const
    {
        return m_position;
    }

    inline const std::string& WigetLayout::getTitle() const
    {
        return m_title;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui
} //namespace v3d