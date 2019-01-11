#include "ShaderSpirVDecoder.h"
#include "Shader.h"
#include "Stream/FileLoader.h"

#include "Utils/Logger.h"

#ifdef USE_SPIRV
#   include "shaderc/libshaderc/include/shaderc/shaderc.hpp"
#endif // USE_SPIRV

namespace v3d
{
namespace resource
{

ShaderSpirVDecoder::ShaderSpirVDecoder(const ShaderHeader& header, bool reflections)
    : m_header(header)
    , m_reflections(reflections)
{
}

ShaderSpirVDecoder::ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const ShaderHeader& header, bool reflections)
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
#ifndef DEBUG
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
            auto getShaderType = [&validShaderType](const std::string& name) -> shaderc_shader_kind
            {
                std::string fileExtension = stream::FileLoader::getFileExtension(name);
                if (fileExtension == "vert")
                {
                    validShaderType = true;
                    return shaderc_shader_kind::shaderc_vertex_shader;
                }
                else if (fileExtension == "frag")
                {
                    validShaderType = true;
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

            if (m_reflections)
            {
                if (!ShaderSpirVDecoder::parseReflections({ result.cbegin(), result.cend() }))
                {
                    LOG_ERROR("ShaderSpirVDecoder::decode: parseReflections failed for shader: %s", name.c_str());
                    //return nullptr;
                }
            }
        }
        else
        {
            //bytecode
            ASSERT(false, "implement");
        }
        /*stream::Stream * resource = nullptr;
        return ResourceCreator::create<Shader>(nullptr, resource);*/
#else //USE_SPIRV
        ASSERT(false, "spirv undefined");
#endif //USE_SPIRV
    }

    return nullptr;
}

bool ShaderSpirVDecoder::parseReflections(const std::vector<u32>& spirv)
{
    return false;
}

} //namespace resource
} //namespace v3d
