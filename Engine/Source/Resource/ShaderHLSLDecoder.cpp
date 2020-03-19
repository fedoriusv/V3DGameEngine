#include "ShaderHLSLDecoder.h"
#include "Stream/FileLoader.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#ifdef D3D_RENDER
#   include <d3dcompiler.h>

#include "Renderer/D3D12/D3D12Debug.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace v3d
{
namespace resource
{

bool reflect(ID3DBlob* shader, stream::Stream* stream);
void reflectConstantBuffers(ID3D11ShaderReflection* reflector, const std::vector<D3D11_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream);
//void reflectTextures(ID3D11ShaderReflection* reflector, const std::vector<D3D11_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream);

ShaderHLSLDecoder::ShaderHLSLDecoder(const renderer::ShaderHeader& header, bool reflections) noexcept
    : m_header(header)
    , m_reflections(reflections)
{
}

ShaderHLSLDecoder::ShaderHLSLDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, bool reflections) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_reflections(reflections)
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
                if (fileExtension == "vs")
                {
                    return renderer::ShaderType::ShaderType_Vertex;
                }
                else if (fileExtension == "ps")
                {
                    return renderer::ShaderType::ShaderType_Fragment;
                }
                
                return renderer::ShaderType::ShaderType_Undefined;
            };

            renderer::ShaderType type = m_header._type == renderer::ShaderType::ShaderType_Undefined ? getShaderTypeFromName(name) : m_header._type;

            std::string shaderVersion = "";
            auto getShaderType = [&shaderVersion](renderer::ShaderType type, u32 api) -> bool
            {
                switch (type)
                {
                case renderer::ShaderType::ShaderType_Vertex:
                {
                    shaderVersion = "vs_5_0";
                    return true;
                }

                case renderer::ShaderType::ShaderType_Fragment:
                {
                    shaderVersion = "ps_5_0";
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

            if (!getShaderType(type, m_header._apiVersion))
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
                compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
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
                    LOG_ERROR("ShaderHLSLDecoder::decode D3DCompile2 is failed. Error %s", renderer::d3d12::D3DDebug::stringError(result).c_str());
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
            LOG_DEBUG("ShaderHLSLDecoder::decode, shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
            LOG_DEBUG("Shader[%s]: %s\n", name.c_str(), source.c_str());
#endif

            renderer::ShaderHeader* resourceHeader = new renderer::ShaderHeader(m_header);
            resourceHeader->_apiVersion = 50;
            resourceHeader->_type = type;
#if DEBUG
            resourceHeader->_debugName = name;
#endif
            stream::MemoryStream* resourceBinary = stream::StreamManager::createMemoryStream(shader->GetBufferPointer(), static_cast<u32>(shader->GetBufferSize()));
            resourceBinary->write(static_cast<u32>(m_reflections));

            if (m_reflections)
            {
                if (!reflect(shader, resourceBinary))
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

bool reflect(ID3DBlob* shader, stream::Stream* stream)
{
    ID3D11ShaderReflection* reflector = nullptr;
    {
        HRESULT result = D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_PPV_ARGS(&reflector));//IID_ID3D11ShaderReflection, &reflector);
        if (FAILED(result))
        {
            LOG_ERROR("ShaderHLSLDecoder::reflect D3DReflect is failed. Error %s", renderer::d3d12::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

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
            input._name = name;
#endif
            input >> stream;
        }
    }

    //output parameters
    {
        u32 outputChannelCount = shaderDesc.OutputParameters;
        stream->write<u32>(outputChannelCount);

        for (UINT outputChannelID = 0; outputChannelID < outputChannelCount; ++outputChannelID)
        {
            D3D11_SIGNATURE_PARAMETER_DESC parameterDesc = {};
            HRESULT result = reflector->GetOutputParameterDesc(outputChannelID, &parameterDesc);
            ASSERT(SUCCEEDED(result), "GetOutputParameterDesc has failed");

            renderer::Shader::Attribute output;
            output._location = parameterDesc.Register;
            output._format = convertTypeToFormat(parameterDesc.ComponentType, parameterDesc.Mask);
#if USE_STRING_ID_SHADER
            const std::string name(parameterDesc.SemanticName);
            ASSERT(!name.empty(), "empty name");
            output._name = name;
#endif
            output >> stream;
        }
    }

    std::array<std::vector<D3D11_SHADER_INPUT_BIND_DESC>, D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER + 1> bindDescs;
    for (UINT resourceID = 0; resourceID < shaderDesc.BoundResources; ++resourceID)
    {
        D3D11_SHADER_INPUT_BIND_DESC bindDesc = {};
        HRESULT result = reflector->GetResourceBindingDesc(resourceID, &bindDesc);
        ASSERT(SUCCEEDED(result), "GetResourceBindingDesc has failed");

        bindDescs[bindDesc.Type].push_back(bindDesc);

    }

    reflectConstantBuffers(reflector, bindDescs[D3D_SIT_CBUFFER], stream);


    return true;
}

void reflectConstantBuffers(ID3D11ShaderReflection* reflector, const std::vector<D3D11_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream)
{
    D3D11_SHADER_DESC shaderDesc = {};
    HRESULT result = reflector->GetDesc(&shaderDesc);
    if (FAILED(result))
    {
        ASSERT(false, "reflector is failed");
        return;
    }

    auto convertTypeToDataType = [](const D3D11_SHADER_TYPE_DESC& desc) -> renderer::DataType
    {
        u32 col = desc.Columns;
        u32 row = desc.Rows;

        if (desc.Class == D3D_SVC_SCALAR) //value
        {
            switch (desc.Type)
            {
            case D3D_SVT_BOOL:
            case D3D_SVT_INT:
            case D3D_SVT_UINT8:
            case D3D_SVT_UINT:
            case D3D_SVT_MIN12INT:
            case D3D_SVT_MIN16INT:
            case D3D_SVT_MIN16UINT:
                return renderer::DataType::DataType_Int;

            case D3D_SVT_FLOAT:
            case D3D_SVT_MIN8FLOAT:
            case D3D_SVT_MIN10FLOAT:
            case D3D_SVT_MIN16FLOAT:
                return renderer::DataType::DataType_Float;

            case D3D_SVT_DOUBLE:
                return renderer::DataType::DataType_Double;

            case D3D_SVT_STRUCTURED_BUFFER:
                return renderer::DataType::DataType_Struct;

            default:
                break;
            }

            ASSERT(false, "not support");
            return  renderer::DataType::DataType_None;
        }
        else if (desc.Class == D3D_SVC_VECTOR) //vector
        {
            switch (desc.Type)
            {
            case D3D_SVT_FLOAT:
            case D3D_SVT_MIN8FLOAT:
            case D3D_SVT_MIN10FLOAT:
            case D3D_SVT_MIN16FLOAT:
            {
                if (col == 2)
                {
                    return renderer::DataType::DataType_Vector2;
                }
                else if (col == 3)
                {
                    return renderer::DataType::DataType_Vector3;
                }
                else if (col == 4)
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
        else if (desc.Class == D3D_SVC_MATRIX_ROWS || desc.Class == D3D_SVC_MATRIX_COLUMNS) //matrix
        {
            switch (desc.Type)
            {
            case D3D_SVT_FLOAT:
            case D3D_SVT_MIN8FLOAT:
            case D3D_SVT_MIN10FLOAT:
            case D3D_SVT_MIN16FLOAT:
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

    u32 unifromBufferCount = shaderDesc.ConstantBuffers;
    stream->write<u32>(unifromBufferCount);

    for (UINT constantBufferID = 0; constantBufferID < unifromBufferCount; ++constantBufferID)
    {
        ID3D11ShaderReflectionConstantBuffer* buffer = reflector->GetConstantBufferByIndex(constantBufferID);
        ASSERT(buffer, "ID3D11ShaderReflectionConstantBuffer is nullptr");

        D3D11_SHADER_BUFFER_DESC bufferDesc = {};
        {
            HRESULT result = buffer->GetDesc(&bufferDesc);
            ASSERT(SUCCEEDED(result), "GetDesc has failed");
        }

        renderer::Shader::UniformBuffer constantBuffer;
        constantBuffer._id = constantBufferID;
        constantBuffer._set = 0;
        constantBuffer._binding = bindDescs[constantBufferID].BindPoint;
        constantBuffer._array = 1;
        constantBuffer._size = bufferDesc.Size;
#if USE_STRING_ID_SHADER
        constantBuffer._name = std::string(bufferDesc.Name);
#endif
        for (UINT var = 0; var < bufferDesc.Variables; ++var)
        {
            ID3D11ShaderReflectionVariable* varable = buffer->GetVariableByIndex(var);
            ASSERT(varable, "ID3D11ShaderReflectionVariable is nullptr");
            D3D11_SHADER_VARIABLE_DESC variableDesc = {};
            {
                HRESULT result = varable->GetDesc(&variableDesc);
                ASSERT(SUCCEEDED(result), "GetDesc has failed");
            }

            ID3D11ShaderReflectionType* type = varable->GetType();
            ASSERT(type, "ID3D11ShaderReflectionType is nullptr");
            D3D11_SHADER_TYPE_DESC typeDesc = {};
            {
                HRESULT result = type->GetDesc(&typeDesc);
                ASSERT(SUCCEEDED(result), "GetDesc has failed");
            }

            ASSERT(typeDesc.Members == 0, "includeed structure is not supported");

            renderer::Shader::UniformBuffer::Uniform uniform;
            uniform._bufferId = constantBufferID;
            uniform._type = convertTypeToDataType(typeDesc);
            uniform._array = typeDesc.Elements + 1;
            uniform._size = variableDesc.Size;
#if USE_STRING_ID_SHADER
            uniform._name = std::string(variableDesc.Name);
#endif

            constantBuffer._uniforms.push_back(uniform);
        }

        constantBuffer >> stream;
    }
}

//void reflectTextures(ID3D11ShaderReflection* reflector, const D3D11_SHADER_INPUT_BIND_DESC& bindDesc, stream::Stream* stream)
//{
//    D3D11_SHADER_DESC shaderDesc = {};
//    HRESULT result = reflector->GetDesc(&shaderDesc);
//    if (FAILED(result))
//    {
//        ASSERT(false, "reflector is failed");
//        return;
//    }
//
//    //renderer::Shader::SampledImage texture;
//    //texture._binding = bindDesc.BindPoint;
//    //constantBuffer._set = 0;
//    //constantBuffer._binding = bindDesc.BindPoint;
//    //constantBuffer._array = 1;
//    //constantBuffer._size = bufferDesc.Size;
//
//}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
