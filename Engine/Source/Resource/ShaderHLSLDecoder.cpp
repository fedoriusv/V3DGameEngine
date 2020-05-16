#include "ShaderHLSLDecoder.h"
#include "Stream/FileLoader.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#ifdef D3D_RENDER
#include <d3dcompiler.h>
#include "Renderer/D3D12/D3DDebug.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace v3d
{
namespace resource
{

const std::map<std::string, renderer::ShaderType> k_HLSL_ExtensionList =
{
    //hlsl
    { "vs", renderer::ShaderType::ShaderType_Vertex },
    { "ps", renderer::ShaderType::ShaderType_Fragment },
};

bool reflect(ID3DBlob* shader, stream::Stream* stream, u32 version);
void reflectConstantBuffers(ID3D11ShaderReflection* reflector, const std::vector<D3D11_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream, u32 version);

ShaderHLSLDecoder::ShaderHLSLDecoder(const renderer::ShaderHeader& header, bool reflections) noexcept
    : m_header(header)
    , m_reflections(reflections)
    , m_version(0)
{
}

ShaderHLSLDecoder::ShaderHLSLDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_reflections(reflections)
    , m_version(0)
{
}

ShaderHLSLDecoder::~ShaderHLSLDecoder()
{
}

Resource * ShaderHLSLDecoder::decode(const stream::Stream* stream, const std::string& name)
{
    if (stream->size() > 0)
    {
        stream->seekBeg(0);

        if (m_header._shaderLang != renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL)
        {
            LOG_ERROR("ShaderHLSLDecoder::decode support only HLSL language");
            return nullptr;
        }

        if (m_header._contentType == renderer::ShaderHeader::ShaderResource::ShaderResource_Source)
        {
            std::string source;
            source.resize(stream->size());
            stream->read(source.data(), stream->size());
#if DEBUG
            utils::Timer timer;
            timer.start();
#endif
            auto getShaderTypeFromName = [](const std::string& name) -> renderer::ShaderType
            {
                std::string fileExtension = stream::FileLoader::getFileExtension(name);
                auto result = k_HLSL_ExtensionList.find(fileExtension);
                if (result == k_HLSL_ExtensionList.cend())
                {
                    return renderer::ShaderType::ShaderType_Undefined;
                }

                switch (result->second)
                {
                case renderer::ShaderType::ShaderType_Vertex:
                    return renderer::ShaderType::ShaderType_Vertex;

                case renderer::ShaderType::ShaderType_Fragment:
                    return renderer::ShaderType::ShaderType_Fragment;
                }
                
                return renderer::ShaderType::ShaderType_Undefined;
            };

            renderer::ShaderType type = m_header._type == renderer::ShaderType::ShaderType_Undefined ? getShaderTypeFromName(name) : m_header._type;

            std::string shaderVersion = "";
            renderer::ShaderHeader::ShaderModel shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_Default;
            auto getShaderType = [&shaderVersion, &shaderModel](renderer::ShaderType type, renderer::ShaderHeader::ShaderModel model) -> bool
            {
                switch (type)
                {
                case renderer::ShaderType::ShaderType_Vertex:
                {
                    if (model == renderer::ShaderHeader::ShaderModel::ShaderModel_Default ||
                        model == renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1)
                    {
                        shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
                        shaderVersion = "vs_5_1";
                    }
                    else
                    {
                        ASSERT(model >= renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0, "min version ShaderModer5.0");
                        shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0;
                        shaderVersion = "vs_5_0";
                    }
                    return true;
                }

                case renderer::ShaderType::ShaderType_Fragment:
                {
                    if (model == renderer::ShaderHeader::ShaderModel::ShaderModel_Default ||
                        model == renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1)
                    {
                        shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
                        shaderVersion = "ps_5_1";
                    }
                    else
                    {
                        ASSERT(model >= renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0, "min version ShaderModer5.0");
                        shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0;
                        shaderVersion = "ps_5_0";
                    }
                    return true;
                }

                default:
                {
                    shaderVersion = "xs_x_x";
                    return false;
                }
                };

                return false;
            };

            if (!getShaderType(type, m_header._shaderVersion))
            {
                LOG_ERROR("ShaderHLSLDecoder::decode wrong version: %s", shaderVersion.c_str());
                return nullptr;
            }

            std::vector<D3D_SHADER_MACRO> macros(m_header._defines.size());
            u32 index = 0;
            for (auto& def : m_header._defines)
            {
                macros[index].Name = def.first.c_str();
                macros[index].Definition = def.second.c_str();
            }

            UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL1;
            if (m_header._optLevel == 0)
            {
                compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
            }
            else if (m_header._optLevel == 1)
            {
                compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
            }
            else if (m_header._optLevel == 2)
            {
                compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
            }
#ifndef DEBUG
            compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

            LOG_DEBUG("Shader[%s]:\n %s\n", name.c_str(), source.c_str());
            ID3DBlob* shader = nullptr;
            {
                ID3DBlob* debugInfo = nullptr;
                HRESULT result = D3DCompile2(source.c_str(), source.size(), m_header._debugName.c_str(), nullptr, nullptr, m_header._entryPoint.c_str(), shaderVersion.c_str(), compileFlags, 0, 0, nullptr, 0, &shader, &debugInfo);
                if (debugInfo)
                {
                    std::string error(reinterpret_cast<c8*>(debugInfo->GetBufferSize(), debugInfo->GetBufferPointer()));
                    LOG_ERROR("Info: \n %s", error.c_str());

                    debugInfo->Release();
                }

                if (FAILED(result))
                {
                    LOG_ERROR("ShaderHLSLDecoder::decode D3DCompile2 is failed. Error %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
                    if (shader)
                    {
                        shader->Release();
                    }

                    return nullptr;
                }
            }

#if DEBUG
            timer.stop();
            u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
            LOG_DEBUG("ShaderHLSLDecoder::decode, shader %s is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

            renderer::ShaderHeader* resourceHeader = new renderer::ShaderHeader(m_header);
            resourceHeader->_type = type;
            resourceHeader->_shaderVersion = shaderModel;
#if DEBUG
            resourceHeader->_debugName = name;
#endif
            u32 bytecodeSize = static_cast<u32>(shader->GetBufferSize());
            stream::Stream* resourceBinary = stream::StreamManager::createMemoryStream(nullptr, bytecodeSize + sizeof(u32) + sizeof(bool));
            resourceBinary->write<u32>(bytecodeSize);
            resourceBinary->write(shader->GetBufferPointer(), bytecodeSize);

            resourceBinary->write<bool>(m_reflections);

            m_version = (shaderModel == renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1) ? 51 : 50;
            if (m_reflections)
            {
                if (!reflect(shader, resourceBinary, m_version))
                {
                    LOG_ERROR("ShaderHLSLDecoder::decode relect parse is failed. Shader %s", name.c_str());

                    shader->Release();

                    delete resourceHeader;
                    delete resourceBinary;

                    return nullptr;
                }
            }

            Resource* resource = new renderer::Shader(resourceHeader);
            resource->init(resourceBinary);

            return resource;
        }
        else
        {
#if DEBUG
            utils::Timer timer;
            timer.start();
#endif
            //TODO
#if DEBUG
            timer.stop();
            u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
            LOG_DEBUG("ShaderSpirVDecoder::decode , shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif
            ASSERT(false, "not impl");
            return nullptr;
        }
    }

    ASSERT(false, "spirv undefined");
    return nullptr;
}

bool reflect(ID3DBlob* shader, stream::Stream* stream, u32 version)
{
    ID3D11ShaderReflection* reflector = nullptr;
    {
        HRESULT result = D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_PPV_ARGS(&reflector));//IID_ID3D11ShaderReflection, &reflector);
        if (FAILED(result))
        {
            LOG_ERROR("ShaderHLSLDecoder::reflect D3DReflect is failed. Error %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }
    u64 requiresFlags = reflector->GetRequiresFlags();

    auto convertTypeToFormat = [](const D3D_REGISTER_COMPONENT_TYPE type, u8 componentsMask) -> renderer::Format
    {
        switch (type)
        {
        case D3D_REGISTER_COMPONENT_FLOAT32:
        {
            if (componentsMask == 15)
                return renderer::Format_R32G32B32A32_SFloat;
            if (componentsMask == 7)
                return renderer::Format_R32G32B32_SFloat;
            if (componentsMask == 3)
                return renderer::Format_R32G32_SFloat;
            if (componentsMask == 1)
                return renderer::Format_R32_SFloat;
        }
        break;

        case D3D_REGISTER_COMPONENT_SINT32:
        {
            if (componentsMask == 15)
                return renderer::Format_R32G32B32A32_SInt;
            if (componentsMask == 7)
                return renderer::Format_R32G32B32_SInt;
            if (componentsMask == 3)
                return renderer::Format_R32G32_SInt;
            if (componentsMask == 1)
                return renderer::Format_R32_SInt;
        }
        break;

        case D3D_REGISTER_COMPONENT_UINT32:
        {
            if (componentsMask == 15)
                return renderer::Format_R32G32B32A32_UInt;
            if (componentsMask == 7)
                return renderer::Format_R32G32B32_UInt;
            if (componentsMask == 3)
                return renderer::Format_R32G32_UInt;
            if (componentsMask == 1)
                return renderer::Format_R32_UInt;
        }
        break;

        default:
            ASSERT(false, "format not found");
        }

        return renderer::Format_Undefined;
    };


    D3D11_SHADER_DESC shaderDesc = {};
    HRESULT result = reflector->GetDesc(&shaderDesc);
    if (FAILED(result))
    {
        reflector->Release();
        return false;
    }

    //input parameters
    {
        u32 inputChannelCount = shaderDesc.InputParameters;
        stream->write<u32>(inputChannelCount);

        for (UINT inputChannelID = 0; inputChannelID < inputChannelCount; ++inputChannelID)
        {
            D3D11_SIGNATURE_PARAMETER_DESC parameterDesc = {};
            HRESULT result = reflector->GetInputParameterDesc(inputChannelID, &parameterDesc);
            ASSERT(SUCCEEDED(result), "GetInputParameterDesc has failed");

            renderer::Shader::Attribute input;
            input._location = parameterDesc.Register;
            input._format = convertTypeToFormat(parameterDesc.ComponentType, parameterDesc.Mask);
#if USE_STRING_ID_SHADER
            const std::string name(parameterDesc.SemanticName);
            ASSERT(!name.empty(), "empty name");
            input._name = name + std::to_string(parameterDesc.SemanticIndex);
#endif
            input >> stream;
        }
    }

    //output parameters
    {
        u32 outputChannelCount = shaderDesc.OutputParameters;
        std::vector<D3D11_SIGNATURE_PARAMETER_DESC> userOutputChannels;
        for (UINT outputChannelID = 0; outputChannelID < outputChannelCount; ++outputChannelID)
        {
            D3D11_SIGNATURE_PARAMETER_DESC parameterDesc = {};
            HRESULT result = reflector->GetOutputParameterDesc(outputChannelID, &parameterDesc);
            ASSERT(SUCCEEDED(result), "GetOutputParameterDesc has failed");

            if (parameterDesc.SystemValueType == D3D_NAME_UNDEFINED || parameterDesc.SystemValueType == D3D_NAME_TARGET) //remove build-in
            {
                userOutputChannels.push_back(parameterDesc);
            }
        }

        u32 userOutputChannelCount = static_cast<u32>(userOutputChannels.size());
        stream->write<u32>(userOutputChannelCount);
        for (UINT outputChannelID = 0; outputChannelID < userOutputChannelCount; ++outputChannelID)
        {
            renderer::Shader::Attribute output;
            output._location = outputChannelID;//parameterDesc.SemanticIndex;
            output._format = convertTypeToFormat(userOutputChannels[outputChannelID].ComponentType, userOutputChannels[outputChannelID].Mask);
#if USE_STRING_ID_SHADER
            const std::string name(userOutputChannels[outputChannelID].SemanticName);
            ASSERT(!name.empty(), "empty name");
            output._name = name + std::to_string(userOutputChannels[outputChannelID].SemanticIndex);
#endif
            output >> stream;
        }
    }

    const u32 countResources = D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER + 1;
    std::array<std::vector<D3D11_SHADER_INPUT_BIND_DESC>, countResources> bindDescs;
    for (UINT resourceID = 0; resourceID < shaderDesc.BoundResources; ++resourceID)
    {
        D3D11_SHADER_INPUT_BIND_DESC bindDesc = {};
        HRESULT result = reflector->GetResourceBindingDesc(resourceID, &bindDesc);
        ASSERT(SUCCEEDED(result), "GetResourceBindingDesc has failed");

        bindDescs[bindDesc.Type].push_back(bindDesc);

    }

    reflectConstantBuffers(reflector, bindDescs[D3D_SIT_CBUFFER], stream, version);

    {
        //dx is not supported
        u32 sampledImagesCount = static_cast<u32>(0);
        stream->write<u32>(sampledImagesCount);
    }

    {
        bool ms = false;
        auto convertDXTypeToTextureTarget = [&ms](const D3D_SRV_DIMENSION& dim) ->renderer::TextureTarget
        {
            switch (dim)
            {
            case D3D_SRV_DIMENSION_TEXTURE1D:
                return renderer::TextureTarget::Texture1D;

            case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
                return renderer::TextureTarget::Texture1DArray;

            case D3D_SRV_DIMENSION_TEXTURE2D:
            case D3D_SRV_DIMENSION_TEXTURE2DMS:
                ms = (dim == D3D_SRV_DIMENSION_TEXTURE2DMS) ? true : false;
                return renderer::TextureTarget::Texture2D;

            case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
            case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
                ms = (dim == D3D_SRV_DIMENSION_TEXTURE2DMSARRAY) ? true : false;
                return renderer::TextureTarget::Texture2DArray;

            case D3D_SRV_DIMENSION_TEXTURE3D:
                return renderer::TextureTarget::Texture3D;

            case D3D_SRV_DIMENSION_TEXTURECUBE:
                return renderer::TextureTarget::TextureCubeMap;

            case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
            case D3D_SRV_DIMENSION_UNKNOWN:
            default:
                ASSERT(false, "not found");
                return renderer::TextureTarget::Texture2D;
            }
        };

        const std::vector<D3D11_SHADER_INPUT_BIND_DESC>& boundTexturesDescs = bindDescs[D3D_SIT_TEXTURE];
        u32 imagesCount = static_cast<u32>(boundTexturesDescs.size());
        stream->write<u32>(imagesCount);

        u32 currentSpace = 0;
        std::vector<std::vector<u32>> textureTable;
        textureTable.resize(4); //k_maxDescriptorSetIndex

        for (u32 imageId = 0; imageId < imagesCount; ++imageId)
        {
            u32 currentBinding = boundTexturesDescs[imageId].BindPoint;
            auto found = std::find(textureTable[currentSpace].begin(), textureTable[currentSpace].end(), currentBinding);
            if (found != textureTable[currentSpace].end() || (!textureTable[currentSpace].empty() && currentBinding <= textureTable[currentSpace].back()))
            {
                ++currentSpace;
            }
            textureTable[currentSpace].push_back(currentBinding);

            renderer::Shader::Image sepImage;
            sepImage._set = currentSpace;
            sepImage._binding = currentBinding;

            sepImage._target = convertDXTypeToTextureTarget(boundTexturesDescs[imageId].Dimension);
            sepImage._array = boundTexturesDescs[imageId].BindCount;
            sepImage._ms = ms;
            sepImage._depth = false;
    #if USE_STRING_ID_SHADER
            sepImage._name = std::string(boundTexturesDescs[imageId].Name);
    #endif
            sepImage >> stream;
        }
    }

    {
        u32 currentSpace = 0;
        std::vector<std::vector<u32>> samplerTable;
        samplerTable.resize(4); //k_maxDescriptorSetIndex

        const std::vector<D3D11_SHADER_INPUT_BIND_DESC>& boundSamplersDescs = bindDescs[D3D_SIT_SAMPLER];
        u32 samplersCount = static_cast<u32>(boundSamplersDescs.size());
        stream->write<u32>(samplersCount);

        for (u32 samplerId = 0; samplerId < samplersCount; ++samplerId)
        {
            u32 currentBinding = boundSamplersDescs[samplerId].BindPoint;
            auto found = std::find(samplerTable[currentSpace].begin(), samplerTable[currentSpace].end(), currentBinding);
            if (found != samplerTable[currentSpace].end() || (!samplerTable[currentSpace].empty() && currentBinding <= samplerTable[currentSpace].back()))
            {
                ++currentSpace;
            }
            samplerTable[currentSpace].push_back(currentBinding);

            renderer::Shader::Sampler sampler;
            sampler._set = currentSpace;
            sampler._binding = currentBinding;
#if USE_STRING_ID_SHADER
            sampler._name = std::string(boundSamplersDescs[samplerId].Name);
#endif
            sampler >> stream;
        }
    }

    {
        u32 pushConstantCount = static_cast<u32>(0);
        stream->write<u32>(pushConstantCount);
    }

    return true;
}

void reflectConstantBuffers(ID3D11ShaderReflection* reflector, const std::vector<D3D11_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream, u32 version)
{
    D3D11_SHADER_DESC shaderDesc = {};
    HRESULT result = reflector->GetDesc(&shaderDesc);
    if (FAILED(result))
    {
        ASSERT(false, "reflector is failed");
        return;
    }

    auto convertTypeToDataType = [](const D3D11_SHADER_TYPE_DESC& desc) -> std::tuple<renderer::DataType, size_t>
    {
        u32 col = desc.Columns;
        u32 row = desc.Rows;

        if (desc.Class == D3D_SVC_SCALAR) //value
        {
            switch (desc.Type)
            {
            case D3D_SVT_INT:
            case D3D_SVT_UINT:
                return { renderer::DataType::DataType_Int, sizeof(s32) };

            case D3D_SVT_FLOAT:
                return { renderer::DataType::DataType_Float, sizeof(f32) };

            case D3D_SVT_DOUBLE:
                return { renderer::DataType::DataType_Double, sizeof(f64) };

            case D3D_SVT_STRUCTURED_BUFFER:
                return { renderer::DataType::DataType_Struct, 0 };

            default:
                break;
            }

            ASSERT(false, "not support");
            return  { renderer::DataType::DataType_None, 0 };
        }
        else if (desc.Class == D3D_SVC_VECTOR) //vector
        {
            switch (desc.Type)
            {
            case D3D_SVT_FLOAT:
            {
                u32 size = col * sizeof(f32);
                if (col == 2)
                {
                    return { renderer::DataType::DataType_Vector2, size };
                }
                else if (col == 3)
                {
                    return { renderer::DataType::DataType_Vector3, size };
                }
                else if (col == 4)
                {
                    return { renderer::DataType::DataType_Vector4, size };
                }
            }

            default:
                break;
            }

            ASSERT(false, "not support");
            return  { renderer::DataType::DataType_None, 0 };
        }
        else if (desc.Class == D3D_SVC_MATRIX_ROWS || desc.Class == D3D_SVC_MATRIX_COLUMNS) //matrix
        {
            switch (desc.Type)
            {
            case D3D_SVT_FLOAT:
            {
                u32 size = col * row * sizeof(f32);
                if (col == 3 && row == 3)
                {
                    return { renderer::DataType::DataType_Matrix3, size };
                }
                else if (col == 4 && row == 4)
                {
                    return { renderer::DataType::DataType_Matrix4, size };
                }
            }

            default:
                break;
            }

            ASSERT(false, "not support");
            return { renderer::DataType::DataType_None, 0 };
        }


        ASSERT(false, "not support");
        return { renderer::DataType::DataType_None, 0 };
    };

    u32 unifromBufferCount = shaderDesc.ConstantBuffers;
    stream->write<u32>(unifromBufferCount);

    std::map<ID3D11ShaderReflectionConstantBuffer*, u32> buffMap;
    for (UINT constantBufferID = 0; constantBufferID < unifromBufferCount; ++constantBufferID)
    {
        ID3D11ShaderReflectionConstantBuffer* buffer = reflector->GetConstantBufferByIndex(constantBufferID);
        ASSERT(buffer, "ID3D11ShaderReflectionConstantBuffer is nullptr");

        buffMap.emplace(buffer, constantBufferID);
    }

    u32 currentSpace = 0;
    std::vector<std::vector<u32>> bufferTable;
    bufferTable.resize(4); //k_maxDescriptorSetIndex

    for (UINT constantBufferID = 0; constantBufferID < unifromBufferCount; ++constantBufferID)
    {
        ID3D11ShaderReflectionConstantBuffer* buffer = reflector->GetConstantBufferByName(bindDescs[constantBufferID].Name);
        ASSERT(buffer, "ID3D11ShaderReflectionConstantBuffer is nullptr");

        D3D11_SHADER_BUFFER_DESC bufferDesc = {};
        {
            HRESULT result = buffer->GetDesc(&bufferDesc);
            ASSERT(SUCCEEDED(result), "GetDesc has failed");
        }

        ASSERT(version == 50 || (version == 51 && bufferDesc.Variables == 1) , "not supported");
        ID3D11ShaderReflectionVariable* structVariable = buffer->GetVariableByIndex(0);
        ASSERT(structVariable, "ID3D11ShaderReflectionVariable is nullptr");

        ID3D11ShaderReflectionType* structType = structVariable->GetType();
        ASSERT(structType, "ID3D11ShaderReflectionType is nullptr");
        D3D11_SHADER_TYPE_DESC structTypeDesc = {};
        {
            HRESULT result = structType->GetDesc(&structTypeDesc);
            ASSERT(SUCCEEDED(result), "GetDesc has failed");
        }
        ASSERT(version == 50 || (version == 51 && structTypeDesc.Class == D3D_SVC_STRUCT), "must be stucture");

        auto foundID = buffMap.find(buffer);
        ASSERT(foundID != buffMap.cend(), "not found");
        u32 bufferID = foundID->second;

        u32 currentBinding = bindDescs[constantBufferID].BindPoint;
        auto found = std::find(bufferTable[currentSpace].begin(), bufferTable[currentSpace].end(), currentBinding);
        if (found != bufferTable[currentSpace].end() || (!bufferTable[currentSpace].empty() && currentBinding <= bufferTable[currentSpace].back()))
        {
            ++currentSpace;
        }
        bufferTable[currentSpace].push_back(currentBinding);

        renderer::Shader::UniformBuffer constantBuffer;
        constantBuffer._id = bufferID;
        constantBuffer._set = currentSpace;
        constantBuffer._binding = currentBinding;
        constantBuffer._array = bufferDesc.Variables;
        constantBuffer._size = bufferDesc.Size;
#if USE_STRING_ID_SHADER
        constantBuffer._name = std::string(bufferDesc.Name);
#endif
        ID3D11ShaderReflectionConstantBuffer* structBuffer = structVariable->GetBuffer();
        ASSERT(structBuffer, "ID3D11ShaderReflectionConstantBuffer is nullptr");

        u32 offset = 0;
        for (u32 mem = 0; mem < structTypeDesc.Members; ++mem)
        {
            ID3D11ShaderReflectionType* memberType = structType->GetMemberTypeByIndex(mem);
            ASSERT(memberType, "ID3D11ShaderReflectionType is nullptr");
            D3D11_SHADER_TYPE_DESC memberDesc = {};
            {
                HRESULT result = memberType->GetDesc(&memberDesc);
                ASSERT(SUCCEEDED(result), "GetDesc has failed");
            }
            const auto [type, size] = convertTypeToDataType(memberDesc);

            renderer::Shader::UniformBuffer::Uniform uniform;
            uniform._bufferId = bufferID;
            uniform._type = type;
            uniform._array = (memberDesc.Elements == 0) ? 1 : memberDesc.Elements;
            uniform._size = static_cast<u32>(size) * uniform._array;
            uniform._offset = offset;
#if USE_STRING_ID_SHADER
            uniform._name = "var_" + std::to_string(mem);
#endif
            ASSERT(memberDesc.Members == 0, "not supported");
            offset += uniform._size;

            constantBuffer._uniforms.push_back(uniform);
        }

        constantBuffer >> stream;
    }
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
