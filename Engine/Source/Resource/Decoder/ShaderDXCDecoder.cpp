#include "ShaderDXCDecoder.h"
#include "Stream/StreamManager.h"
#include "Stream/FileLoader.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Platform.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)
#include "Renderer/D3D12/D3DDebug.h"
#include "ShaderReflectionSpirV.h"
#include "ShaderReflectionDXC.h"

#if USE_CUSTOM_DXC
#   include "dxc/inc/dxcapi.h"
#   include "dxc/inc/d3d12shader.h"
#   pragma comment(lib, "dxcompiler.lib")
#   pragma comment(lib, "dxil.lib")
#else
#   if defined(PLATFORM_WINDOWS)
#       include <dxcapi.h>
#       include <d3d12shader.h>
#       pragma comment(lib, "dxcompiler.lib")
#   elif defined(PLATFORM_XBOX)
#       include <dxcapi_xs.h>
#       include <d3d12shader_xs.h>
#       pragma comment(lib, "dxcompiler_x.lib")
#   endif //PLATFORM
#endif //USE_CUSTOM_DXC

#ifdef USE_SPIRV
#   include "spirv-tools/libspirv.hpp"
#endif //USE_SPIRV

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

static const std::map<std::string, renderer::ShaderType> k_HLSL_ExtensionList =
{
    //hlsl
    { "vs", renderer::ShaderType::Vertex },
    { "ps", renderer::ShaderType::Fragment },
    { "cs", renderer::ShaderType::Compute },
};

static constexpr u32 g_DXDCIndifier = 0x43425844; //DXBC


static bool checkBytecodeSigning(IDxcBlob* bytecode);
#ifdef USE_SPIRV
static bool disassembleSpirv(IDxcBlob* binaryBlob, std::string& assembleSPIV);
#endif //USE_SPIRV

ShaderDXCDecoder::ShaderDXCDecoder(ShaderCompileFlags compileFlags) noexcept
    : m_compileFlags(compileFlags)
{
}

ShaderDXCDecoder::ShaderDXCDecoder(const std::vector<std::string>& supportedExtensions, ShaderCompileFlags compileFlags) noexcept
    : ShaderDecoder(supportedExtensions)
    , m_compileFlags(compileFlags)
{
}

ShaderDXCDecoder::ShaderDXCDecoder(std::vector<std::string>&& supportedExtensions, ShaderCompileFlags compileFlags) noexcept
    : ShaderDecoder(std::move(supportedExtensions))
    , m_compileFlags(compileFlags)
{
}

Resource* ShaderDXCDecoder::decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags, const std::string& name) const
{
    if (!stream || stream->size() == 0)
    {
        LOG_ERROR("ShaderDXCDecoder::decode the stream is empty");
        return nullptr;
    }

    stream->seekBeg(0);

    auto isHLSL_ShaderModel6 = [](renderer::ShaderModel model) -> bool
    {
        return model == renderer::ShaderModel::Default || (model >= renderer::ShaderModel::HLSL_6_0 && model <= renderer::ShaderModel::HLSL_6_6);
    };

    const renderer::Shader::LoadPolicy& shaderPolicy = *static_cast<const renderer::Shader::LoadPolicy*>(policy);
    if (!isHLSL_ShaderModel6(shaderPolicy.shaderModel))
    {
        LOG_ERROR("ShaderDXCDecoder::decode support HLSL SM6.0 and above");
        return nullptr;
    }

    if (shaderPolicy.content == renderer::ShaderContent::Source)
    {
        std::string source;
        source.resize(stream->size());
        stream->read(source.data(), stream->size());
#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        IDxcBlob* binaryShader = nullptr;
        if (!ShaderDXCDecoder::compile(source, shaderPolicy, m_compileFlags, binaryShader, name))
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

        resourceBinary->write<renderer::ShaderType>(shaderPolicy.type);
        resourceBinary->write<renderer::ShaderModel>(shaderPolicy.shaderModel);
        resourceBinary->write(shaderPolicy.entryPoint);
        resourceBinary->write<u32>(bytecodeSize);
        resourceBinary->write(binaryShader->GetBufferPointer(), bytecodeSize);
        resourceBinary->write<bool>(shaderPolicy.useReflection);

        if (shaderPolicy.useReflection && !ShaderDXCDecoder::reflect(resourceBinary, shaderPolicy, m_compileFlags, binaryShader, name))
        {
            LOG_ERROR("ShaderDXCDecoder::decode: reflect is failed");

            binaryShader->Release();
            stream::StreamManager::destroyStream(resourceBinary);

            return nullptr;
        }
        binaryShader->Release();

        renderer::Shader::ShaderHeader shaderHeader(shaderPolicy.type);
        resource::ResourceHeader::fill(&shaderHeader, name, resourceBinary->size(), 0);

        Resource* resource = V3D_NEW(renderer::Shader, memory::MemoryLabel::MemoryObject)(shaderHeader);
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

bool ShaderDXCDecoder::compile(const std::string& source, const renderer::Shader::LoadPolicy& policy, ShaderCompileFlags flags, IDxcBlob*& shader, const std::string& name)
{
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

    class DXCIncludeHandler : public IDxcIncludeHandler
    {
    public:

        DXCIncludeHandler(IDxcUtils* utils) noexcept
            : m_DXUtils(utils)
        {
        }

        ~DXCIncludeHandler()
        {
            reset();
        }

        void addIncludePath(const std::string& path)
        {
            m_paths.emplace(path);
        }

        void reset()
        {
            m_paths.clear();
        }

        HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
        {
            std::string fileStr(platform::Platform::wideToUtf8(pFilename));
            if (!stream::FileStream::isExists(fileStr))
            {
                std::set<std::string>::iterator it;
                for (it = m_paths.begin(); it != m_paths.end(); ++it)
                {
                    std::string testFilePath(*it);
                    std::replace(testFilePath.begin(), testFilePath.end(), '/', '\\');
                    testFilePath = testFilePath + fileStr.substr(2);
                    if (stream::FileStream::isExists(testFilePath))
                    {
                        fileStr = testFilePath;
                        break;
                    }
                }
            }

            static const c8 nullStr[] = " ";
            const UINT code = DXC_CP_ACP;
            IDxcBlobEncoding* source = nullptr;
            HRESULT result = m_DXUtils->CreateBlobFromPinned(nullStr, ARRAYSIZE(nullStr), code, &source);
            if (SUCCEEDED(result))
            {
                result = m_DXUtils->LoadFile(platform::Platform::utf8ToWide(fileStr.c_str()).c_str(), nullptr, &source);
            }

            if (FAILED(result))
            {
                return result;
            }
            *ppIncludeSource = source;

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
        {
            if (riid == __uuidof(IUnknown))
            {
                AddRef();
                *ppvObject = (void*)(IUnknown*)this;
                return S_OK;
            }
            else if (riid == __uuidof(IDxcIncludeHandler))
            {
                AddRef();
                *ppvObject = (void*)(IDxcIncludeHandler*)this;
                return S_OK;
            }

            return E_FAIL;
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return 0;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            return 0;
        }

    private:

        IDxcUtils* m_DXUtils;
        std::set<std::string> m_paths;
    };

    const std::wstring entryPoint = std::wstring(policy.entryPoint.cbegin(), policy.entryPoint.cend());
    const std::wstring target = getShaderTarget(policy.type, policy.shaderModel);

    IDxcCompiler3* DXCompiler = nullptr;
    {
        HRESULT result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DXCompiler));
        if (FAILED(result))
        {
            LOG_FATAL("ShaderDXCDecoder DxcCreateInstance can't create IDxcCompiler. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    IDxcUtils* DXUtils = nullptr;
    {
        HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&DXUtils));
        if (FAILED(result))
        {
            LOG_FATAL("ShaderDXCDecoder DxcCreateInstance can't create IDxcUtils. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    std::vector<LPCWSTR> arguments;

    arguments.push_back(L"-E");
    arguments.push_back(entryPoint.c_str());

    arguments.push_back(L"-T");
    arguments.push_back(target.c_str());

    if (flags & ShaderCompileFlag::ShaderCompile_OptimizationFull)
    {
        arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
    }
    else if (flags & ShaderCompileFlag::ShaderCompile_OptimizationPerformance)
    {
        arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL2);
    }
    else if (flags & ShaderCompileFlag::ShaderCompile_OptimizationSize)
    {
        arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL1);
    }
    else
    {
        arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL0);
        arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
        arguments.push_back(DXC_ARG_DEBUG);
        //arguments.push_back(DXC_ARG_SKIP_VALIDATION);
        arguments.push_back(L"-Qembed_debug");
    }
    arguments.push_back(L"-HV");
    arguments.push_back(L"2021");

#ifdef DEBUG
    arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
#else
    arguments.push_back(L"-no-warnings");
#endif
    if (flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
    {
        arguments.push_back(L"-spirv");
    }

    std::vector<std::wstring> defines;
    for (u32 i = 0; i < policy.defines.size(); ++i)
    {
        std::wstring defineArg;
        defineArg.append(std::move(std::wstring(policy.defines[i].first.cbegin(), policy.defines[i].first.cend())));
        defineArg.append(L"=");
        defineArg.append(std::move(std::wstring(policy.defines[i].second.cbegin(), policy.defines[i].second.cend())));

        defines.push_back(std::move(defineArg));

        arguments.push_back(L"-D");
        arguments.push_back(defines[i].c_str());
    }

    DXCIncludeHandler includer(DXUtils);
    std::vector<std::wstring> includes;
    for (u32 i = 0; i < policy.includes.size(); ++i)
    {
        includes.push_back(std::move(std::wstring(policy.includes[i].cbegin(), policy.includes[i].cend())));

        arguments.push_back(L"-I");
        arguments.push_back(includes[i].c_str());
    }

    for (auto& path : policy.paths)
    {
        includer.addIncludePath(path);
    }

    DxcBuffer dxBuffer = {};
    dxBuffer.Ptr = source.c_str();
    dxBuffer.Size = static_cast<u32>(source.size());
    dxBuffer.Encoding = CP_ACP;

    IDxcResult* compileResult = nullptr;
    {
        HRESULT result = DXCompiler->Compile(&dxBuffer, arguments.data(), static_cast<u32>(arguments.size()), &includer, IID_PPV_ARGS(&compileResult));
        if (FAILED(result))
        {
            DXUtils->Release();
            DXCompiler->Release();

            LOG_ERROR("ShaderDXCDecoder Compile can't create IDxcResult. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
            return false;
        }
    }

    {
        IDxcBlobUtf16* compileName = nullptr;
        IDxcBlobUtf8* compileErrors = nullptr;
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

            DXUtils->Release();
            DXCompiler->Release();

            return false;
        }

        shader = compileBinary;
    }

    {
        IDxcBlob* compileBinary = nullptr;
        BOOL presented = compileResult->HasOutput(DXC_OUT_PDB);
        if (presented)
        {
            HRESULT result = compileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&compileBinary), nullptr);
            if (FAILED(result))
            {
                LOG_ERROR("ShaderDXCDecoder GetOutput. Can't get an binary buffer. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
                ASSERT(false, "wrong");
            }

            //TODO: save to file PDB and add compile flag

            if (compileBinary)
            {
                compileBinary->Release();
            }
        }
    }

    {
        IDxcBlob* compileBinary = nullptr;
        BOOL presented = compileResult->HasOutput(DXC_OUT_ROOT_SIGNATURE);
        if (presented)
        {
            HRESULT result = compileResult->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&compileBinary), nullptr);
            if (FAILED(result))
            {
                LOG_ERROR("ShaderDXCDecoder GetOutput. Can't get an binary buffer. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
                ASSERT(false, "wrong");
            }

            //TODO: save to shader, dx only

            if (compileBinary)
            {
                compileBinary->Release();
            }
        }
    }

#if defined(DEBUG)
    {
        if (flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
        {
            std::string disassembleShaderCode;
            if (disassembleSpirv(shader, disassembleShaderCode))
            {
                LOG_DEBUG("Disassemble SPIRV [%s]: \n %s", name.c_str(), disassembleShaderCode.c_str());
            }
        }
        else
        {
            DxcBuffer dxCompiledBuffer = {};
            dxCompiledBuffer.Ptr = shader->GetBufferPointer();
            dxCompiledBuffer.Size = shader->GetBufferSize();
            dxCompiledBuffer.Encoding = CP_ACP;

            IDxcResult* compileResult = nullptr;
            HRESULT result = DXCompiler->Disassemble(&dxCompiledBuffer, IID_PPV_ARGS(&compileResult));
            if (SUCCEEDED(result))
            {
                if (compileResult->HasOutput(DXC_OUT_DISASSEMBLY))
                {
                    IDxcBlobUtf8* disassembleShader = nullptr;
                    HRESULT result = compileResult->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&disassembleShader), nullptr);
                    if (FAILED(result))
                    {
                        LOG_ERROR("ShaderDXCDecoder GetOutput. Can't get an binary buffer. Error: %s", renderer::dx3d::D3DDebug::stringError(result).c_str());
                        ASSERT(false, "wrong");
                    }

                    if (disassembleShader)
                    {
                        std::string disassembleShaderCode(reinterpret_cast<c8*>(disassembleShader->GetBufferSize(), disassembleShader->GetBufferPointer()));
                        LOG_DEBUG("Disassemble [%s]: \n %s", name.c_str(), disassembleShaderCode.c_str());

                        disassembleShader->Release();
                    }
                }
            }

            if (compileResult)
            {
                compileResult->Release();
            }
        }
    }
#endif //DEBUG

    bool useValidator = !(flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV); //Only for DXBC
    if (useValidator)
    {
        IDxcValidator* DXValidator = nullptr;
        {
            HRESULT result = DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&DXValidator));
            if (FAILED(result))
            {
                DXUtils->Release();
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

        if (!checkBytecodeSigning(shader))
        {
            LOG_WARNING("ShaderDXCDecoder Validation: Shader %s is not signed", name.c_str());
        }

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

    DXUtils->Release();
    DXCompiler->Release();

    return true;
}

bool ShaderDXCDecoder::reflect(stream::Stream* stream, const renderer::Shader::LoadPolicy& policy, ShaderCompileFlags flags, IDxcBlob* shader, const std::string& name)
{
    if (flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
    {
#ifdef USE_SPIRV
        std::vector<u32> spirv(shader->GetBufferSize() / sizeof(u32));
        memcpy(spirv.data(), shader->GetBufferPointer(), shader->GetBufferSize());
        ASSERT(spirv[0] == 0x07230203, "invalid spirv magic number in head");

        //TODO: parses twice for HLSL and dublicate all binding.
        ShaderReflectionSpirV reflector(policy.shaderModel);
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