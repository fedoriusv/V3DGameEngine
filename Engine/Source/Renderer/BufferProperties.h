#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    class Buffer;
    class VertexStreamBuffer;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxVertexInputAttributes = 16;
    constexpr u32 k_maxVertexInputBindings = 16;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VertexInputAttributeDesc struct
    */
    struct VertexInputAttribDescription
    {
        enum InputRate : u32
        {
            InputRate_Vertex = 0,
            InputRate_Instance = 1
        };

        struct InputBinding
        {
            InputBinding()
            {
                memset(this, 0, sizeof(InputBinding));
            }
            
            InputBinding(u32 index, InputRate rate, u32 stride)
                : _index(index)
                , _rate(rate)
                , _stride(stride)
            {
            }

            u32       _index;
            InputRate _rate;
            u32       _stride;
        };

        struct InputAttribute
        {
            InputAttribute()
            {
                memset(this, 0, sizeof(InputAttribute));
            }

            InputAttribute(u32 binding, u32 stream, Format format, u32 offset)
                : _bindingId(binding)
                , _streamId(stream)
                , _format(format)
                , _offest(offset)
            {
            }

            u32          _bindingId;
            u32          _streamId;
            Format       _format;
            u32          _offest;
        };

        VertexInputAttribDescription() noexcept
        {
        }

        VertexInputAttribDescription(std::vector<InputBinding> inputBindings, std::vector<VertexInputAttribDescription::InputAttribute> inputAttributes) noexcept
        {
            u32 index = 0;
            for (auto& binding : inputBindings)
            {
                _inputBindings[index] = binding;
                index++;
            }
            _countInputBindings = index;

            index = 0;
            for (auto& attribute : inputAttributes)
            {
                _inputAttribute[index] = attribute;
                index++;
            }
            _countInputAttributes = index;
        }

        u32 _countInputBindings;
        std::array<InputBinding, k_maxVertexInputBindings> _inputBindings;

        u32 _countInputAttributes;
        std::array<InputAttribute, k_maxVertexInputAttributes> _inputAttribute;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamBufferDescription struct
    */
    struct StreamBufferDescription
    {
        StreamBufferDescription() = delete;
        explicit StreamBufferDescription(const StreamBufferDescription& desc) = default;

        StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream) noexcept;
        StreamBufferDescription(VertexStreamBuffer* vertex, u32 stream, u32 offset, u32 size) noexcept;
        ~StreamBufferDescription();


        std::vector<std::pair<Buffer*, u32>> _vertices;
        std::vector<std::pair<u64, u64>>     _offsets;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
