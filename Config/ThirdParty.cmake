##############################################################
#ThirdParty
##############################################################

set(THIRD_PARTY_LIB_DEFINE "")
set(THIRD_PARTY_LIB_INCLUDE_SUFFIX "")

#Json
set(THIRD_PARTY_LIB_INCLUDE_SUFFIX "${THIRD_PARTY_LIB_INCLUDE_SUFFIX}" "json/include")

#DirectXMath
set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_DIRECTX_MATH")

#Crc32
option(CRC32C_BUILD_TESTS "Build CRC32C's unit tests" OFF)
option(CRC32C_BUILD_BENCHMARKS "Build CRC32C's benchmarks" OFF)
option(CRC32C_USE_GLOG "Build CRC32C's tests with Google Logging" OFF)
option(CRC32C_INSTALL "Install CRC32C's header and library" OFF)

#Stb
if(STB_LIB)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_STB")
endif()

#Gli
if(GLI_LIB)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_GLI")
    set(THIRD_PARTY_LIB_INCLUDE_SUFFIX "${THIRD_PARTY_LIB_INCLUDE_SUFFIX}" "glm")
endif()

#Assimp
if(ASSIMP_LIB)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_ASSIMP")
    option(BUILD_SHARED_LIBS "Build package with shared libraries." OFF)
    option(ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." OFF)
    option(ASSIMP_BUILD_ASSIMP_TOOLS "If the supplementary tools for Assimp are built in addition to the library." OFF)
endif()

#HWCPipe
if(HWCPipe_LIB)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_HWC_PIPE")
endif()

#DirectXShaderCompiler
if(DXCompiler_LIB)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_CUSTOM_DXC")
endif()

#Tracy
if(PROFILER_TRACY_LIB)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DTRACY_ENABLE")
    set(THIRD_PARTY_LIB_INCLUDE_SUFFIX "${THIRD_PARTY_LIB_INCLUDE_SUFFIX}" "tracy/public")
    option(TRACY_ON_DEMAND "On-demand profiling" ON)
endif()

#imgui
if(IMGUI_SOURCE)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_IMGUI")
    set(THIRD_PARTY_SOURCE_INCLUDE_SUFFIX "${THIRD_PARTY_SOURCE_INCLUDE_SUFFIX}" "imgui")
endif()