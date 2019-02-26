#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace scene
{
    class Scene
    {
    public:
        Scene() noexcept;
        ~Scene();

        void onLoad(v3d::renderer::CommandList & cmd);

        void onUpdate();
        void onRender(v3d::renderer::CommandList& cmd);

    private:

    };

} //namespace scene
} //namespace v3d
