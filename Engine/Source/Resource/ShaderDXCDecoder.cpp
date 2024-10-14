#include "ShaderDXCDecoder.h"
#include "Stream/StreamManager.h"
#include "Stream/FileLoader.h"
#include "Utils/Logger.h"
#include "Utils//Timer.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)
//#include "Renderer/D3D12/D3DDebug.h"
#include "ShaderReflectionSpirV.h"
#include "ShaderReflectionDXC.h"

#if USE_CUSTOM_DXC
#   include "dxc/inc/dxcapi.h"
#   include "dxc/inc/d3d12shader.h"
#else
#   if defined(PLATFORM_WINDOWS)
#       include <dxcapi.h>
#       include <d3d12shader.h>
#       pragma comment(lib, "dxcompiler.lib")
#       pragma comment(lib, "dxil.lib")
#   elif defined(PLATFORM_XBOX)
#       include <dxcapi_xs.h>
#       include <d3d12shader_xs.h>
#       pragma comment(lib, "dxcompiler.lib")
#   endif //PLATFORM
#endif //DXC

#ifdef USE_SPIRV
#   include "spirv-tools/libspirv.hpp"
#endif //USE_SPIRV

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

const std::map<std::string, renderer::ShaderType> k_HLSL_ExtensionList =
{
    //hlsl
    { "vs", renderer::ShaderType::Vertex },
    { "ps", renderer::ShaderType::Fragment },
    { "cs", renderer::ShaderType::Compute },
};

constexpr u32 g_DXDCIndifier = 0x43425844; //DXBC


bool checkBytecodeSigning(IDxcBlob* bytecode);
#ifdef USE_SPIRV
bool disassembleSpirv(IDxcBlob* binaryBlob, std::string& assembleSPIV);
#endif //USE_SPIRV

ShaderDXCDecoder::ShaderDXCDecoder() noexcept
{
}

ShaderDXCDecoder::ShaderDXCDecoder(const std::vector<std::string>& supportedExtensions) noexcept
    : ShaderDecoder(supportedExtensions)
{
}

ShaderDXCDecoder::ShaderDXCDecoder(std::vector<std::string>&& supportedExtensions) noexcept
    : ShaderDecoder(std::move(supportedExtensions))
{
}

Resource* ShaderDXCDecoder::decode(const stream::Stream* stream, const Policy* policy, u32 flags, const std::string& name) const
{
    if (!stream || stream->size() == 0)
    {
        LOG_ERROR("ShaderDXCDecoder::decode the stream is empty");
        return nullptr;
    }

    stream->seekBeg(0);

    auto isHLSL_ShaderModel6 = [](renderer::ShaderModel model) -> bool
    {
        return model == renderer::ShaderModel::Default
            || (model >= renderer::ShaderModel::HLSL_6_0 || model <= renderer::ShaderModel::HLSL_6_6);
    };

    const ShaderPolicy* shaderPolicy = static_cast<const ShaderPolicy*>(policy);
    if (!isHLSL_ShaderModel6(shaderPolicy->_model))
    {
        LOG_ERROR("ShaderDXCDecoder::decode support HLSL SM6.0 and above");
        return nullptr;
    }

    if (shaderPolicy->_content == renderer::ShaderContent::Source)
    {
        std::string source;
        source.resize(stream->size());
        stream->read(source.data(), stream->size());
#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        IDxcBlob* binaryShader = nullptr;
        if (!ShaderDXCDecoder::compile(source, shaderPolicy, flags, binaryShader))
        {
            if (binaryShader)
            {
                binaryShader->Release();
            }

            LOG_ERROR("ShaderDXCDecoder::decode: compile is failed");
            return nullptr;
        }

        ASSERT(binaryShader, "nullptr");
        u32 bytecodeSize = static_cast<u32>(binaryShader->GetBufferSize());
        stream::Stream* resourceBinary = stream::StreamManager::createMemoryStream();

        resourceBinary->write<u32>(bytecodeSize);
        resourceBinary->write(binaryShader->GetBufferPointer(), bytecodeSize);
        resourceBinary->write(m_entrypoint);

        resourceBinary->write<bool>(m_reflections);
        if (m_reflections && !ShaderDXCDecoder::reflect(resourceBinary, binaryShader))
        {
            LOG_ERROR("ShaderDXCDecoder::decode: reflect is failed");

            binaryShader->Release();
            stream::StreamManager::destroyStream(resourceBinary);

            return nullptr;
        }
        binaryShader->Release();

        renderer::ShaderHeader shaderHeader(m_header);
        resource::ResourceHeader::fillResourceHeader(&shaderHeader, name, resourceBinary->size(), 0);
        shaderHeader._shaderType = shaderType;
        shaderHeader._contentType = renderer::ShaderHeader::ShaderContent::Bytecode;
        shaderHeader._shaderModel = m_outputSM;

        Resource* resource = ::V3D_NEW(renderer::Shader, memory::MemoryLabel::MemoryObject)(V3D_NEW(renderer::ShaderHeader, memory::MemoryLabel::MemoryObject)(shaderHeader));
        if (!resource->load(resourceBinary))
        {
            LOG_ERROR("ShaderDXCDecoder::decode: the shader loading is failed");

            V3D_DELETE(resource, memory::MemoryLabel::MemoryObject);
            resource = nullptr;
        }
        stream::StreamManager::destroyStream(resourceBinary);

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("ShaderDXCDecoder::decode, the shader %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return resource;
    }
    else
    {
        ASSERT(false, "not impl");
        return nullptr;
    }
}

bool ShaderDXCDecoder::compile(const std::string& source, const ShaderPolicy* policy, ShaderCompileFlags flags, IDxcBlob*& shader) const
{
    std::vector<LPCWSTR> arguments;
    if (flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
    {
        arguments.push_back(L"-spirv");
    }

    if (flags & ShaderCompileFlag::ShaderCompile_OptimizationFull)
    {
        arguments.push_back(L"-O3");
    }
    else if (flags & ShaderCompileFlag::ShaderCompile_OptimizationPerformance)
    {
        arguments.push_back(L"-O2");
    }
    else if (flags & ShaderCompileFlag::ShaderCompile_OptimizationSize)
    {
        arguments.push_back(L"-O1");
    }
    else
    {
        arguments.push_back(L"-Od");
        arguments.push_back(L"-Vd");
        arguments.push_back(L"-Zi");
        //arguments.push_back(L"-Qembed_debug");
    }
#ifndef DEBUG
    arguments.push_back(L"-no-warnings");
#endif

    auto getShaderTarget = [](renderer::ShaderType type, renderer::ShaderModel model) -> std::wstring
    {
        std::wstring target = L"";

        switch (type)
        {
        case renderer::ShaderType::Vertex:
            target.assign(L"vs_");
            break;

        case renderer::ShaderType::Fragment:
            target.assign(L"ps_");
            break;

        case renderer::ShaderType::Compute:
            target.assign(L"cs_");
            break;

        default:
            ASSERT(false, "type hasn't detected");
            return L"xc_";
        }

        switch (model)
        {
        case renderer::ShaderModel::HLSL_5_1:
            target.append(L"5_1");
            break;

        case renderer::ShaderModel::Default:
        case renderer::ShaderModel::HLSL_6_0:
            target.append(L"6_0");
            break;

        case renderer::ShaderModel::HLSL_6_1:
            target.append(L"6_1");
            break;

        case renderer::ShaderModel::HLSL_6_2:
            target.append(L"6_2");
            break;

        case renderer::ShaderModel::HLSL_6_3:
            target.append(L"6_3");
            break;

        case renderer::ShaderModel::HLSL_6_4:
            target.append(L"6_4");
            break;

        case renderer::ShaderModel::HLSL_6_5:
            target.append(L"6_5");
            break;

        case renderer::ShaderModel::HLSL_6_6:
            target.append(L"6_6");
            break;

        default:
            ASSERT(false, "target hasn't detected");
            return L"xs_x_x";
        }

        return target;
    };

    const std::wstring entryPoint = std::wstring(policy->_entryPoint.cbegin(), policy->_entryPoint.cend());
    const std::wstring target = getShaderTarget(policy->_type, policy->_model);

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
#else //USE_CUSTOM_DXC
    IDxcCompiler3* DXCompiler = nullptr;
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

    std::vector<std::pair<std::wstring, std::wstring>> wsDefines(policy->_defines.size());
    for (u32 i = 0; i < wsDefines.size(); ++i)
    {
        wsDefines[i].first = std::move(std::wstring(policy->_defines[i].first.cbegin(), policy->_defines[i].first.cend()));
        wsDefines[i].second = std::move(std::wstring(policy->_defines[i].second.cbegin(), policy->_defines[i].second.cend()));
    }

    std::vector<DxcDefine> dxcDefines(wsDefines.size());
    for (u32 i = 0; i < policy->_defines.size(); ++i)
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
        ASSERT(DXSource, "nullptr");
        if (m_outputSM == renderer::ShaderHeader::ShaderModel::SpirV)
        {
            std::string disassembleShaderCode;
            if (disassembleSpirv(shader, disassembleShaderCode))
            {
                LOG_DEBUG("Disassemble SPIRV [%s]: \n %s", name.c_str(), disassembleShaderCode.c_str());
            }
        }
        else
        {
            IDxcBlobEncoding* disassembleShader = nullptr;

            HRESULT result = DXCompiler->Disassemble(shader, &disassembleShader);
            if (SUCCEEDED(result))
            {
                ASSERT(disassembleShader, "nullptr");
                std::string disassembleShaderCode(reinterpret_cast<c8*>(disassembleShader->GetBufferSize(), disassembleShader->GetBufferPointer()));
                LOG_DEBUG("Disassemble [%s]: \n %s", name.c_str(), disassembleShaderCode.c_str());
            }

            if (disassembleShader)
            {
                disassembleShader->Release();
            }
        }
    }
#endif

    bool useValidator = !(flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV); //Only for DXBC
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

bool ShaderDXCDecoder::reflect(stream::Stream* stream, const ShaderPolicy* policy, ShaderCompileFlags flags, IDxcBlob* shader) const
{
    if (flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
    {
#ifdef USE_SPIRV
        std::vector<u32> spirv(shader->GetBufferSize() / sizeof(u32));
        memcpy(spirv.data(), shader->GetBufferPointer(), shader->GetBufferSize());
        ASSERT(spirv[0] == 0x07230203, "invalid spirv magic number in head");

        //TODO: parses twice for HLSL and dublicate all binding.
        ShaderReflectionSpirV reflector(policy->_model);
        return reflector.reflect(spirv, stream);
#endif //USE_SPIRV
    }
    else
    {
        ShaderReflectionDXC reflector;
        return reflector.reflect(shader, stream);
    }

    ASSERT(false, "shader model is not supported");
    return false;
}

bool checkBytecodeSigning(IDxcBlob* bytecode)
{
    if (!bytecode && bytecode->GetBufferSize() == 0)
    {
        return false;
    }

    struct DxilHeader
    {
        u32 _dxbc;
        u32 _hashDigest[4];
    };

    const DxilHeader* header = reinterpret_cast<const DxilHeader*>(bytecode->GetBufferPointer());
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

#ifdef USE_SPIRV
bool disassembleSpirv(IDxcBlob* binaryBlob, std::string& assembleSPIV)
{
    if (!binaryBlob)
    {
        return true;
    }

    size_t num32BitWords = (binaryBlob->GetBufferSize() + 3) / 4;
    std::string binaryStr((char*)binaryBlob->GetBufferPointer(), binaryBlob->GetBufferSize());
    binaryStr.resize(num32BitWords * 4, 0);

    std::vector<uint32_t> words;
    words.resize(num32BitWords, 0);
    memcpy(words.data(), binaryStr.data(), binaryStr.size());

    spvtools::SpirvTools spirvTools(SPV_ENV_UNIVERSAL_1_6);
    uint32_t options = SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES | SPV_BINARY_TO_TEXT_OPTION_INDENT;

    if (!spirvTools.Disassemble(words, &assembleSPIV, options))
    {
        return false;
    }
    return true;
}
#endif //USE_SPIRV

} //namespace resource
} //namespace v3d
#endif //PLATFORM_WINDOWS