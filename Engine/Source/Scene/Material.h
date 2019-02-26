#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"


namespace v3d
{
namespace scene
{
    /**
    * Material class. Component
    */
    class Material : public Component, public resource::Resource
    {
    public:

        struct Diffuse
        {
            Diffuse();

            core::Vector4D _color;
            std::string    _texture;

            void operator >> (stream::Stream * stream);
            void operator << (const stream::Stream * stream);
        };

        struct Ambient
        {
            Ambient();

            core::Vector4D _color;
            std::string    _texture;

            void operator >> (stream::Stream * stream);
            void operator << (const stream::Stream * stream);
        };

        Material() noexcept;
        ~Material();

        void init(stream::Stream* stream) override;
        bool load() override;

    private:

        std::string m_name;

        Diffuse     m_diffuse;
        Ambient     m_ambient;

    };

} //namespace scene
} //namespace v3d
