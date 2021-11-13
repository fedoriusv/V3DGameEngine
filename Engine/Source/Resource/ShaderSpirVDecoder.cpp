#include "ShaderSpirVDecoder.h"
#include "Stream/FileLoader.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#ifdef USE_SPIRV
#   include "ShaderReflectionSpirV.h"
#   include <shaderc/libshaderc/include/shaderc/shaderc.hpp>

#   define PATCH_SYSTEM 0
#   include "ShaderPatchSpirV.h"
#   include "ShaderSpirVPatcherBugDriverFix.h"

namespace v3d
{
namespace resource
{

const std::map<std::string, renderer::ShaderType> k_SPIRV_ExtensionList =
{
    //glsl
    { "vert", renderer::ShaderType::Vertex },
    { "frag", renderer::ShaderType::Fragment },
    { "comp", renderer::ShaderType::Compute },

    //hlsl
    { "vs", renderer::ShaderType::Vertex },
    { "ps", renderer::ShaderType::Fragment },
    { "cs", renderer::ShaderType::Compute },
};

 ShaderSpirVDecoder::ShaderSpirVDecoder(const renderer::ShaderHeader& header, bool reflections) noexcept
    : m_header(header)
    , m_reflections(reflections)
{
}

ShaderSpirVDecoder::ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_reflections(reflections)
{
}

Resource* ShaderSpirVDecoder::decode(const stream::Stream* stream, const std::string& name) const
{
    if (stream->size() > 0)
    {
        stream->seekBeg(0);

        if (m_header._contentType == renderer::ShaderHeader::ShaderResource::Source)
        {
            std::string source;
            source.resize(stream->size());
            stream->read(source.data(), stream->size());
#if DEBUG
            utils::Timer timer;
            timer.start();
#endif
            shaderc::CompileOptions options;
            switch (m_header._optLevel)
            {
            case 0:
            default:
                options.SetOptimizationLevel(shaderc_optimization_level_zero);
                break;

            case 1:
                options.SetOptimizationLevel(shaderc_optimization_level_size);
                break;

            case 2:
            case 3:
                options.SetOptimizationLevel(shaderc_optimization_level_performance);
                break;
            }

#if (DEBUG & VULKAN_DEBUG)
            options.SetWarningsAsErrors();
#endif
            switch (m_header._shaderModel)
            {
            case renderer::ShaderHeader::ShaderModel::GLSL_450:
                options.SetSourceLanguage(shaderc_source_language_glsl);
#if (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_0)
                options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
                options.SetTargetSpirv(shaderc_spirv_version_1_0);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_1)
                options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
                options.SetTargetSpirv(shaderc_spirv_version_1_3);
#else
                options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
                options.SetTargetSpirv(shaderc_spirv_version_1_5);
#endif
                break;

            case renderer::ShaderHeader::ShaderModel::HLSL_5_0:
            case renderer::ShaderHeader::ShaderModel::HLSL_5_1:
                options.SetSourceLanguage(shaderc_source_language_hlsl);
#if (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_0)
                options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
                options.SetTargetSpirv(shaderc_spirv_version_1_0);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_1)
                options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
                options.SetTargetSpirv(shaderc_spirv_version_1_3);
#else
                options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
                options.SetTargetSpirv(shaderc_spirv_version_1_5);
#endif
                break;

            default:
                LOG_ERROR("ShaderSpirVDecoder::decode: shader model %d is not supported", m_header._shaderModel);
                ASSERT(false, "shader lang doesn't support");
                return nullptr;
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
            renderer::ShaderType type = m_header._type;

            auto getShaderTypeFromName = [&validShaderType, &type](const std::string& name) -> shaderc_shader_kind
            {
                std::string fileExtension = stream::FileLoader::getFileExtension(name);
                auto result = k_SPIRV_ExtensionList.find(fileExtension);
                if (result == k_SPIRV_ExtensionList.cend())
                {
                    validShaderType = false;
                    return shaderc_shader_kind::shaderc_vertex_shader;
                }

                type = result->second;
                validShaderType = true;
                switch (type)
                {
                case renderer::ShaderType::Vertex:
                    return shaderc_shader_kind::shaderc_vertex_shader;

                case renderer::ShaderType::Fragment:
                    return shaderc_shader_kind::shaderc_fragment_shader;

                case renderer::ShaderType::Compute:
                    return shaderc_shader_kind::shaderc_compute_shader;

                default:
                    validShaderType = false;
                    return shaderc_shader_kind::shaderc_vertex_shader;
                }

                validShaderType = false;
                return shaderc_shader_kind::shaderc_vertex_shader;
            };

            auto getShaderType = [&validShaderType](renderer::ShaderType type) -> shaderc_shader_kind
            {
                validShaderType = true;
                switch (type)
                {
                case renderer::ShaderType::Vertex:
                    return  shaderc_shader_kind::shaderc_vertex_shader;

                case renderer::ShaderType::Fragment:
                    return  shaderc_shader_kind::shaderc_fragment_shader;

                case renderer::ShaderType::Compute:
                    return  shaderc_shader_kind::shaderc_compute_shader;

                default:
                    validShaderType = false;
                    return shaderc_shader_kind::shaderc_vertex_shader;
                }

                validShaderType = false;
                return shaderc_shader_kind::shaderc_vertex_shader;
            };

            shaderc_shader_kind shaderType = (type == renderer::ShaderType::Undefined)  ? getShaderTypeFromName(name) : getShaderType(type);
            if (!validShaderType)
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: Invalid shader type or unsupport");
                return nullptr;
            }

            LOG_DEBUG("Compile Shader %s to SpirV:\n %s\n", name.c_str(), source.c_str());

            shaderc::Compiler compiler;
            shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, shaderType, "shader", m_header._entryPoint.c_str(), options);
            if (!compiler.IsValid())
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: CompileGlslToSpv is invalid");
                return nullptr;
            }

            shaderc_compilation_status status = result.GetCompilationStatus();
            auto getCompileStatusError = [](shaderc_compilation_status status) -> std::string
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
                LOG_ERROR("ShaderSpirVDecoder::decode: Shader [%s]%s, compile error %s", stringType.c_str(), name.c_str(), getCompileStatusError(status).c_str());
                if (result.GetNumErrors() == 0)
                {
                    return nullptr;
                }
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
#if (DEBUG & VULKAN_DEBUG)
            shaderc::AssemblyCompilationResult assambleResult = compiler.CompileGlslToSpvAssembly(source, shaderType, "shader", m_header._entryPoint.c_str(), options);
            ASSERT(compiler.IsValid(), "error");
            shaderc_compilation_status assambleStatus = assambleResult.GetCompilationStatus();
            ASSERT(assambleStatus == shaderc_compilation_status_success, "error");
            LOG_DEBUG("ASSEMBLE SPIRV:");
            std::string assambleSPIRV(assambleResult.cbegin(), assambleResult.cend());
            LOG_DEBUG("%s", assambleSPIRV.c_str());
#endif //(DEBUG & VULKAN_DEBUG)

            std::vector<u32> spirvBinary{ result.cbegin(), result.cend() };

#if PATCH_SYSTEM
            if (shaderType == shaderc_fragment_shader && m_header._flags & 0x08) //patched
            {
                std::vector<u32> spirvBinaryPatched(spirvBinary);
                PatchDriverBugOptimization patch;

                ShaderPatcherSpirV patcher;
                if (patcher.process(&patch, spirvBinaryPatched))
                {
                    std::swap(spirvBinary, spirvBinaryPatched);
                }
                else
                {
                    ASSERT(false, "patch is failed");
                }
            }
#endif //PATCH_SYSTEM

            u32 size = static_cast<u32>(spirvBinary.size()) * sizeof(u32);
            stream::Stream* resourceSpirvBinary = stream::StreamManager::createMemoryStream(nullptr, size + sizeof(u32) + sizeof(bool));
            resourceSpirvBinary->write<u32>(size);
            resourceSpirvBinary->write(spirvBinary.data(), size);

            resourceSpirvBinary->write<bool>(m_reflections);
            if (m_reflections)
            {
                auto isHLSL = [](renderer::ShaderHeader::ShaderModel model) -> bool
                {
                    return model == renderer::ShaderHeader::ShaderModel::HLSL_5_0 || model == renderer::ShaderHeader::ShaderModel::HLSL_5_1;
                };

                if (isHLSL(m_header._shaderModel) || m_header._shaderModel == renderer::ShaderHeader::ShaderModel::GLSL_450)
                {
                    ShaderReflectionSpirV reflector(m_header._shaderModel);
                    if (!reflector.reflect(spirvBinary, resourceSpirvBinary))
                    {
                        LOG_ERROR("ShaderSpirVDecoder::decode: parseReflections failed for shader: %s", name.c_str());
                        delete resourceSpirvBinary;

                        return nullptr;
                    }
                }
                else
                {
                    LOG_ERROR("ShaderSpirVDecoder::decode: shader model si not supported for this reflector: %s", name.c_str());
                    delete resourceSpirvBinary;

                    return nullptr;
                }
            }

#if DEBUG
            timer.stop();
            u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
            LOG_DEBUG("ShaderSpirVDecoder::decode, shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

            renderer::ShaderHeader* resourceHeader = new renderer::ShaderHeader(m_header);
            resourceHeader->_type = type;
            resourceHeader->_shaderModel = renderer::ShaderHeader::ShaderModel::GLSL_450;
#if DEBUG
            resourceHeader->_debugName = name;
#endif

            Resource* resource = new renderer::Shader(resourceHeader);
            resource->init(resourceSpirvBinary);

            return resource;
        }
        else //ShaderResource_Bytecode
        {
            bool validShaderType = false;
            auto getShaderType = [&validShaderType](const std::string& name) -> renderer::ShaderType
            {
                std::string fileExtension = stream::FileLoader::getFileExtension(name);
                if (fileExtension == "vspv")
                {
                    validShaderType = true;
                    return renderer::ShaderType::Vertex;
                }
                else if (fileExtension == "fspv")
                {
                    validShaderType = true;
                    return renderer::ShaderType::Fragment;
                }
                else if (fileExtension == "cspv")
                {
                    validShaderType = true;
                    return renderer::ShaderType::Compute;
                }

                validShaderType = false;
                return renderer::ShaderType::Undefined;
            };

            renderer::ShaderType type = getShaderType(name);
            ASSERT(validShaderType, "invalid type");
#if DEBUG
            utils::Timer timer;
            timer.start();
#endif
            std::vector<u32> bytecode(stream->size() / sizeof(u32));
            stream->read(bytecode.data(), sizeof(u32), static_cast<u32>(bytecode.size()));
            ASSERT(bytecode[0] == 0x07230203, "invalid spirv magic number in head");

            stream::Stream* resourceSpirvBinary = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(bytecode.size()) + sizeof(u32) + sizeof(bool));
            resourceSpirvBinary->write<u32>(stream->size());
            resourceSpirvBinary->write(bytecode.data(), stream->size());

            resourceSpirvBinary->write<bool>(m_reflections);
            if (m_reflections)
            {
                ASSERT(false, m_header._shaderModel == renderer::ShaderHeader::ShaderModel::SpirV);
                ShaderReflectionSpirV reflector(m_header._shaderModel);
                if (!reflector.reflect({ bytecode.cbegin(), bytecode.cend() }, resourceSpirvBinary))
                {
                    LOG_ERROR("ShaderSpirVDecoder::decode: parseReflections failed for shader: %s", name.c_str());
                    delete resourceSpirvBinary;

                    return nullptr;
                }
            }
#if DEBUG
            timer.stop();
            u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
            LOG_DEBUG("ShaderSpirVDecoder::decode, shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif
            renderer::ShaderHeader* resourceHeader = new renderer::ShaderHeader(m_header);
            resourceHeader->_type = type;
            resourceHeader->_shaderModel = renderer::ShaderHeader::ShaderModel::GLSL_450;
#if DEBUG
            resourceHeader->_debugName = name;
#endif

            Resource* resource = new renderer::Shader(resourceHeader);
            resource->init(resourceSpirvBinary);

            return resource;
        }
    }

    ASSERT(false, "spirv undefined");
    return nullptr;
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
