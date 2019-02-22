#pragma once

#include "ResourceLoader.h"

namespace v3d
{
namespace renderer
{
    class Context;
} //namespace renderer

namespace scene
{
    class Model;
} //namespace scene

namespace resource
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ModelFileLoader class. Loader from file
    */
    class ModelFileLoader : public ResourceLoader<scene::Model*>
    {
    public:

        ModelFileLoader() noexcept;
        ~ModelFileLoader();

        scene::Model* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
