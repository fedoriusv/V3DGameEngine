#include "Wiget.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

Wiget::Wiget(Wiget::Context* context) noexcept
    : m_data(context)
{
}

Wiget::Wiget(const Wiget& other) noexcept
    : m_data(nullptr) //allocate and copy on child constructor
{
}

} // namespace ui
} // namespace v3d