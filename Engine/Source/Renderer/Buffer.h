#pragma once

#include "Common.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    class Context;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Buffer base class. Render side
    */
    class Buffer : public utils::Observable
    {
    public:

        Buffer() {};
        virtual ~Buffer() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
