#include "ShaderSpirVDecoder.h"
#include "Shader.h"
#include "Stream/FileLoader.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"

#ifdef USE_SPIRV
#   include "shaderc/libshaderc/include/shaderc/shaderc.hpp"
//#   include "shaderc/shaderc.hpp"
#   include "SPIRV-Cross/spirv_glsl.hpp"
#endif // USE_SPIRV

namespace v3d
{
namespace resource
{

ShaderSpirVDecoder::ShaderSpirVDecoder(const ShaderHeader& header, bool reflections) noexcept
    : m_header(header)
    , m_reflections(reflections)
{
}

ShaderSpirVDecoder::ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const ShaderHeader& header, bool reflections) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_reflections(reflections)
{
}

ShaderSpirVDecoder::~ShaderSpirVDecoder()
{
}

Resource * ShaderSpirVDecoder::decode(const stream::Stream* stream, const std::string& name)
{
    if (stream->size() > 0)
    {
#ifdef USE_SPIRV
        stream->seekBeg(0);

        std::string source;
        stream->read(source);

        if (m_header._contentType == ShaderHeader::ShaderResource::ShaderResource_Source)
        {
            shaderc::CompileOptions options;
            options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
            options.SetOptimizationLevel(shaderc_optimization_level_zero);
#ifdef DEBUG
            options.SetWarningsAsErrors();
#endif
            switch (m_header._shaderLang)
            {
            case ShaderHeader::ShaderLang::ShaderLang_GLSL:
                options.SetSourceLanguage(shaderc_source_language_glsl);
                break;

            case ShaderHeader::ShaderLang::ShaderLang_HLSL:
                options.SetSourceLanguage(shaderc_source_language_hlsl);
                break;

            default:
                ASSERT(false, "shader lang doesn't support");
                options.SetSourceLanguage(shaderc_source_language_glsl);
            }

            for (auto& define : m_header._defines)
            {
                if (define.second.empty())
                {
                    options.AddMacroDefinition(define.first);
                }
                else
                {
                    options.AddMacroDefinition(define.first, define.second);
                }
            }

            bool validShaderType = false;
            resource::ShaderType type = resource::ShaderType::ShaderType_Undefined;
            auto getShaderType = [&validShaderType, &type](const std::string& name) -> shaderc_shader_kind
            {
                std::string fileExtension = stream::FileLoader::getFileExtension(name);
                if (fileExtension == "vert")
                {
                    validShaderType = true;
                    type = resource::ShaderType::ShaderType_Vertex;
                    return shaderc_shader_kind::shaderc_vertex_shader;
                }
                else if (fileExtension == "frag")
                {
                    validShaderType = true;
                    type = resource::ShaderType::ShaderType_Fragment;
                    return shaderc_shader_kind::shaderc_fragment_shader;
                }
                else if (fileExtension == "geom")
                {
                    validShaderType = true;
                    return shaderc_shader_kind::shaderc_geometry_shader;
                }
                else if (fileExtension == "comp")
                {
                    validShaderType = true;
                    return shaderc_shader_kind::shaderc_compute_shader;
                }
                else if (fileExtension == "tesc")
                {
                    validShaderType = true;
                    return shaderc_shader_kind::shaderc_tess_control_shader;
                }
                else if (fileExtension == "tese")
                {
                    validShaderType = true;
                    return shaderc_shader_kind::shaderc_tess_evaluation_shader;
                }
                validShaderType = false;
                return shaderc_shader_kind::shaderc_vertex_shader;
            };

            shaderc_shader_kind shaderType = getShaderType(name);
            if (!validShaderType)
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: Invalid shader type or unsupport");
                return nullptr;
            }

            LOG_DEBUG("Compile Shader %s to SpirV:\n %s\n", name.c_str(), source.c_str());

            shaderc::Compiler compiler;
            shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, shaderType, "shader", options);
            if (!compiler.IsValid())
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: CompileGlslToSpv is invalid");
                return nullptr;
            }


            shaderc_compilation_status status = result.GetCompilationStatus();
            auto getCompileError = [](shaderc_compilation_status status) -> std::string
            {
                switch (status)
                {
                case shaderc_compilation_status_invalid_stage:
                    return "shaderc_compilation_status_invalid_stage";
                case shaderc_compilation_status_compilation_error:
                    return "shaderc_compilation_status_compilation_error";
                case shaderc_compilation_status_internal_error:
                    return "shaderc_compilation_status_internal_error";
                case shaderc_compilation_status_null_result_object:
                    return "shaderc_compilation_status_null_result_object";
                case shaderc_compilation_status_invalid_assembly:
                    return "shaderc_compilation_status_invalid_assembly";
                default:
                    return "unknown";
                }
            };

            auto getStringType = [](shaderc_shader_kind shaderType) -> std::string
            {
                switch (shaderType)
                {
                case shaderc_glsl_vertex_shader:
                    return "vertex";
                case shaderc_glsl_fragment_shader:
                    return "fragment";
                case shaderc_glsl_compute_shader:
                    return "compute";
                case shaderc_glsl_geometry_shader:
                    return "geometry";
                case shaderc_glsl_tess_control_shader:
                    return "tess_control";
                case shaderc_glsl_tess_evaluation_shader:
                    return "tess_eval";
                default:
                    return "unknown";
                }
            };
            std::string stringType = getStringType(shaderType);
            if (status != shaderc_compilation_status_success)
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: Shader [%s]%s, compile error %s", stringType.c_str(), name.c_str(), getCompileError(status).c_str());
            }

            if (result.GetNumErrors() > 0)
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: header [%s]%s shader error messages:\n%s", stringType.c_str(), name.c_str(), result.GetErrorMessage().c_str());
                return nullptr;
            }

            if (result.GetNumWarnings() > 0)
            {
                LOG_WARNING("ShaderSpirVDecoder::decode: header [%s]%s shader warnings messages:\n%s", stringType.c_str(), name.c_str(), result.GetErrorMessage().c_str());
            }

            u32 size = (u32)(result.cend() - result.cbegin()) * sizeof(u32);
            stream::Stream* resourceSpirvBinary = stream::StreamManager::createMemoryStream(nullptr, size + sizeof(u32) + sizeof(bool));
            resourceSpirvBinary->write<u32>(size);
            resourceSpirvBinary->write(result.cbegin(), size);

            resourceSpirvBinary->write<bool>(m_reflections);
            if (m_reflections)
            {
                if (!ShaderSpirVDecoder::parseReflections({ result.cbegin(), result.cend() }, resourceSpirvBinary))
                {
                    LOG_ERROR("ShaderSpirVDecoder::decode: parseReflections failed for shader: %s", name.c_str());
                    delete resourceSpirvBinary;

                    return nullptr;
                }
            }

            ShaderHeader* resourceHeader = new ShaderHeader(m_header);
            resourceHeader->_type = type;
            resourceHeader->_apiVersion = m_sourceVersion;

            Resource* resource = new Shader(resourceHeader);
            resource->init(resourceSpirvBinary);

            return resource;
        }
        else
        {
            //bytecode
            ASSERT(false, "implement");
        }
#else //USE_SPIRV
        ASSERT(false, "spirv undefined");
#endif //USE_SPIRV
    }

    return nullptr;
}

bool ShaderSpirVDecoder::parseReflections(const std::vector<u32>& spirv, stream::Stream* stream)
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
        }

        default:
            ASSERT(false, "format not found");
        }

        return renderer::Format_Undefined;
    };

    if (m_header._shaderLang == ShaderHeader::ShaderLang::ShaderLang_GLSL)
    {
        spirv_cross::CompilerGLSL glsl(spirv);
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        const spirv_cross::CompilerGLSL::Options& options = glsl.get_common_options();
        m_sourceVersion = options.version;

        u32 inputChannelCount = static_cast<u32>(resources.stage_inputs.size());
        stream->write<u32>(inputChannelCount);
        for (auto& inputChannel : resources.stage_inputs)
        {
            const std::string& name = glsl.get_name(inputChannel.id);
            ASSERT(!name.empty(), "empty name");

            u32 location = glsl.get_decoration(inputChannel.id, spv::DecorationLocation);

            const spirv_cross::SPIRType& type = glsl.get_type(inputChannel.type_id);

            Shader::Attribute input;
            input._location = location;
            input._format = convertSPRIVTypeToFormat(type);
            input._name = name;

            input >> stream;
        }

        u32 outputChannelCount = static_cast<u32>(resources.stage_outputs.size());
        stream->write<u32>(outputChannelCount);
        for (auto& outputChannel : resources.stage_outputs)
        {
            const std::string& name = glsl.get_name(outputChannel.id);
            ASSERT(!name.empty(), "empty name");

            u32 location = glsl.get_decoration(outputChannel.id, spv::DecorationLocation);

            const spirv_cross::SPIRType& type = glsl.get_type(outputChannel.type_id);
            u32 col = type.columns;
            u32 row = type.vecsize;

            Shader::Attribute output;
            output._location = location;
            output._format = convertSPRIVTypeToFormat(type);
            output._name = name;

            output >> stream;
        }

        u32 unifromBufferCount = static_cast<u32>(resources.uniform_buffers.size());
        //stream->write<u32>(unifromBufferCount);
        s32 buffID = 0;
        for (auto& buffer : resources.uniform_buffers)
        {
            const std::string& name = glsl.get_name(buffer.id);
            ASSERT(!name.empty(), "empty name");

            u32 binding = glsl.get_decoration(buffer.id, spv::DecorationBinding);
            u32 set = glsl.get_decoration(buffer.id, spv::DecorationDescriptorSet);
            const spirv_cross::SPIRType& block_type = glsl.get_type(buffer.type_id);

            /*stream->write<s32>(buffID);
            stream->write(name);
            stream->write<u32>(set);
            stream->write<u32>(binding);*/

            u32 posMembers = stream->tell();

            u32 countMembers = 0;
            //stream->write<u32>(countMembers);

            u32 index = 0;
            while (true)
            {
                const std::string& member_name = glsl.get_member_name(buffer.base_type_id, index);
                if (member_name.empty())
                {
                    break;
                }
                const spirv_cross::SPIRType& type = glsl.get_type(block_type.member_types[index]);
                u32 col = type.columns;
                u32 row = type.vecsize;
                ++index;

                //ShaderDataType::EDataType innerType = getInnerDataType(type);
                //stream->write<s32>(buffID);
                //stream->write(member_name);
                //stream->write<ShaderDataType::DataType>(innerType);
                //stream->write<u32>(col);
                //stream->write<u32>(row);
            }

            u32 posCurr = stream->tell();
            countMembers = index;
            //stream->seekBeg(posMembers);
            //stream->write<u32>(countMembers);
            //stream->seekBeg(posCurr);

            ++buffID;
        }

        u32 samplersCount = static_cast<u32>(resources.sampled_images.size());
        //stream->write<u32>(samplersCount);
        for (auto& image : resources.sampled_images)
        {
            const std::string& name = glsl.get_name(image.id);
            ASSERT(!name.empty(), "empty name");

            u32 binding = glsl.get_decoration(image.id, spv::DecorationBinding);
            u32 set = glsl.get_decoration(image.id, spv::DecorationDescriptorSet);

            const spirv_cross::SPIRType& type = glsl.get_type(image.type_id);
            bool depth = type.image.depth;

            //TextureTarget innerType = getInnerTextureTarget(type);
            //stream->write(name);
            //stream->write<u32>(set);
            //stream->write<u32>(binding);
            //stream->write<TextureTarget>(innerType);
            //stream->write<bool>(depth);
        }

        return true;

    }
    else
    {
        return false;
    }
}

} //namespace resource
} //namespace v3d