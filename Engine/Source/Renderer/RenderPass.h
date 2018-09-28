#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    class RenderPass
    {
    public:

        RenderPass() {};
        virtual ~RenderPass() {};

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
