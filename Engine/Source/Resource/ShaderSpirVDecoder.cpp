#include "ShaderSpirVDecoder.h"
#include "Stream/FileLoader.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#ifdef VULKAN_RENDER
#   include "Renderer/Vulkan/VulkanConfig.h"
#endif //VULKAN_RENDER

#ifdef USE_SPIRV
#   include "ShaderReflectionSpirV.h"
#   include <shaderc/libshaderc/include/shaderc/shaderc.hpp>

#   define PATCH_SYSTEM 0
#   include "SpirVPatch/ShaderPatchSpirV.h"
#   include "SpirVPatch/ShaderSpirVPatcherBugDriverFix.h"

#   define LOG_LOADIMG_TIME (DEBUG || 1)

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

constexpr u32 g_SPIRVIndifier = 0x07230203; //Spirv

 ShaderSpirVDecoder::ShaderSpirVDecoder() noexcept
     : ShaderDecoder()
{
}

ShaderSpirVDecoder::ShaderSpirVDecoder(const std::vector<std::string>& supportedExtensions) noexcept
    : ShaderDecoder(supportedExtensions)
{
}

ShaderSpirVDecoder::ShaderSpirVDecoder(std::vector<std::string>&& supportedExtensions) noexcept
    : ShaderDecoder(std::move(supportedExtensions))
{
}

Resource* ShaderSpirVDecoder::decode(const stream::Stream* stream, const Policy* policy, u32 flags, const std::string& name) const
{
    if (!stream || stream->size() == 0)
    {
        ASSERT(false, "bad stream");
        return nullptr;
    }

    stream->seekBeg(0); //read from beginning

    const ShaderDecoder::ShaderPolicy* shaderPolicy = static_cast<const ShaderDecoder::ShaderPolicy*>(policy);
    if (shaderPolicy->_content == renderer::ShaderContent::Source)
    {
        std::string source;
        source.resize(stream->size());
        stream->read(source.data(), stream->size());
#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        shaderc::CompileOptions options;

        if (flags & ShaderCompileFlag::ShaderCompile_OptimizationPerformance || flags & ShaderCompileFlag::ShaderCompile_OptimizationFull)
        {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        }
        else if (flags & ShaderCompileFlag::ShaderCompile_OptimizationSize)
        {
            options.SetOptimizationLevel(shaderc_optimization_level_size);
        }
        else
        {
            options.SetOptimizationLevel(shaderc_optimization_level_zero);
        }
#if (DEBUG & VULKAN_DEBUG)
        options.SetWarningsAsErrors();
#endif

        switch (shaderPolicy->_shaderModel)
        {
        case renderer::ShaderModel::Default:
        case renderer::ShaderModel::GLSL_450:
            options.SetSourceLanguage(shaderc_source_language_glsl);
            options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
#if (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_0)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
            options.SetTargetSpirv(shaderc_spirv_version_1_0);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_1)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
            options.SetTargetSpirv(shaderc_spirv_version_1_3);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_2)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
            options.SetTargetSpirv(shaderc_spirv_version_1_5);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_3)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
            options.SetTargetSpirv(shaderc_spirv_version_1_6);
#else
            ASSERT(false, "unsupported vulkan version");
#endif
            break;

        case renderer::ShaderModel::HLSL_5_1:
        case renderer::ShaderModel::HLSL_6_1:
        case renderer::ShaderModel::HLSL_6_2:
        case renderer::ShaderModel::HLSL_6_3:
        case renderer::ShaderModel::HLSL_6_4:
        case renderer::ShaderModel::HLSL_6_5:
        case renderer::ShaderModel::HLSL_6_6:
            options.SetSourceLanguage(shaderc_source_language_hlsl);
#if (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_0)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
            options.SetTargetSpirv(shaderc_spirv_version_1_0);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_1)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
            options.SetTargetSpirv(shaderc_spirv_version_1_3);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_2)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
            options.SetTargetSpirv(shaderc_spirv_version_1_5);
#elif (VULKAN_CURRENT_VERSION == VULKAN_VERSION_1_3)
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
            options.SetTargetSpirv(shaderc_spirv_version_1_6);
#else
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
            options.SetTargetSpirv(shaderc_spirv_version_1_6);
#endif
            break;

        default:
            LOG_ERROR("ShaderSpirVDecoder::decode: shader model %d is not supported", shaderPolicy->_shaderModel);
            ASSERT(false, "shader lang doesn't support");
            return nullptr;
        }

        for (auto& define : shaderPolicy->_defines)
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

        shaderc_shader_kind scShaderType = getShaderType(shaderPolicy->_type);
        if (!validShaderType)
        {
            LOG_ERROR("ShaderSpirVDecoder::decode: Invalid shader type or unsupport");
            return nullptr;
        }

        LOG_DEBUG("Compile Shader %s to SpirV:\n %s\n", name.c_str(), source.c_str());

        shaderc::Compiler compiler;
        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, scShaderType, "shader", shaderPolicy->_entryPoint.c_str(), options);
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
        std::string stringType = getStringType(scShaderType);
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
        shaderc::AssemblyCompilationResult assambleResult = compiler.CompileGlslToSpvAssembly(source, scShaderType, "shader", shaderPolicy->_entryPoint.c_str(), options);
        ASSERT(compiler.IsValid(), "error");
        shaderc_compilation_status assambleStatus = assambleResult.GetCompilationStatus();
        ASSERT(assambleStatus == shaderc_compilation_status_success, "error");
        LOG_DEBUG("ASSEMBLE SPIRV:");
        std::string assambleSPIRV(assambleResult.cbegin(), assambleResult.cend());
        LOG_DEBUG("%s", assambleSPIRV.c_str());
#endif //(DEBUG & VULKAN_DEBUG)

        std::vector<u32> spirvBinary{ result.cbegin(), result.cend() };

#if PATCH_SYSTEM //TODO
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

        bool reflections = !(flags & ShaderCompileFlag::ShaderCompile_DontUseReflection);

        u32 size = static_cast<u32>(spirvBinary.size()) * sizeof(u32);
        stream::Stream* resourceSpirvBinary = stream::StreamManager::createMemoryStream();

        resourceSpirvBinary->write<renderer::ShaderType>(shaderPolicy->_type);
        resourceSpirvBinary->write<renderer::ShaderModel>(shaderPolicy->_shaderModel);
        resourceSpirvBinary->write(shaderPolicy->_entryPoint);
        resourceSpirvBinary->write<u32>(size);
        resourceSpirvBinary->write(spirvBinary.data(), size);
        resourceSpirvBinary->write<bool>(reflections);

        if (reflections)
        {
            ShaderReflectionSpirV reflector(shaderPolicy->_shaderModel);
            if (!reflector.reflect(spirvBinary, resourceSpirvBinary))
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: parseReflections failed for shader: %s", name.c_str());
                stream::StreamManager::destroyStream(resourceSpirvBinary);

                return nullptr;
            }
        }

        renderer::Shader::ShaderHeader shaderHeader;
        resource::ResourceHeader::fill(&shaderHeader, name, resourceSpirvBinary->size(), 0);

        Resource* resource = V3D_NEW(renderer::Shader, memory::MemoryLabel::MemoryObject)(shaderHeader);
        if (!resource->load(resourceSpirvBinary))
        {
            LOG_ERROR("ShaderSpirVDecoder::decode: shader load is failed");

            V3D_DELETE(resource, memory::MemoryLabel::MemoryObject);
            resource = nullptr;
        }
        stream::StreamManager::destroyStream(resourceSpirvBinary);

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("ShaderSpirVDecoder::decode, shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return resource;
    }
    else //Bytecode
    {
#if DEBUG
        utils::Timer timer;
        timer.start();
#endif
        bool reflections = !(flags & ShaderCompileFlag::ShaderCompile_DontUseReflection);

        std::vector<u32> bytecode(stream->size() / sizeof(u32));
        stream->read(bytecode.data(), sizeof(u32), static_cast<u32>(bytecode.size()));
        ASSERT(bytecode[0] == g_SPIRVIndifier, "invalid spirv magic number in head");

        stream::Stream* resourceSpirvBinary = stream::StreamManager::createMemoryStream();

        resourceSpirvBinary->write<renderer::ShaderType>(shaderPolicy->_type);
        resourceSpirvBinary->write<renderer::ShaderModel>(shaderPolicy->_shaderModel);
        resourceSpirvBinary->write(shaderPolicy->_entryPoint);
        resourceSpirvBinary->write<u32>(stream->size());
        resourceSpirvBinary->write(bytecode.data(), stream->size());
        resourceSpirvBinary->write<bool>(reflections);

        if (reflections)
        {
            ShaderReflectionSpirV reflector(shaderPolicy->_shaderModel);
            if (!reflector.reflect({ bytecode.cbegin(), bytecode.cend() }, resourceSpirvBinary))
            {
                LOG_ERROR("ShaderSpirVDecoder::decode: parseReflections failed for shader: %s", name.c_str());
                stream::StreamManager::destroyStream(resourceSpirvBinary);

                return nullptr;
            }
        }

        renderer::Shader::ShaderHeader shaderHeader;
        resource::ResourceHeader::fill(&shaderHeader, name, resourceSpirvBinary->size(), 0);

        Resource* resource = V3D_NEW(renderer::Shader, memory::MemoryLabel::MemoryObject)(shaderHeader);
        if (!resource->load(resourceSpirvBinary))
        {
            LOG_ERROR("ShaderSpirVDecoder::decode: the shader loading is failed");

            V3D_DELETE(resource, memory::MemoryLabel::MemoryObject);
            resource = nullptr;
        }
        stream::StreamManager::destroyStream(resourceSpirvBinary);

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("ShaderSpirVDecoder::decode, the shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return resource;
    }
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
