#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ResourceID class. Use for hashing instead pointer addresses
    */
    template<class TResource, typename Type>
    class V3D_API ResourceID
    {
    public:

        using ValueType = Type;

        Type ID() const
        {
            return m_ID;
        }

    protected:

        ResourceID() noexcept
            : m_ID(0)
        {
            static_assert(std::is_integral<Type>::value, "wrong type");
            m_ID = s_IDGenerator.fetch_add(1, std::memory_order_relaxed);
            ASSERT(m_ID != std::numeric_limits<Type>::max(), "range out");
        }

        ~ResourceID() = default;

    private:

        Type m_ID;
        static std::atomic<Type> s_IDGenerator;
    };

    template<class TResource, typename Type>
    std::atomic<Type> ResourceID<TResource, Type>::s_IDGenerator = 1;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
