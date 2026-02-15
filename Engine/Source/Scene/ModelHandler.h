#pragma once

#include "Common.h"
#include "Scene/Scene.h"
#include "Resource/Resource.h"

namespace v3d
{
namespace resource
{
    class ModelResource;
} //namespace resource
namespace renderer
{
    class Device;
    class CmdListRender;
    class Texture;
} //namespace renderer
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ModelHandler
    {
    public:

        ModelHandler() = default;
        ~ModelHandler() = default;

    private:

        renderer::Device* m_device;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
