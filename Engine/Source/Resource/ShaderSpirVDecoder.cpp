#include "ShaderSpirVDecoder.h"
#include "Stream/FileLoader.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#ifdef USE_SPIRV
#   include <shaderc/libshaderc/include/shaderc/shaderc.hpp>
#   include <SPIRV-Cross/spirv_glsl.hpp>
#endif // USE_SPIRV

namespace v3d
{
namespace resource
{

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

        if (m_header._contentType == renderer::ShaderHeader::ShaderResource::ShaderResource_Source)
        {
#if DEBUG
            utils::Timer timer;
            timer.start();
#endif
            shaderc::CompileOptions options;
            options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
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
                options.SetOptimizationLevel(shaderc_optimization_level_performance);
                break;
            }

#if (DEBUG & VULKAN_DEBUG)
            options.SetWarningsAsErrors();
#endif
            switch (m_header._shaderLang)
            {
            case renderer::ShaderHeader::ShaderLang::ShaderLang_GLSL:
                options.SetSourceLanguage(shaderc_source_language_glsl);
                break;

            case renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL:
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
            renderer::ShaderType type = renderer::ShaderType::ShaderType_Undefined;
            auto getShaderType = [&validShaderType, &type](const std::string& name) -> shaderc_shader_kind
            {
                std::string fileExtension = stream::FileLoader::getFileExtension(name);
                if (fileExtension == "vert" || fileExtension == "vs")
                {
                    validShaderType = true;
                    type = renderer::ShaderType::ShaderType_Vertex;
                    return shaderc_shader_kind::shaderc_vertex_shader;
                }
                else if (fileExtension == "frag" || fileExtension == "ps")
                {
                    validShaderType = true;
                    type = renderer::ShaderType::ShaderType_Fragment;
                    return shaderc_shader_kind::shaderc_fragment_shader;
                }
                else if (fileExtension == "geom" || fileExtension == "gs")
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
            shaderc::AssemblyCompilationResult assambleResult = compiler.CompileGlslToSpvAssembly(source, shaderType, "shader", options);
            ASSERT(compiler.IsValid(), "error");
            shaderc_compilation_status assambleStatus = assambleResult.GetCompilationStatus();
            ASSERT(assambleStatus == shaderc_compilation_status_success, "error");
            LOG_DEBUG("ASSEMBLE SPIRV:");
            std::string assambleSPIRV(assambleResult.cbegin(), assambleResult.cend());
            LOG_DEBUG("%s", assambleSPIRV.c_str());
#endif //(DEBUG & VULKAN_DEBUG)

            u32 size = (u32)(result.cend() - result.cbegin()) * sizeof(u32);
            stream::Stream* resourceSpirvBinary = stream::StreamManager::createMemoryStream(nullptr, size + sizeof(u32) + sizeof(bool));
            resourceSpirvBinary->write<u32>(size);
            resourceSpirvBinary->write(result.cbegin(), size);

            resourceSpirvBinary->write<bool>(m_reflections);
            if (!m_reflections)
            {
                if (!ShaderSpirVDecoder::parseReflections({ result.cbegin(), result.cend() }, resourceSpirvBinary))
                {
                    LOG_ERROR("ShaderSpirVDecoder::decode: parseReflections failed for shader: %s", name.c_str());
                    delete resourceSpirvBinary;

                    return nullptr;
                }
            }

#if DEBUG
            timer.stop();
            u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
            LOG_DEBUG("ShaderSpirVDecoder::decode , shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

            renderer::ShaderHeader* resourceHeader = new renderer::ShaderHeader(m_header);
            resourceHeader->_type = type;
            resourceHeader->_apiVersion = m_sourceVersion;
#if DEBUG
            resourceHeader->_debugName = name;
#endif

            Resource* resource = new renderer::Shader(resourceHeader);
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
#ifdef USE_SPIRV
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
        }

        default:
            ASSERT(false, "format not found");
        }

        return renderer::Format_Undefined;
    };

    if (m_header._shaderLang == renderer::ShaderHeader::ShaderLang::ShaderLang_GLSL ||
        m_header._shaderLang == renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL)
    {
        spirv_cross::CompilerGLSL glsl(spirv);
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        const spirv_cross::CompilerGLSL::Options& options = glsl.get_common_options();
        m_sourceVersion = options.version;

        u32 inputChannelCount = static_cast<u32>(resources.stage_inputs.size());
        stream->write<u32>(inputChannelCount);
        for (auto& inputChannel : resources.stage_inputs)
        {
#if USE_STRING_ID_SHADER
            const std::string& name = glsl.get_name(inputChannel.id);
            ASSERT(!name.empty(), "empty name");
#endif
            u32 location = glsl.get_decoration(inputChannel.id, spv::DecorationLocation);

            const spirv_cross::SPIRType& type = glsl.get_type(inputChannel.type_id);

            renderer::Shader::Attribute input;
            input._location = location;
            input._format = convertSPRIVTypeToFormat(type);
#if USE_STRING_ID_SHADER
            input._name = name;
#endif
            input >> stream;
        }

        u32 outputChannelCount = static_cast<u32>(resources.stage_outputs.size());
        stream->write<u32>(outputChannelCount);
        for (auto& outputChannel : resources.stage_outputs)
        {
#if USE_STRING_ID_SHADER
            const std::string& name = glsl.get_name(outputChannel.id);
            ASSERT(!name.empty(), "empty name");
#endif
            u32 location = glsl.get_decoration(outputChannel.id, spv::DecorationLocation);

            const spirv_cross::SPIRType& type = glsl.get_type(outputChannel.type_id);
            u32 col = type.columns;
            u32 row = type.vecsize;

            renderer::Shader::Attribute output;
            output._location = location;
            output._format = convertSPRIVTypeToFormat(type);
#if USE_STRING_ID_SHADER
            output._name = name;
#endif
            output >> stream;
        }

        u32 unifromBufferCount = static_cast<u32>(resources.uniform_buffers.size());
        stream->write<u32>(unifromBufferCount);
        s32 buffID = 0;
        for (auto& buffer : resources.uniform_buffers)
        {
#if USE_STRING_ID_SHADER
            std::string name = glsl.get_name(buffer.id);
            if (m_header._shaderLang == renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL && name.empty())
            {
                name = "cbuffer_" + std::to_string(buffID);
            }
            else
            {
                ASSERT(!name.empty(), "empty name");
            }
#endif
            u32 binding = glsl.get_decoration(buffer.id, spv::DecorationBinding);
            u32 set = glsl.get_decoration(buffer.id, spv::DecorationDescriptorSet);
            const spirv_cross::SPIRType& block_type = glsl.get_type(buffer.type_id);

            renderer::Shader::UniformBuffer block;
            block._id = buffID;
            block._set = set;
            block._binding = binding;
            block._array = block_type.array.empty() ? 1 : block_type.array[0];
#if USE_STRING_ID_SHADER
            block._name = name;
#endif
            u32 posMembers = stream->tell();
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


            while (true)
            {
#if USE_STRING_ID_SHADER
                const std::string& member_name = glsl.get_member_name(buffer.base_type_id, index);
                if (member_name.empty())
                {
                    break;
                }
#endif
                const spirv_cross::SPIRType& type = glsl.get_type(block_type.member_types[index]);
                ++index;
                u32 col = type.columns;
                u32 row = type.vecsize;

                renderer::Shader::UniformBuffer::Uniform uniform;
                uniform._bufferId = buffID;
                uniform._array = type.array.empty() ? 1 : type.array[0];
                uniform._type = convertSPRIVTypeToDataType(type);
#if USE_STRING_ID_SHADER
                uniform._name = member_name;
#endif
                if (uniform._type == renderer::DataType::DataType_Struct)
                {
                    uniform._size = calculateMemberSizeInsideStruct(type);
                }
                else
                {
                    u32 dataSize = (type.width == 32) ? 4 : 8;
                    uniform._size = dataSize * col * row;
                }

                block._uniforms.push_back(uniform);
                membersSize += uniform._size * uniform._array;
            }

            block._size = membersSize;
            block >> stream;

            u32 posCurr = stream->tell();
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

        u32 samplersCount = static_cast<u32>(resources.sampled_images.size());
        stream->write<u32>(samplersCount);
        for (auto& image : resources.sampled_images)
        {
#if USE_STRING_ID_SHADER
            const std::string& name = glsl.get_name(image.id);
            ASSERT(!name.empty(), "empty name");
#endif
            u32 binding = glsl.get_decoration(image.id, spv::DecorationBinding);
            u32 set = glsl.get_decoration(image.id, spv::DecorationDescriptorSet);

            const spirv_cross::SPIRType& type = glsl.get_type(image.type_id);
            bool depth = type.image.depth;

            renderer::Shader::SampledImage image;
            image._set = set;
            image._binding = binding;
            image._target = convertSPRIVTypeToTextureData(type);
            image._array = type.array.empty() ? 1 : type.array[0];
            image._ms = type.image.ms;
            image._depth = depth;
#if USE_STRING_ID_SHADER
            image._name = name;
#endif
            image >> stream;
        }

        u32 pushConstantCount = static_cast<u32>(resources.push_constant_buffers.size());
        stream->write<u32>(pushConstantCount);
        for (auto& pushConstant : resources.push_constant_buffers)
        {
#if USE_STRING_ID_SHADER
            const std::string& name = glsl.get_name(pushConstant.id);
            ASSERT(!name.empty(), "empty name");
#endif
            u32 offset = glsl.get_decoration(pushConstant.id, spv::DecorationOffset);

            const spirv_cross::SPIRType& type = glsl.get_type(pushConstant.type_id);
            bool depth = type.image.depth;

            renderer::Shader::PushConstant constant;
            constant._offset = offset;
            constant._size = type.width; //TODO check
#if USE_STRING_ID_SHADER
            constant._name = name;
#endif
            constant >> stream;
        }

        return true;
    }
    else
    {
        ASSERT(false, "not implemented");
        return false;
    }
#endif //USE_SPIRV
    ASSERT(false, "not implemented");
    return false;
}

} //namespace resource
} //namespace v3d
