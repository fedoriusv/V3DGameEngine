#pragma once

#include "Render.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"

namespace v3d
{
namespace renderer
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Descriptor struct
    */
    struct Descriptor
    {
        enum Type : u32
        {
            Descriptor_ConstantBuffer = 1,
            Descriptor_TextureSampled,
            Descriptor_RWTexture,
            Descriptor_RWBuffer,
            Descriptor_Sampler,
        };

        enum Frequency : u32
        {
            Frequency_Dynamic = 0,
            Frequency_Static,
        };

        struct ConstantBuffer
        {
            void*   _data = nullptr;
            u32     _offset = 0;
            u32     _size = 0;
        };

        Descriptor(const ConstantBuffer& CBO, u32 slot) noexcept
            : _type(Type::Descriptor_ConstantBuffer)
            , _frequency(Frequency::Frequency_Dynamic)
            , _slot(slot)
            , _resource(CBO)
        {
        }

        Descriptor(const TextureView& view, u32 slot) noexcept
            : _type(Type::Descriptor_TextureSampled)
            , _frequency(Frequency::Frequency_Dynamic)
            , _slot(slot)
            , _resource(view)
        {
        }

        Descriptor(Texture* UAV, u32 slot) noexcept
            : _type(Type::Descriptor_RWTexture)
            , _frequency(Frequency::Frequency_Dynamic)
            , _slot(slot)
            , _resource(UAV)
        {
        }

        Descriptor(Buffer* UAV, u32 slot) noexcept
            : _type(Type::Descriptor_RWBuffer)
            , _frequency(Frequency::Frequency_Dynamic)
            , _slot(slot)
            , _resource(UAV)
        {
        }

        Descriptor(SamplerState* sampler, u32 slot) noexcept
            : _type(Type::Descriptor_Sampler)
            , _frequency(Frequency::Frequency_Dynamic)
            , _slot(slot)
            , _resource(sampler)
        {
        }

        Descriptor(Type type) noexcept
            : _type(type)
            , _frequency(Frequency::Frequency_Dynamic)
            , _slot(0)
            , _resource()
        {
        }

        Type        _type       : 16;
        Frequency   _frequency  : 16;
        u32         _slot;
        std::variant<std::monostate, ConstantBuffer, TextureView, Texture*, Buffer*, SamplerState*> _resource;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace renderer
} //namespace v3d