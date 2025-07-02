#pragma once

#include "Common.h"
#include "Scene/Component.h"
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


    class ModelHandler
    {
    public:

        ModelHandler() = default;
        ~ModelHandler() = default;

        static ModelHandler* createModelHandler();

        void visibilityTest(scene::SceneData& scene);

        void drawStaticGeometry();
        void drawAnimatedGeometry();

    private:

        renderer::Device* m_device;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
