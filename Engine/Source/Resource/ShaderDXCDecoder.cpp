#include "ShaderDXCDecoder.h"
#include "Stream/StreamManager.h"
#include "Stream/FileLoader.h"
#include "Utils/Logger.h"
#include "Utils//Timer.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)
#include "Renderer/Core/D3D12/D3DDebug.h"
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
//#     pragma comment(lib, "dxil.lib")
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

ShaderDXCDecoder::ShaderDXCDecoder(const renderer::ShaderHeader& header, const std::string& entrypoint, const renderer::Shader::DefineList& defines, 
    const std::vector<std::string>& includes, renderer::ShaderHeader::ShaderModel output, renderer::ShaderCompileFlags flags) noexcept
    : m_header(header)
    , m_reflections(!(flags & renderer::ShaderCompileFlag::ShaderSource_DontUseReflection))

    , m_entrypoint(entrypoint)
    , m_defines(defines)
    , m_includes(includes)

    , m_outputSM(output)
{
    ASSERT(!(flags & renderer::ShaderCompileFlag::ShaderSource_UseLegacyCompilerForHLSL), "must be false");
}

ShaderDXCDecoder::ShaderDXCDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, const std::string& entrypoint, const renderer::Shader::DefineList& defines,
    const std::vector<std::string>& includes, renderer::ShaderHeader::ShaderModel output, renderer::ShaderCompileFlags flags) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_reflections(!(flags& renderer::ShaderCompileFlag::ShaderSource_DontUseReflection))

    , m_entrypoint(entrypoint)
    , m_defines(defines)
    , m_includes(includes)

    , m_outputSM(output)
{
    ASSERT(!(flags & renderer::ShaderCompileFlag::ShaderSource_UseLegacyCompilerForHLSL), "must be false");
}

Resource* ShaderDXCDecoder::decode(const stream::Stream* stream, const std::string& name) const
{
    if (stream->size() == 0)
    {
        LOG_ERROR("ShaderDXCDecoder::decode the stream is empty");
        return nullptr;
    }
    stream->seekBeg(0);

    auto isHLSL_ShaderModel6 = [](renderer::ShaderHeader::ShaderModel model) -> bool
    {
        return model == renderer::ShaderHeader::ShaderModel::Default ||
            model == renderer::ShaderHeader::ShaderModel::HLSL_6_1 || model == renderer::ShaderHeader::ShaderModel::HLSL_6_6;
    };

    if (!isHLSL_ShaderModel6(m_header._shaderModel))
    {
        LOG_ERROR("ShaderDXCDecoder::decode support HLSL SM6.0 and above");
        return nullptr;
    }

    if (m_header._contentType == renderer::ShaderHeader::ShaderContent::Source)
    {
        std::string source;
        source.resize(stream->size());
        stream->read(source.data(), stream->size());
#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        renderer::ShaderType shaderType = m_header._shaderType;
        if (m_header._shaderType == renderer::ShaderType::Undefined)
        {
            std::string fileExtension = stream::FileLoader::getFileExtension(name);
            auto result = k_HLSL_ExtensionList.find(fileExtension);
            if (result != k_HLSL_ExtensionList.cend())
            {
                shaderType = result->second;
            }
        }

        IDxcBlob* binaryShader = nullptr;
        if (!ShaderDXCDecoder::compile(source, shaderType, std::wstring(name.cbegin(), name.cend()), binaryShader))
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

bool ShaderDXCDecoder::compile(const std::string& source, renderer::ShaderType shaderType, const std::wstring& name, IDxcBlob*& shader) const
{
    std::vector<LPCWSTR> arguments;
    if (m_outputSM == renderer::ShaderHeader::ShaderModel::SpirV)
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
        case renderer::ShaderType::Vertex:
        {
            switch (model)
            {
            case renderer::ShaderHeader::ShaderModel::HLSL_5_0:
                return L"vs_5_0";

            case renderer::ShaderHeader::ShaderModel::HLSL_5_1:
                return L"vs_5_1";

            case renderer::ShaderHeader::ShaderModel::Default:
            case renderer::ShaderHeader::ShaderModel::HLSL_6_1:
                return L"vs_6_1";

            case renderer::ShaderHeader::ShaderModel::HLSL_6_6:
                return L"vs_6_6";

            default:
                ASSERT(false, "target hasn't detected");
                return L"vs_x_x";
            }
        }

        case renderer::ShaderType::Fragment:
        {
            switch (model)
            {
            case renderer::ShaderHeader::ShaderModel::HLSL_5_0:
                return L"ps_5_0";

            case renderer::ShaderHeader::ShaderModel::HLSL_5_1:
                return L"ps_5_1";

            case renderer::ShaderHeader::ShaderModel::Default:
            case renderer::ShaderHeader::ShaderModel::HLSL_6_1:
                return L"ps_6_1";

            case renderer::ShaderHeader::ShaderModel::HLSL_6_6:
                return L"ps_6_6";

            default:
                ASSERT(false, "target hasn't detected");
                return L"ps_x_x";
            }
        }

        case renderer::ShaderType::Compute:
        {
            switch (model)
            {
            case renderer::ShaderHeader::ShaderModel::HLSL_5_0:
                return L"cs_5_0";

            case renderer::ShaderHeader::ShaderModel::HLSL_5_1:
                return L"cs_5_1";

            case renderer::ShaderHeader::ShaderModel::Default:
            case renderer::ShaderHeader::ShaderModel::HLSL_6_1:
                return L"cs_6_1";

            case renderer::ShaderHeader::ShaderModel::HLSL_6_6:
                return L"cs_6_6";

            default:
                ASSERT(false, "target hasn't detected");
                return L"cs_x_x";
            }
        }

        default:
        {
            ASSERT(false, "shader type hasn't detected");
            return L"xs_x_x";
        };
        }
    };

    const std::wstring entryPoint = m_entrypoint.empty() ? L"main" : std::wstring(m_entrypoint.cbegin(), m_entrypoint.cend());
    const std::wstring target = getShaderTarget(shaderType, m_header._shaderModel);

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

    std::vector<std::pair<std::wstring, std::wstring>> wsDefines(m_defines.size());
    for (u32 i = 0; i < wsDefines.size(); ++i)
    {
        wsDefines[i].first = std::move(std::wstring(m_defines[i].first.cbegin(), m_defines[i].first.cend()));
        wsDefines[i].second = std::move(std::wstring(m_defines[i].second.cbegin(), m_defines[i].second.cend()));
    }

    std::vector<DxcDefine> dxcDefines(wsDefines.size());
    for (u32 i = 0; i < m_defines.size(); ++i)
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

    bool useValidator = m_outputSM != renderer::ShaderHeader::ShaderModel::SpirV; //Only for DXBC
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
    switch (m_outputSM)
    {
#ifdef USE_SPIRV
    case renderer::ShaderHeader::ShaderModel::SpirV:
    {
        std::vector<u32> spirv(shader->GetBufferSize() / sizeof(u32));
        memcpy(spirv.data(), shader->GetBufferPointer(), shader->GetBufferSize());
        ASSERT(spirv[0] == 0x07230203, "invalid spirv magic number in head");

        //TODO: parses twice for HLSL and dublicate all binding.
        ShaderReflectionSpirV reflector(m_header._shaderModel);
        return reflector.reflect(spirv, stream);
    }
#endif //USE_SPIRV
    case renderer::ShaderHeader::ShaderModel::HLSL_6_1:
    case renderer::ShaderHeader::ShaderModel::HLSL_6_6:
    {
        ShaderReflectionDXC reflector;
        return reflector.reflect(shader, stream);
    }

    default:
        ASSERT(false, "shader model is not supported");
        return false;
    }

    return false;
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