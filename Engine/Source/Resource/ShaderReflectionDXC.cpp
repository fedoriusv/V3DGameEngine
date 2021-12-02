#include "ShaderReflectionDXC.h"
#include "Utils/Logger.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)
#include "Renderer/D3D12/D3DDebug.h"

#if USE_CUSTOM_DXC
#   include "dxc/inc/dxcapi.h"
#   include "dxc/inc/d3d12shader.h"
#else
#   if defined(PLATFORM_WINDOWS)
#       include <dxcapi.h>
#       include <d3d12shader.h>
#       pragma comment(lib, "dxcompiler.lib")
#   elif defined(PLATFORM_XBOX)
#       include <dxcapi_xs.h>
#       include <d3d12shader_xs.h>
#   endif //PLATFORM
#endif

namespace v3d
{
namespace resource
{

void reflectAttributes(ID3D12ShaderReflection* reflector, const D3D12_SHADER_DESC& shaderDesc, stream::Stream* stream);
void reflectConstantBuffers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream);
void reflectSampledImages(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundTexturesDescs, stream::Stream* stream);
void reflectSamplers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundSamplersDescs, stream::Stream* stream);

ShaderReflectionDXC::ShaderReflectionDXC() noexcept
{
}

ShaderReflectionDXC::~ShaderReflectionDXC()
{
}

bool ShaderReflectionDXC::reflect(const IDxcBlob* shader, stream::Stream* stream) const
{
    IDxcContainerReflection* DXContainerReflection = nullptr;
    {
        HRESULT result = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&DXContainerReflection));
        if (FAILED(result))
        {
            LOG_FATAL("ShaderDXCDecoder DxcCreateInstance can't create IDxcContainerReflection. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    {
        HRESULT result = DXContainerReflection->Load(const_cast<IDxcBlob*>(shader));
        if (FAILED(result))
        {
            LOG_FATAL("ShaderDXCDecoder DXContainerReflection Load is failed. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            DXContainerReflection->Release();

            return false;
        }
    }

    u32 partIndex = 0;
    {
#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d) (unsigned int)((unsigned char)(a) | (unsigned char)(b) << 8 | (unsigned char)(c) << 16 | (unsigned char)(d) << 24)
#endif
        HRESULT result = DXContainerReflection->FindFirstPartKind(MAKEFOURCC('D', 'X', 'I', 'L'), &partIndex);
        ASSERT(SUCCEEDED(result), "failed");
#undef MAKEFOURCC
    }

    ID3D12ShaderReflection* shaderReflection = nullptr;
    {
        HRESULT result = DXContainerReflection->GetPartReflection(partIndex, IID_PPV_ARGS(&shaderReflection));
        if (FAILED(result))
        {
            LOG_FATAL("ShaderDXCDecoder GetPartReflection is failed. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            if (shaderReflection)
            {
                shaderReflection->Release();
            }
            DXContainerReflection->Release();

            return false;
        }
    }

    D3D12_SHADER_DESC shaderDesc = {};
    {
        HRESULT result = shaderReflection->GetDesc(&shaderDesc);
        if (FAILED(result))
        {
            shaderReflection->Release();
            DXContainerReflection->Release();

            return false;
        }
    }

    reflectAttributes(shaderReflection, shaderDesc, stream);

    const u32 countResources = D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER + 1;
    std::array<std::vector<D3D12_SHADER_INPUT_BIND_DESC>, countResources> bindDescs;
    for (UINT resourceID = 0; resourceID < shaderDesc.BoundResources; ++resourceID)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc = {};
        HRESULT result = shaderReflection->GetResourceBindingDesc(resourceID, &bindDesc);
        ASSERT(SUCCEEDED(result), "GetResourceBindingDesc has failed");

        bindDescs[bindDesc.Type].push_back(bindDesc);

    }
    reflectConstantBuffers(shaderReflection, bindDescs[D3D_SIT_CBUFFER], stream);

    {
        //DX is not supported
        u32 sampledImagesCount = static_cast<u32>(0);
        stream->write<u32>(sampledImagesCount);
    }

    reflectSampledImages(shaderReflection, bindDescs[D3D_SIT_TEXTURE], stream);
    reflectSamplers(shaderReflection, bindDescs[D3D_SIT_SAMPLER], stream);

    {
        u32 countStorageImage = static_cast<u32>(0);
        stream->write<u32>(countStorageImage);
    }

    {
        u32 countStorageBuffers = static_cast<u32>(0);
        stream->write<u32>(countStorageBuffers);
    }

    {
        u32 pushConstantCount = static_cast<u32>(0);
        stream->write<u32>(pushConstantCount);
    }


    shaderReflection->Release();
    DXContainerReflection->Release();

    return true;
}

void reflectAttributes(ID3D12ShaderReflection* reflector, const D3D12_SHADER_DESC& shaderDesc, stream::Stream* stream)
{
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

    //input parameters
    {
        u32 inputChannelCount = shaderDesc.InputParameters;
        u32 inputPositionRegister = -1;
        std::vector<D3D12_SIGNATURE_PARAMETER_DESC> userInputChannels;
        for (UINT inputChannelID = 0; inputChannelID < inputChannelCount; ++inputChannelID)
        {
            D3D12_SIGNATURE_PARAMETER_DESC parameterDesc = {};
            HRESULT result = reflector->GetInputParameterDesc(inputChannelID, &parameterDesc);
            ASSERT(SUCCEEDED(result), "GetOutputParameterDesc has failed");

            if (parameterDesc.SystemValueType == D3D_NAME_POSITION)
            {
                inputPositionRegister = parameterDesc.Register;
            }

            if (parameterDesc.SystemValueType == D3D_NAME_UNDEFINED) //remove build-in position
            {
                userInputChannels.push_back(parameterDesc);
            }
        }

        u32 userInputChannelCount = static_cast<u32>(userInputChannels.size());
        stream->write<u32>(userInputChannelCount);
        for (UINT inputChannelID = 0; inputChannelID < userInputChannelCount; ++inputChannelID)
        {
            const D3D12_SIGNATURE_PARAMETER_DESC& parameterDesc = userInputChannels[inputChannelID];

            renderer::Shader::Attribute input;
            input._location = parameterDesc.Register;
            if (inputPositionRegister != -1 && inputPositionRegister < input._location) //remove register offset
            {
                input._location = input._location - 1;
            }
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
        u32 outputPositionRegister = -1;
        std::vector<D3D12_SIGNATURE_PARAMETER_DESC> userOutputChannels;
        for (UINT outputChannelID = 0; outputChannelID < outputChannelCount; ++outputChannelID)
        {
            D3D12_SIGNATURE_PARAMETER_DESC parameterDesc = {};
            HRESULT result = reflector->GetOutputParameterDesc(outputChannelID, &parameterDesc);
            ASSERT(SUCCEEDED(result), "GetOutputParameterDesc has failed");

            if (parameterDesc.SystemValueType == D3D_NAME_POSITION)
            {
                outputPositionRegister = parameterDesc.Register;
            }

            if (parameterDesc.SystemValueType == D3D_NAME_UNDEFINED || parameterDesc.SystemValueType == D3D_NAME_TARGET) //remove build-in
            {
                userOutputChannels.push_back(parameterDesc);
            }
        }

        u32 userOutputChannelCount = static_cast<u32>(userOutputChannels.size());
        stream->write<u32>(userOutputChannelCount);
        for (UINT outputChannelID = 0; outputChannelID < userOutputChannelCount; ++outputChannelID)
        {
            const D3D12_SIGNATURE_PARAMETER_DESC& parameterDesc = userOutputChannels[outputChannelID];

            renderer::Shader::Attribute output;
            output._location = parameterDesc.Register;
            if (outputPositionRegister != -1 && outputPositionRegister < output._location) //remove register offset
            {
                output._location = output._location - 1;
            }
            output._format = convertTypeToFormat(parameterDesc.ComponentType, parameterDesc.Mask);
#if USE_STRING_ID_SHADER
            const std::string name(parameterDesc.SemanticName);
            ASSERT(!name.empty(), "empty name");
            output._name = name + std::to_string(parameterDesc.SemanticIndex);
#endif
            output >> stream;
        }
    }
}

void reflectConstantBuffers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream)
{
    D3D12_SHADER_DESC shaderDesc = {};
    HRESULT result = reflector->GetDesc(&shaderDesc);
    if (FAILED(result))
    {
        ASSERT(false, "reflector is failed");
        return;
    }

    auto convertTypeToDataType = [](const D3D12_SHADER_TYPE_DESC& desc) -> std::tuple<renderer::DataType, size_t>
    {
        u32 col = desc.Columns;
        u32 row = desc.Rows;

        if (desc.Class == D3D_SVC_SCALAR) //value
        {
            switch (desc.Type)
            {
            case D3D_SVT_INT:
            case D3D_SVT_UINT:
            case D3D_SVT_BOOL:
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
            }

            ASSERT(false, "not support");
            return { renderer::DataType::DataType_None, 0 };
        }


        ASSERT(false, "not support");
        return { renderer::DataType::DataType_None, 0 };
    };

    u32 unifromBufferCount = shaderDesc.ConstantBuffers;
    stream->write<u32>(unifromBufferCount);

    std::map<ID3D12ShaderReflectionConstantBuffer*, u32> buffMap;
    for (UINT constantBufferID = 0; constantBufferID < unifromBufferCount; ++constantBufferID)
    {
        ID3D12ShaderReflectionConstantBuffer* buffer = reflector->GetConstantBufferByIndex(constantBufferID);
        ASSERT(buffer, "ID3D11ShaderReflectionConstantBuffer is nullptr");

        buffMap.emplace(buffer, constantBufferID);
    }

    u32 currentSpace = 0;
    std::vector<std::vector<u32>> bufferTable;
    bufferTable.resize(renderer::k_maxDescriptorSetIndex);

    for (UINT constantBufferID = 0; constantBufferID < unifromBufferCount; ++constantBufferID)
    {
        ID3D12ShaderReflectionConstantBuffer* buffer = reflector->GetConstantBufferByName(bindDescs[constantBufferID].Name);
        ASSERT(buffer, "ID3D11ShaderReflectionConstantBuffer is nullptr");

        D3D12_SHADER_BUFFER_DESC bufferDesc = {};
        {
            HRESULT result = buffer->GetDesc(&bufferDesc);
            ASSERT(SUCCEEDED(result), "GetDesc has failed");
        }

        ID3D12ShaderReflectionVariable* structVariable = buffer->GetVariableByIndex(0);
        ASSERT(structVariable, "ID3D11ShaderReflectionVariable is nullptr");

        ID3D12ShaderReflectionType* structType = structVariable->GetType();
        ASSERT(structType, "ID3D11ShaderReflectionType is nullptr");
        D3D12_SHADER_TYPE_DESC structTypeDesc = {};
        {
            HRESULT result = structType->GetDesc(&structTypeDesc);
            ASSERT(SUCCEEDED(result), "GetDesc has failed");
        }

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
        ID3D12ShaderReflectionConstantBuffer* structBuffer = structVariable->GetBuffer();
        ASSERT(structBuffer, "ID3D11ShaderReflectionConstantBuffer is nullptr");

        u32 offset = 0;
        for (u32 mem = 0; mem < structTypeDesc.Members; ++mem)
        {
            ID3D12ShaderReflectionType* memberType = structType->GetMemberTypeByIndex(mem);
            ASSERT(memberType, "ID3D11ShaderReflectionType is nullptr");
            D3D12_SHADER_TYPE_DESC memberDesc = {};
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

void reflectSampledImages(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundTexturesDescs, stream::Stream* stream)
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

    u32 imagesCount = static_cast<u32>(boundTexturesDescs.size());
    stream->write<u32>(imagesCount);

    u32 currentSpace = 0;
    std::vector<std::vector<u32>> textureTable;
    textureTable.resize(renderer::k_maxDescriptorSetIndex);

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

void reflectSamplers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundSamplersDescs, stream::Stream* stream)
{
    u32 currentSpace = 0;
    std::vector<std::vector<u32>> samplerTable;
    samplerTable.resize(renderer::k_maxDescriptorSetIndex);

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

} //namespace resource
} //namespace v3d
#endif //PLATFORM_WINDOWS