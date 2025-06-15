#pragma once

#include "Common.h"
#include "Scene/Component.h"
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
} //namespace renderer
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    class Mesh;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Model class
    */
    class Model : public Object
    {
    public:

        Model() noexcept;
        ~Model();

        struct Geometry
        {
            std::vector<Mesh*> _LODs;
        };

        std::vector<Geometry> m_geometry;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ModelHelper class
    */
    class ModelHelper
    {
    public:

        [[nodisard]] static Model* createModel(renderer::Device* device, renderer::CmdListRender* cmdList, resource::ModelResource* resource);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
