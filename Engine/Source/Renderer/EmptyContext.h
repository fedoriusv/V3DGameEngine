#pragma once

#include "Context.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class EmptyContext final : public Context
    {
    public:

        EmptyContext();
        ~EmptyContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;

    private:

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
