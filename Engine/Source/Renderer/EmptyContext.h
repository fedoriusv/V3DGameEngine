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

        void setViewport(const core::Rect32& viewport) override;

        //test
        void clearColor(const core::Vector4D& color) override;

        Textue* createTexture() const override;

    private:

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
