#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Copiable interface.
    */
    template<typename Type>
    class V3D_API Copiable
    {
    public:

        Copiable() = default;
        virtual ~Copiable() = default;

        virtual Type* copy() const = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
