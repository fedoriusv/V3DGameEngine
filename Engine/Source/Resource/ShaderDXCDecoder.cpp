#include "ShaderDXCDecoder.h"
#include "Stream/StreamManager.h"
#include "Utils/Logger.h"
#include "Utils//Timer.h"

#ifdef D3D_RENDER
#include "Renderer/D3D12/D3DDebug.h"

#if USE_CUSTOM_DXC
#   include "dxc/inc/dxcapi.h"
#   include "dxc/inc/d3d12shader.h"
#else
#   include <dxcapi.h>
#   include <d3d12shader.h>
#   pragma comment(lib, "dxcompiler.lib")
//#    pragma comment(lib, "dxil.lib")
#endif

namespace v3d
{
namespace resource
{

constexpr u32 g_DXDCIndifier = 0x43425844; //DXBC

void reflectAttributes(ID3D12ShaderReflection* reflector, const D3D12_SHADER_DESC& shaderDesc, stream::Stream* stream);
void reflectConstantBuffers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& bindDescs, stream::Stream* stream);
void reflectSampledImages(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundTexturesDescs, stream::Stream* stream);
void reflectSamplers(ID3D12ShaderReflection* reflector, const std::vector<D3D12_SHADER_INPUT_BIND_DESC>& boundSamplersDescs, stream::Stream* stream);

bool checkBytecodeSigning(IDxcBlob* bytecode);


ShaderDXCDecoder::ShaderDXCDecoder(const renderer::ShaderHeader& header, renderer::ShaderHeader::ShaderModel output, bool reflections) noexcept
    : m_header(header)
    , m_reflections(reflections)
    , m_output(output)
{
}

ShaderDXCDecoder::ShaderDXCDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, renderer::ShaderHeader::ShaderModel output, bool reflections) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_reflections(reflections)
    , m_output(output)
{
}

Resource* ShaderDXCDecoder::decode(const stream::Stream* stream, const std::string& name) const
{
    if (stream->size() == 0)
    {
        return nullptr;
    }

    stream->seekBeg(0);

    auto isHLSL_ShaderModel6 = [](renderer::ShaderHeader::ShaderModel model) -> bool
    {
        return model == renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_0 || model == renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_1;
    };

    if (!isHLSL_ShaderModel6(m_header._shaderModel))
    {
        LOG_ERROR("ShaderDXCDecoder::decode support HLSL SM 6.0 and above");
        return nullptr;
    }

#if DEBUG
    const std::string shaderName = m_header._debugName.empty() ? name : m_header._debugName;
#else
    const std::string shaderName = name;
#endif

    if (m_header._contentType == renderer::ShaderHeader::ShaderResource::ShaderResource_Source)
    {
        std::string source;
        source.resize(stream->size());
        stream->read(source.data(), stream->size());
#if DEBUG
        utils::Timer timer;
        timer.start();
#endif

        IDxcBlob* binaryShader = nullptr;
        if (!compile(source, std::wstring(shaderName.cbegin(), shaderName.cend()), binaryShader))
        {
            if (binaryShader)
            {
                binaryShader->Release();
            }

            return nullptr;
        }

        ASSERT(binaryShader, "nullptr");
        u32 bytecodeSize = static_cast<u32>(binaryShader->GetBufferSize());
        stream::Stream* resourceBinary = stream::StreamManager::createMemoryStream(nullptr, bytecodeSize + sizeof(u32) + sizeof(bool));
        resourceBinary->write<u32>(bytecodeSize);
        resourceBinary->write(binaryShader->GetBufferPointer(), bytecodeSize);

        resourceBinary->write<bool>(m_reflections);
        if (m_reflections && !reflect(resourceBinary, binaryShader))
        {
            if (binaryShader)
            {
                binaryShader->Release();
            }
            delete resourceBinary;

            return nullptr;
        }

        renderer::ShaderHeader* resourceHeader = new renderer::ShaderHeader(m_header);
#if DEBUG
        resourceHeader->_debugName = shaderName;
#endif
        Resource* resource = new renderer::Shader(resourceHeader);
        resource->init(resourceBinary);

#if DEBUG
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("ShaderDXCDecoder::decode, shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

        //binaryShader->Release();
        return resource;
    }
    else
    {
        ASSERT(false, "not impl");
        return nullptr;
    }
}

bool ShaderDXCDecoder::compile(const std::string& source, const std::wstring& name, IDxcBlob*& shader) const
{
    std::vector<LPCWSTR> arguments;
    if (m_output == renderer::ShaderHeader::ShaderModel::ShaderModel_SpirV)
    {
        arguments.push_back(L"-spirv");
    }

    if (m_header._optLevel == 0)
    {
        arguments.push_back(L"-Od");
        arguments.push_back(L"-Vd");
        arguments.push_back(L"-Zi");

        //arguments.push_back(L"-Qembed_debug");
    }
    else if (m_header._optLevel == 1)
    {
        arguments.push_back(L"-O1");
    }
    else if (m_header._optLevel == 2)
    {
        arguments.push_back(L"-O2");
    }
    else if (m_header._optLevel == 3)
    {
        arguments.push_back(L"-O3");
    }
#ifndef DEBUG
    arguments.push_back(L"-no-warnings");
#endif

    auto getShaderTarget = [](renderer::ShaderType type, renderer::ShaderHeader::ShaderModel model) -> std::wstring
    {
        switch (type)
        {
        case renderer::ShaderType::ShaderType_Vertex:
        {
            switch (model)
            {
            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0:
                return L"vs_5_0";

            case renderer::ShaderHeader::ShaderModel::ShaderModel_Default:
            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1:
                return L"vs_5_1";

            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_0:
                return L"vs_6_0";

            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_1:
                return L"vs_6_1";

            default:
                ASSERT(false, "target hasn't detected");
                return L"vs_x_x";
            }
        }

        case renderer::ShaderType::ShaderType_Fragment:
        {
            switch (model)
            {
            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_0:
                return L"ps_5_0";

            case renderer::ShaderHeader::ShaderModel::ShaderModel_Default:
            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1:
                return L"ps_5_1";

            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_0:
                return L"ps_6_0";

            case renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_1:
                return L"ps_6_1";

            default:
                ASSERT(false, "target hasn't detected");
                return L"ps_x_x";
            }
        }

        default:
        {
            ASSERT(false, "shader type hasn't detected");
            return L"xs_x_x";
        };
        }
    };

    const std::wstring entryPoint = m_header._entryPoint.empty() ? L"main" : std::wstring(m_header._entryPoint.cbegin(), m_header._entryPoint.cend());
    const std::wstring target = getShaderTarget(m_header._type, m_header._shaderModel);

#if USE_CUSTOM_DXC
    IDxcCompiler3* DXCompiler = nullptr;
    {
        HRESULT result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DXCompiler));
        if (FAILED(result))
        {
            LOG_FATAL("ShaderDXCDecoder DxcCreateInstance can't create IDxcCompiler. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    for (u32 i = 0; i < m_header._defines.size(); ++i)
    {
        std::wstring defineArg = (L"-D");
        defineArg.append(std::move(std::wstring(m_header._defines[i].first.cbegin(), m_header._defines[i].first.cend())));
        defineArg.append(L" ");
        defineArg.append(std::move(std::wstring(m_header._defines[i].second.cbegin(), m_header._defines[i].second.cend())));

        arguments.push_back(defineArg.c_str());
    }

    std::wstring entryPointArg(L"-E " + entryPoint);
    arguments.push_back(entryPointArg.c_str());

    std::wstring targetArg(L"-T " + target);
    arguments.push_back(targetArg.c_str()); //has compile error
    ASSERT(false, "need implement");

    DxcBuffer dxBuffer = {};
    dxBuffer.Ptr = source.c_str();
    dxBuffer.Size = static_cast<u32>(source.size());
    dxBuffer.Encoding = CP_UTF8;

    IDxcResult* compileResult = nullptr;
    {
        HRESULT result = DXCompiler->Compile(&dxBuffer, arguments.data(), static_cast<u32>(arguments.size()), nullptr, IID_PPV_ARGS(&compileResult));
        if (FAILED(result))
        {
            DXCompiler->Release();

            LOG_ERROR("ShaderDXCDecoder Compile can't create IDxcResult. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    {
        IDxcBlobUtf16* compileName = nullptr;
        IDxcBlob* compileErrors = nullptr;
        HRESULT result = compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&compileErrors), &compileName);
        if (FAILED(result))
        {
            LOG_ERROR("ShaderDXCDecoder GetOutput. Can't get an error buffer. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            ASSERT(false, "wrong");
        }

        if (compileErrors && compileErrors->GetBufferSize() > 0)
        {
            std::string compileErrorString(reinterpret_cast<c8*>(compileErrors->GetBufferSize(), compileErrors->GetBufferPointer()));
            LOG_ERROR("ShaderDXCDecoder Compile Error: %s", compileErrorString.c_str());
        }

        if (compileErrors)
        {
            compileErrors->Release();
            compileErrors = nullptr;
        }
    }

    {
        IDxcBlobUtf16* compileName = nullptr;
        IDxcBlob* compileBinary = nullptr;
        HRESULT result = compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compileBinary), &compileName);
        if (FAILED(result))
        {
            LOG_ERROR("ShaderDXCDecoder GetOutput. Can't get an binary buffer. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            ASSERT(false, "wrong");
        }

        if (!compileBinary || compileBinary->GetBufferSize() == 0)
        {
            LOG_ERROR("ShaderDXCDecoder Compile is failed");
            compileResult->Release();
            DXCompiler->Release();

            return false;
        }
    }
    //TODO

    return true;
#else
    IDxcCompiler2* DXCompiler = nullptr;
    {
        HRESULT result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DXCompiler));
        if (FAILED(result))
        {
            LOG_FATAL("ShaderDXCDecoder DxcCreateInstance can't create IDxcCompiler. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    IDxcLibrary* DXLibrary = nullptr;
    {
        HRESULT result = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&DXLibrary));
        if (FAILED(result))
        {
            DXCompiler->Release();

            LOG_FATAL("ShaderDXCDecoder DxcCreateInstance can't create IDxcLibrary. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    IDxcBlobEncoding* DXSource = nullptr;
    LOG_DEBUG("Shader[%s]:\n %s\n", name.c_str(), source.c_str());
    {
        HRESULT result = DXLibrary->CreateBlobWithEncodingFromPinned(source.c_str(), static_cast<u32>(source.size()), CP_UTF8, &DXSource);
        if (FAILED(result))
        {
            DXLibrary->Release();
            DXCompiler->Release();

            LOG_ERROR("ShaderDXCDecoder CreateBlobWithEncodingFromPinned can't create IDxcBlobEncoding. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    std::vector<std::pair<std::wstring, std::wstring>> wsDefines(m_header._defines.size());
    for (u32 i = 0; i < wsDefines.size(); ++i)
    {
        wsDefines[i].first = std::move(std::wstring(m_header._defines[i].first.cbegin(), m_header._defines[i].first.cend()));
        wsDefines[i].second = std::move(std::wstring(m_header._defines[i].second.cbegin(), m_header._defines[i].second.cend()));
    }

    std::vector<DxcDefine> dxcDefines(wsDefines.size());
    for (u32 i = 0; i < m_header._defines.size(); ++i)
    {
        DxcDefine& m = dxcDefines[i];
        m.Name = wsDefines[i].first.c_str();
        m.Value = wsDefines[i].second.c_str();
    }

    IDxcOperationResult* compileResult = nullptr;
    HRESULT result = DXCompiler->Compile(DXSource, name.c_str(), entryPoint.c_str(), target.c_str(), arguments.data(), static_cast<u32>(arguments.size()), dxcDefines.data(), static_cast<u32>(dxcDefines.size()), nullptr, &compileResult);
    if (FAILED(result))
    {
        DXSource->Release();

        DXLibrary->Release();
        DXCompiler->Release();

        LOG_ERROR("ShaderDXCDecoder Compile can't create IDxcBlobEncoding. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
        return false;
    }
    DXSource->Release();

    HRESULT compilationStatus;
    ASSERT(compileResult, "nullptr");
    {
        HRESULT result = compileResult->GetStatus(&compilationStatus);
        ASSERT(SUCCEEDED(result), "compilationStatus is failed");
    }

    if (FAILED(compileResult->GetResult(&shader)) || !shader)
    {
        ASSERT(false, "shader is failed");
        compileResult->Release();

        DXSource->Release();

        DXLibrary->Release();
        DXCompiler->Release();

        return false;
    }

    if (FAILED(compilationStatus))
    {
        IDxcBlobEncoding* errorBuffer;
        compileResult->GetErrorBuffer(&errorBuffer);
        ASSERT(errorBuffer, "nullptr");

        std::string compileErrorString(reinterpret_cast<c8*>(errorBuffer->GetBufferSize(), errorBuffer->GetBufferPointer()));
        LOG_ERROR("ShaderDXCDecoder Compile Error: %s", compileErrorString.c_str());

        errorBuffer->Release();
        compileResult->Release();

        DXSource->Release();

        DXLibrary->Release();
        DXCompiler->Release();

        return false;
    }
    compileResult->Release();

#if (DEBUG & D3D_DEBUG)
    {
        IDxcBlobEncoding* disassembleShader = nullptr;
        ASSERT(DXSource, "nullptr");
        if (m_output != renderer::ShaderHeader::ShaderModel::ShaderModel_SpirV)
        {
            HRESULT result = DXCompiler->Disassemble(shader, &disassembleShader);
            if (SUCCEEDED(result))
            {
                ASSERT(disassembleShader, "nullptr");
                std::string disassembleShaderCode(reinterpret_cast<c8*>(disassembleShader->GetBufferSize(), disassembleShader->GetBufferPointer()));
                LOG_DEBUG("Disassemble [%s]: \n %s", name.c_str(), disassembleShaderCode.c_str());
            }
        }

        if (disassembleShader)
        {
            disassembleShader->Release();
        }
    }
#endif

    bool useValidator = true;
    if (useValidator)
    {
        IDxcValidator* DXValidator = nullptr;
        {
            HRESULT result = DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&DXValidator));
            if (FAILED(result))
            {
                DXLibrary->Release();
                DXCompiler->Release();

                LOG_FATAL("ShaderDXCDecoder DxcCreateInstance can't create DxcValidator. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
                return false;
            }
        }

        IDxcOperationResult* validationResult = nullptr;
        {
            HRESULT result = DXValidator->Validate(shader, DxcValidatorFlags_Default, &validationResult);
            ASSERT(SUCCEEDED(result), "validationResult is failed");
        }

        HRESULT validatiorStatus;
        ASSERT(validationResult, "nullptr");
        {
            HRESULT result = validationResult->GetStatus(&validatiorStatus);
            ASSERT(SUCCEEDED(result), "validatiorStatus is failed");
        }
        checkBytecodeSigning(shader);

        if (FAILED(validatiorStatus))
        {
            IDxcBlobEncoding* errorBuffer;
            validationResult->GetErrorBuffer(&errorBuffer);
            ASSERT(errorBuffer, "nullptr");

            std::string compileErrorString(reinterpret_cast<c8*>(errorBuffer->GetBufferSize(), errorBuffer->GetBufferPointer()));
            LOG_ERROR("ShaderDXCDecoder Validation Error: %s", compileErrorString.c_str());

            errorBuffer->Release();
        }

        validationResult->Release();
        DXValidator->Release();
    }

    DXLibrary->Release();
    DXCompiler->Release();

    return true;
#endif //USE_CUSTOM_DXC
}

bool ShaderDXCDecoder::reflect(stream::Stream* stream, IDxcBlob* shader) const
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
        HRESULT result = DXContainerReflection->Load(shader);
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

bool checkBytecodeSigning(IDxcBlob* bytecode)
{
    if (!bytecode && bytecode->GetBufferSize() == 0)
    {
        return false;
    }

    struct DxilMinimalHeader
    {
        u32 _dxbc;
        u32 _hashDigest[4];
    };

    DxilMinimalHeader* header = reinterpret_cast<DxilMinimalHeader*>(bytecode->GetBufferPointer());
    if (header->_dxbc != g_DXDCIndifier)
    {
        return false;
    }

    bool isSigned = false;
    isSigned |= header->_hashDigest[0] != 0x0;
    isSigned |= header->_hashDigest[1] != 0x0;
    isSigned |= header->_hashDigest[2] != 0x0;
    isSigned |= header->_hashDigest[3] != 0x0;

    return isSigned;
}

#endif //D3D_RENDER
} //namespace resource
} //namespace v3d