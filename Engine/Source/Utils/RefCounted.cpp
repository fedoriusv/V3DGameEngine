#include "RefCounted.h"

namespace v3d
{
namespace utils
{

 RefCounted::RefCounted(s32 refCount)
    : m_refCount(refCount)
{
}

s32 RefCounted::getCount() const
{
    return m_refCount;
}

void RefCounted::grab() const
{
    ++m_refCount;
}

void RefCounted::drop() const
{
    --m_refCount;
    if (m_refCount == 0)
    {
        delete this;
    }
}

} //namespace utils
} //namespace v3d
