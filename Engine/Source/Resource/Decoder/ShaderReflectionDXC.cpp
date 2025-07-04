#include "ShaderReflectionDXC.h"
#include "Stream/Stream.h"
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

static void reflectAttributes(ID3D12ShaderReflection* reflector, const D3D12_SHADER_DESC& shaderDesc, stream::Stream* stream);
static void reflectConstantBuffers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream);
static void reflectSampledImages(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundTexturesDescs, stream::Stream* stream);
static void reflectSamplers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundSamplersDescs, stream::Stream* stream);
static void reflectUAVs(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundUAVsDescs, stream::Stream* stream);

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
    reflectUAVs(shaderReflection, bindDescs[D3D_SIT_UAV_RWTYPED], stream);

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

            const std::string name(parameterDesc.SemanticName);
            ASSERT(!name.empty(), "empty name");
            input._name = name + std::to_string(parameterDesc.SemanticIndex);

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

            const std::string name(parameterDesc.SemanticName);
            ASSERT(!name.empty(), "empty name");
            output._name = name + std::to_string(parameterDesc.SemanticIndex);

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
            case D3D_SVT_INT16:
                return { renderer::DataType::Int16, sizeof(s16) };
            case D3D_SVT_UINT16:
                return { renderer::DataType::UInt16, sizeof(u16) };
            case D3D_SVT_INT:
                return { renderer::DataType::Int32, sizeof(s32) };
            case D3D_SVT_UINT:
                return { renderer::DataType::UInt32, sizeof(u32) };
            case D3D_SVT_INT64:
                return { renderer::DataType::Int64, sizeof(s64) };
            case D3D_SVT_UINT64:
                return { renderer::DataType::UInt64, sizeof(u64) };
            case D3D_SVT_FLOAT16:
                return { renderer::DataType::Float16, 2/*sizeof(std::float16_t)*/ };
            case D3D_SVT_FLOAT:
                return { renderer::DataType::Float32, sizeof(f32) };
            case D3D_SVT_DOUBLE:
                return { renderer::DataType::Float64, sizeof(f64) };
            case D3D_SVT_STRUCTURED_BUFFER:
                return { renderer::DataType::Struct, 0 };

            default:
                break;
            }

            ASSERT(false, "not support");
            return  { renderer::DataType::None, 0 };
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
                    return { renderer::DataType::Vector2, size };
                }
                else if (col == 3)
                {
                    return { renderer::DataType::Vector3, size };
                }
                else if (col == 4)
                {
                    return { renderer::DataType::Vector4, size };
                }
            }
            }

            ASSERT(false, "not support");
            return  { renderer::DataType::None, 0 };
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
                    return { renderer::DataType::Matrix3, size };
                }
                else if (col == 4 && row == 4)
                {
                    return { renderer::DataType::Matrix4, size };
                }
            }
            }

            ASSERT(false, "not support");
            return { renderer::DataType::None, 0 };
        }


        ASSERT(false, "not support");
        return { renderer::DataType::None, 0 };
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

        u32 currentSpace = bindDescs[constantBufferID].Space;
        u32 currentRegister = bindDescs[constantBufferID].BindPoint;

        renderer::Shader::UniformBuffer constantBuffer;
        constantBuffer._id = bufferID;
        constantBuffer._set = currentSpace;
        constantBuffer._binding = currentRegister;
        constantBuffer._array = bufferDesc.Variables;
        constantBuffer._size = bufferDesc.Size;
        constantBuffer._name = std::string(bufferDesc.Name);

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
            uniform._bufferID = bufferID;
            uniform._type = type;
            uniform._array = (memberDesc.Elements == 0) ? 1 : memberDesc.Elements;
            uniform._size = static_cast<u32>(size) * uniform._array;
            uniform._offset = offset;
            uniform._name = "var_" + std::to_string(mem);

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

    for (u32 imageId = 0; imageId < imagesCount; ++imageId)
    {
        u32 currentSpace = boundTexturesDescs[imageId].Space;
        u32 currentRegister = boundTexturesDescs[imageId].BindPoint;

        renderer::Shader::Image sepImage;
        sepImage._set = currentSpace;
        sepImage._binding = currentRegister;

        sepImage._target = convertDXTypeToTextureTarget(boundTexturesDescs[imageId].Dimension);
        sepImage._array = boundTexturesDescs[imageId].BindCount;
        sepImage._ms = ms;
        sepImage._depth = false;
        sepImage._name = std::string(boundTexturesDescs[imageId].Name);

        sepImage >> stream;
    }
}

void reflectSamplers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundSamplersDescs, stream::Stream* stream)
{
    u32 samplersCount = static_cast<u32>(boundSamplersDescs.size());
    stream->write<u32>(samplersCount);

    for (u32 samplerId = 0; samplerId < samplersCount; ++samplerId)
    {
        u32 currentSpace = boundSamplersDescs[samplerId].Space;
        u32 currentRegister = boundSamplersDescs[samplerId].BindPoint;

        renderer::Shader::Sampler sampler;
        sampler._set = currentSpace;
        sampler._binding = currentRegister;
        sampler._name = std::string(boundSamplersDescs[samplerId].Name);

        sampler >> stream;
    }
}

void reflectUAVs(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundUAVsDescs, stream::Stream* stream)
{
    std::vector<D3D12_SHADER_INPUT_BIND_DESC> UAVImageDescs;
    std::vector<D3D12_SHADER_INPUT_BIND_DESC> UAVBufferDescs;
    for (auto& desc : boundUAVsDescs)
    {
        ASSERT(desc.Dimension != D3D_SRV_DIMENSION_UNKNOWN, "unknown");
        if (desc.Dimension == D3D_SRV_DIMENSION_BUFFER || desc.Dimension == D3D_SRV_DIMENSION_BUFFEREX)
        {
            UAVBufferDescs.push_back(desc);
        }
        else
        {
            UAVImageDescs.push_back(desc);
        }
    }

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

    auto convertResourceTypeToFormat = [](D3D_RESOURCE_RETURN_TYPE type, u32 flags) -> renderer::Format
    {
        u32 countComponents = 0;
        if (flags & D3D_SIF_TEXTURE_COMPONENTS)
        {
            countComponents = 4;
        }
        else if (flags & D3D_SIF_TEXTURE_COMPONENT_1)
        {
            countComponents = 3;
        }
        else if (flags & D3D_SIF_TEXTURE_COMPONENT_0)
        {
            countComponents = 2;
        }

        switch (type)
        {
        case D3D_RETURN_TYPE_UNORM:
            if (countComponents == 4)
            {
                return renderer::Format::Format_R8G8B8A8_UNorm;
            }
            else if (countComponents == 3)
            {
                return renderer::Format::Format_R8G8B8_UNorm;
            }
            else if (countComponents == 2)
            {
                return renderer::Format::Format_R8G8_UNorm;
            }
            break;

        case D3D_RETURN_TYPE_SNORM:
            if (countComponents == 4)
            {
                return renderer::Format::Format_R8G8B8A8_SNorm;
            }
            else if (countComponents == 3)
            {
                return renderer::Format::Format_R8G8B8_SNorm;
            }
            else if (countComponents == 2)
            {
                return renderer::Format::Format_R8G8_SNorm;
            }
            break;

        case D3D_RETURN_TYPE_SINT:
            if (countComponents == 4)
            {
                return renderer::Format::Format_R32G32B32A32_SInt;
            }
            else if (countComponents == 3)
            {
                return renderer::Format::Format_R32G32B32_SInt;
            }
            else if (countComponents == 2)
            {
                return renderer::Format::Format_R32G32_SInt;
            }
            break;

        case D3D_RETURN_TYPE_UINT:
            if (countComponents == 4)
            {
                return renderer::Format::Format_R32G32B32A32_UInt;
            }
            else if (countComponents == 3)
            {
                return renderer::Format::Format_R32G32B32_UInt;
            }
            else if (countComponents == 2)
            {
                return renderer::Format::Format_R32G32_UInt;
            }
            break;

        case D3D_RETURN_TYPE_FLOAT:
            if (countComponents == 4)
            {
                return renderer::Format::Format_R32G32B32A32_SFloat;
            }
            else if (countComponents == 3)
            {
                return renderer::Format::Format_R32G32B32_SFloat;
            }
            else if (countComponents == 2)
            {
                return renderer::Format::Format_R32G32_SFloat;
            }
            break;

        case D3D_RETURN_TYPE_MIXED:
        case D3D_RETURN_TYPE_DOUBLE:
        case D3D_RETURN_TYPE_CONTINUED:
        default:
            ASSERT(false, "not defined");
        }

        ASSERT(false, "unknown");
        return renderer::Format::Format_Undefined;
    };

    {
        u32 countStorageImage = static_cast<u32>(UAVImageDescs.size());
        stream->write<u32>(countStorageImage);

        for (auto& desc : UAVImageDescs)
        {
            renderer::Shader::StorageImage UAVTexture;
            UAVTexture._set = desc.Space;
            UAVTexture._binding = desc.BindPoint;

            UAVTexture._target = convertDXTypeToTextureTarget(desc.Dimension);
            UAVTexture._format = convertResourceTypeToFormat(desc.ReturnType, desc.uFlags);
            UAVTexture._array = desc.BindCount;
            UAVTexture._readonly = false;
            UAVTexture._name = std::string(desc.Name);

            UAVTexture >> stream;
        }
    }

    {
        u32 countStorageBuffers = static_cast<u32>(UAVBufferDescs.size());
        stream->write<u32>(countStorageBuffers);

        for (auto& desc : UAVBufferDescs)
        {
            renderer::Shader::StorageBuffer UAVBuffer;
            UAVBuffer._set = desc.Space;
            UAVBuffer._binding = desc.BindPoint;

            UAVBuffer._format = convertResourceTypeToFormat(desc.ReturnType, desc.uFlags);
            UAVBuffer._array = desc.BindCount;
            UAVBuffer._readonly = false;
            UAVBuffer._name = std::string(desc.Name);

            UAVBuffer >> stream;
        }

    }
}

} //namespace resource
} //namespace v3d
#endif //PLATFORM_WINDOWS