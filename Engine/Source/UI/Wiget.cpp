#include "Wiget.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

static u64 s_uidGenerator = 0;

Wiget::Wiget(Wiget::State* state) noexcept
    : m_data(state)
{
    ++s_uidGenerator;
    cast_data<StateBase>(m_data)._uid = s_uidGenerator;
}

Wiget::Wiget(const Wiget& other) noexcept
    : m_data(nullptr) //Allocate and copy in the child constructor
{
}

Wiget::Wiget(Wiget&& other) noexcept
{
}

Wiget::~Wiget()
{
    ASSERT(m_data == nullptr, "must be deleted");
}

bool Wiget::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    StateBase& state = cast_data<StateBase>(m_data);

    if (!state._isCreated)
    {
        if (state._onCreated)
        {
            std::invoke(state._onCreated, this);
        }
        state._isCreated = true;
    }

    if (state._stateMask & State::StateMask::Active)
    {
        if (state._onActiveChanged)
        {
            std::invoke(state._onActiveChanged, this);
        }
        state._stateMask &= ~State::StateMask::Active;
    }

    if (state._stateMask & State::StateMask::Visible)
    {
        if (state._onVisibleChanged)
        {
            std::invoke(cast_data<StateBase>(m_data)._onVisibleChanged, this);
        }
        state._stateMask &= ~State::StateMask::Visible;
    }

    if (state._onUpdate)
    {
        std::invoke(state._onUpdate, this, dt);
    }

    return true;
}

math::TVector2D<f32> Wiget::calculateSize(WigetHandler* handler, Wiget* parent, Wiget* layout)
{
    return { 0.0f, 0.0f };
}

void Wiget::handleNotify(const utils::Reporter<WigetReport>* reporter, const WigetReport& data)
{
}

} // namespace ui
} // namespace v3d