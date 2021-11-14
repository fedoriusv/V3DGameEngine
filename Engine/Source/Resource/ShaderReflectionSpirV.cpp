#include "ShaderReflectionSpirV.h"
#include "Renderer/Shader.h"
#include "Stream/StreamManager.h"

#ifdef USE_SPIRV
#include <SPIRV-Cross/spirv_glsl.hpp>

namespace v3d
{
namespace resource
{

ShaderReflectionSpirV::ShaderReflectionSpirV(renderer::ShaderHeader::ShaderModel model) noexcept
    : m_model(model)
    , m_version(0)
{
}

ShaderReflectionSpirV::~ShaderReflectionSpirV()
{
}

bool ShaderReflectionSpirV::reflect(const std::vector<u32>& spirv, stream::Stream* stream)
{
   auto convertSPRIVTypeToFormat = [](const spirv_cross::SPIRType& type) -> renderer::Format
    {
        switch (type.basetype)
        {
        case spirv_cross::SPIRType::Float:
        {
            if (type.width == 32)
            {
                if (type.vecsize == 1)
                    return renderer::Format_R32_SFloat;
                if (type.vecsize == 2)
                    return renderer::Format_R32G32_SFloat;
                if (type.vecsize == 3)
                    return renderer::Format_R32G32B32_SFloat;
                if (type.vecsize == 4)
                    return renderer::Format_R32G32B32A32_SFloat;
            }
            break;
        }

        case spirv_cross::SPIRType::Int:
        {
            if (type.width == 32)
            {
                if (type.vecsize == 1)
                    return renderer::Format_R32_SInt;
                if (type.vecsize == 2)
                    return renderer::Format_R32G32_SInt;
                if (type.vecsize == 3)
                    return renderer::Format_R32G32B32_SInt;
                if (type.vecsize == 4)
                    return renderer::Format_R32G32B32A32_SInt;
            }
            break;
        }

        case spirv_cross::SPIRType::UInt:
        {
            if (type.width == 32)
            {
                if (type.vecsize == 1)
                    return renderer::Format_R32_UInt;
                if (type.vecsize == 2)
                    return renderer::Format_R32G32_UInt;
                if (type.vecsize == 3)
                    return renderer::Format_R32G32B32_UInt;
                if (type.vecsize == 4)
                    return renderer::Format_R32G32B32A32_UInt;
            }
            break;
        }

        default:
            ASSERT(false, "format not found");
        }

        return renderer::Format_Undefined;
    };

    auto convertSPRIVTypeToImageFormat = [](const spirv_cross::SPIRType& type)->renderer::Format
    {
        ASSERT(type.basetype == spirv_cross::SPIRType::Image, "wrong type");
        switch (type.image.format)
        {
            case spv::ImageFormat::ImageFormatRgba32f:
                return renderer::Format_R32G32B32A32_SFloat;

            case spv::ImageFormat::ImageFormatRg32f:
                return renderer::Format_R32G32_SFloat;

            case spv::ImageFormat::ImageFormatR32f:
                return renderer::Format_R32_SFloat;

            case spv::ImageFormat::ImageFormatRgba8:
                return renderer::Format_R8G8B8A8_UNorm;

            case spv::ImageFormat::ImageFormatRgba8Snorm:
                return renderer::Format_R8G8B8A8_SNorm;

            case spv::ImageFormat::ImageFormatRg8:
                return renderer::Format_R8G8_UNorm;

            case spv::ImageFormat::ImageFormatRg8Snorm:
                return renderer::Format_R8G8_SNorm;

            case spv::ImageFormat::ImageFormatR8:
                return renderer::Format_R8_UNorm;

            case spv::ImageFormat::ImageFormatR8Snorm:
                return renderer::Format_R8_SNorm;

            case spv::ImageFormat::ImageFormatUnknown:
            default:
                ASSERT(false, "format not found");
                break;
        }

        return renderer::Format_Undefined;
    };

    spirv_cross::CompilerGLSL glsl(spirv);
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    const spirv_cross::CompilerGLSL::Options& options = glsl.get_common_options();
    m_version = options.version;

    u32 inputChannelCount = static_cast<u32>(resources.stage_inputs.size());
    stream->write<u32>(inputChannelCount);
    for (auto& inputChannel : resources.stage_inputs)
    {
        u32 location = glsl.get_decoration(inputChannel.id, spv::DecorationLocation);

        const spirv_cross::SPIRType& type = glsl.get_type(inputChannel.type_id);

        renderer::Shader::Attribute input;
        input._location = location;
        input._format = convertSPRIVTypeToFormat(type);
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(inputChannel.id);
        ASSERT(!name.empty(), "empty name");
        input._name = name;
#endif
        input >> stream;
    }

    u32 outputChannelCount = static_cast<u32>(resources.stage_outputs.size());
    stream->write<u32>(outputChannelCount);
    for (auto& outputChannel : resources.stage_outputs)
    {
        u32 location = glsl.get_decoration(outputChannel.id, spv::DecorationLocation);

        const spirv_cross::SPIRType& type = glsl.get_type(outputChannel.type_id);
        renderer::Shader::Attribute output;
        output._location = location;
        output._format = convertSPRIVTypeToFormat(type);
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(outputChannel.id);
        ASSERT(!name.empty(), "empty name");
        output._name = name;
#endif
        output >> stream;
    }

    u32 unifromBufferCount = static_cast<u32>(resources.uniform_buffers.size());
    stream->write<u32>(unifromBufferCount);
    s32 buffID = 0;
    for (auto& buffer : resources.uniform_buffers)
    {
        u32 binding = glsl.get_decoration(buffer.id, spv::DecorationBinding);
        u32 set = glsl.get_decoration(buffer.id, spv::DecorationDescriptorSet);
        const spirv_cross::SPIRType& block_type = glsl.get_type(buffer.type_id);

        renderer::Shader::UniformBuffer block;
        block._id = buffID;
        block._set = set;
        block._binding = binding;
        block._array = block_type.array.empty() ? 1 : block_type.array[0];
        ASSERT(block._array == 1, "unsupported now. Use arrays inside uniform block");
#if USE_STRING_ID_SHADER
        auto isHLSL = [](renderer::ShaderHeader::ShaderModel model) -> bool
        {
            return model == renderer::ShaderHeader::ShaderModel::HLSL_5_0 || model == renderer::ShaderHeader::ShaderModel::HLSL_5_1;
        };

        std::string name = glsl.get_name(buffer.id);
        if (isHLSL(m_model) && name.empty())
        {
            name = "cbuffer_" + std::to_string(buffID);
        }
        else
        {
            ASSERT(!name.empty(), "empty name");
        }
        block._name = name;
#endif
        //u32 posMembers = stream->tell();
        u32 countMembers = 0;
        u32 membersSize = 0;
        u32 index = 0;

        auto convertSPRIVTypeToDataType = [](const spirv_cross::SPIRType& type) -> renderer::DataType
        {
            u32 col = type.columns;
            u32 row = type.vecsize;

            if (row == 1 && col == 1) //value
            {
                switch (type.basetype)
                {
                case spirv_cross::SPIRType::Int:
                case spirv_cross::SPIRType::UInt:
                    return renderer::DataType::DataType_Int;

                case spirv_cross::SPIRType::Half:
                case spirv_cross::SPIRType::Float:
                    return renderer::DataType::DataType_Float;

                case spirv_cross::SPIRType::Double:
                    return renderer::DataType::DataType_Double;

                case spirv_cross::SPIRType::Struct:
                    return renderer::DataType::DataType_Struct;

                default:
                    break;
                }

                ASSERT(false, "not support");
                return  renderer::DataType::DataType_None;
            }
            else if (row > 1 && col == 1) //vector
            {
                switch (type.basetype)
                {
                case spirv_cross::SPIRType::Half:
                case spirv_cross::SPIRType::Float:
                //case spirv_cross::SPIRType::Double:
                {
                    if (row == 2)
                    {
                        return renderer::DataType::DataType_Vector2;
                    }
                    else if (row == 3)
                    {
                        return renderer::DataType::DataType_Vector3;
                    }
                    else if (row == 4)
                    {
                        return renderer::DataType::DataType_Vector4;
                    }
                }
                break;

                default:
                    break;
                }

                ASSERT(false, "not support");
                return  renderer::DataType::DataType_None;
            }
            else if (col > 1 && row > 1) //matrix
            {
                switch (type.basetype)
                {
                case spirv_cross::SPIRType::Half:
                case spirv_cross::SPIRType::Float:
                //case spirv_cross::SPIRType::Double:
                {
                    if (col == 3 && row == 3)
                    {
                        return renderer::DataType::DataType_Matrix3;
                    }
                    else if (col == 4 && row == 4)
                    {
                        return renderer::DataType::DataType_Matrix4;
                    }
                }
                break;

                default:
                    break;
                }

                ASSERT(false, "not support");
                return  renderer::DataType::DataType_None;
            }

            ASSERT(false, "not support");
            return  renderer::DataType::DataType_None;
        };

        auto calculateMemberSizeInsideStruct = [&glsl](const spirv_cross::SPIRType& type) -> u32
        {
            if (type.basetype != spirv_cross::SPIRType::Struct)
            {
                return 0;
            }

            u32 size = 0;
            for (auto& member : type.member_types)
            {
                const spirv_cross::SPIRType& type = glsl.get_type(member);
                u32 col = type.columns;
                u32 row = type.vecsize;
                u32 array = type.array.empty() ? 1 : type.array[0];
                ASSERT(type.basetype != spirv_cross::SPIRType::Struct, "struct");

                u32 dataSize = (type.width == 32) ? 4 : 8;
                size += dataSize * col * row * array;
            }

            return size;
        };


        u32 offset = 0;
        while (index < block_type.member_types.size())
        {
            const spirv_cross::SPIRType& type = glsl.get_type(block_type.member_types[index]);
            u32 col = type.columns;
            u32 row = type.vecsize;

            renderer::Shader::UniformBuffer::Uniform uniform;
            uniform._bufferId = buffID;
            uniform._array = type.array.empty() ? 1 : type.array[0];
            uniform._type = convertSPRIVTypeToDataType(type);
#if USE_STRING_ID_SHADER
            const std::string& member_name = glsl.get_member_name(buffer.base_type_id, index);
            if (member_name.empty())
            {
                ASSERT(false, "empty field");
            }
            uniform._name = member_name;
#endif
            if (uniform._type == renderer::DataType::DataType_Struct)
            {
                uniform._size = calculateMemberSizeInsideStruct(type) * uniform._array;
            }
            else
            {
                u32 dataSize = (type.width == 32) ? 4 : 8;
                uniform._size = dataSize * col * row * uniform._array;
            }

            uniform._offset = offset;
            offset += uniform._size;

            ++index;

            block._uniforms.push_back(uniform);
            membersSize += uniform._size;
        }

        block._size = membersSize * block._array;
        block >> stream;

        //u32 posCurr = stream->tell();
        countMembers = index;

        ++buffID;
    }

    auto convertSPRIVTypeToTextureData = [](const spirv_cross::SPIRType& type) -> renderer::TextureTarget
    {
        switch (type.image.dim)
        {
        case spv::Dim::Dim1D:
            return renderer::TextureTarget::Texture1D;

        case spv::Dim::Dim2D:
            return renderer::TextureTarget::Texture2D;

        case spv::Dim::Dim3D:
            return renderer::TextureTarget::Texture3D;

        case spv::Dim::DimCube:
            return renderer::TextureTarget::TextureCubeMap;

        default:
            break;
        }

        ASSERT(false, "not support");
        return  renderer::TextureTarget::Texture2D;
    };

    u32 sampledImagesCount = static_cast<u32>(resources.sampled_images.size());
    stream->write<u32>(sampledImagesCount);
    for (auto& image : resources.sampled_images)
    {
        u32 binding = glsl.get_decoration(image.id, spv::DecorationBinding);
        u32 set = glsl.get_decoration(image.id, spv::DecorationDescriptorSet);

        const spirv_cross::SPIRType& type = glsl.get_type(image.type_id);
        bool depth = type.image.depth;

        renderer::Shader::Image sampledImage;
        sampledImage._set = set;
        sampledImage._binding = binding;
        sampledImage._target = convertSPRIVTypeToTextureData(type);
        sampledImage._array = type.array.empty() ? 1 : type.array[0];
        sampledImage._ms = type.image.ms;
        sampledImage._depth = depth;
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(image.id);
        ASSERT(!name.empty(), "empty name");
        sampledImage._name = name;
#endif
        sampledImage >> stream;
    }

    u32 imagesCount = static_cast<u32>(resources.separate_images.size());
    stream->write<u32>(imagesCount);
    for (auto& image : resources.separate_images)
    {
        u32 binding = glsl.get_decoration(image.id, spv::DecorationBinding);
        u32 set = glsl.get_decoration(image.id, spv::DecorationDescriptorSet);

        const spirv_cross::SPIRType& type = glsl.get_type(image.type_id);
        bool depth = type.image.depth;

        renderer::Shader::Image sepImage;
        sepImage._set = set;
        sepImage._binding = binding;
        sepImage._target = convertSPRIVTypeToTextureData(type);
        sepImage._array = type.array.empty() ? 1 : type.array[0];
        sepImage._ms = type.image.ms;
        sepImage._depth = depth;
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(image.id);
        ASSERT(!name.empty(), "empty name");
        sepImage._name = name;
#endif
        sepImage >> stream;
    }

    u32 samplersCount = static_cast<u32>(resources.separate_samplers.size());
    stream->write<u32>(samplersCount);
    for (auto& sampler : resources.separate_samplers)
    {
        u32 binding = glsl.get_decoration(sampler.id, spv::DecorationBinding);
        u32 set = glsl.get_decoration(sampler.id, spv::DecorationDescriptorSet);

        renderer::Shader::Sampler samplerImage;
        samplerImage._set = set;
        samplerImage._binding = binding;
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(sampler.id);
        ASSERT(!name.empty(), "empty name");
        samplerImage._name = name;
#endif
        samplerImage >> stream;
    }

    u32 storageImages = static_cast<u32>(resources.storage_images.size());
    stream->write<u32>(storageImages);
    for (auto& image : resources.storage_images)
    {
        u32 binding = glsl.get_decoration(image.id, spv::DecorationBinding);
        u32 set = glsl.get_decoration(image.id, spv::DecorationDescriptorSet);

        const spirv_cross::SPIRType& type = glsl.get_type(image.type_id);

        renderer::Shader::StorageImage storageImage;
        storageImage._set = set;
        storageImage._binding = binding;
        storageImage._array = type.array.empty() ? 1 : type.array[0];
        storageImage._target = convertSPRIVTypeToTextureData(type);
        storageImage._format = convertSPRIVTypeToImageFormat(type);
        storageImage._readonly = glsl.get_decoration(image.id, spv::DecorationNonWritable);
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(image.id);
        ASSERT(!name.empty(), "empty name");
        storageImage._name = name;
#endif
        storageImage >> stream;
    }

    u32 storageBuffers = static_cast<u32>(resources.storage_buffers.size());
    stream->write<u32>(storageBuffers);
    for (auto& buffer : resources.storage_buffers)
    {
        u32 binding = glsl.get_decoration(buffer.id, spv::DecorationBinding);
        u32 set = glsl.get_decoration(buffer.id, spv::DecorationDescriptorSet);

        renderer::Shader::StorageBuffer storageBuffer;
        storageBuffer._set = set;
        storageBuffer._binding = binding;
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(buffer.id);
        ASSERT(!name.empty(), "empty name");
        storageBuffer._name = name;
#endif
        storageBuffer >> stream;
    }

    u32 pushConstantCount = static_cast<u32>(resources.push_constant_buffers.size());
    stream->write<u32>(pushConstantCount);
    for (auto& pushConstant : resources.push_constant_buffers)
    {
        u32 offset = glsl.get_decoration(pushConstant.id, spv::DecorationOffset);

        const spirv_cross::SPIRType& type = glsl.get_type(pushConstant.type_id);

        renderer::Shader::PushConstant constant;
        constant._offset = offset;
        constant._size = type.width; //TODO check
#if USE_STRING_ID_SHADER
        const std::string& name = glsl.get_name(pushConstant.id);
        ASSERT(!name.empty(), "empty name");
        constant._name = name;
#endif
        constant >> stream;
    }

    return true;
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV