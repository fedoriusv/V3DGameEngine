#pragma once

#include "Common.h"
#include "Transform.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Renderable class property
    */
    class Renderable
    {
    public:

        virtual ~Renderable() = default;

        virtual TypePtr getType() const = 0;

    protected:

        Renderable() = default;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d