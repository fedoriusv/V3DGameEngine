#ThirdParty
##############################################################

option(STB_HEADERS "Use Stb headers" ON)
#https://github.com/nothings/stb.git

option(GIL_HEADERS "Use gli headers. Uses glm" ON)
#https://github.com/g-truc/gli.git
#https://github.com/g-truc/glm.git

##############################################################

set(EXTRA_LIB_DEFINE "")
set(EXTRA_LIB_INCLUDE_SUFFIX "")
#Stb
if(STB_HEADERS)
    set(EXTRA_LIB_DEFINE "${EXTRA_LIB_DEFINE} -DUSE_STB")
endif()

#Gli
if(GIL_HEADERS)
    set(EXTRA_LIB_DEFINE "${EXTRA_LIB_DEFINE} -DUSE_GLI")
    set(EXTRA_LIB_INCLUDE_SUFFIX "${EXTRA_LIB_INCLUDE_SUFFIX}"  "glm")
endif()

#Crc32c lib is required"
#https://github.com/google/crc32c.git

option(ASSIMP_LIB "Use Assimp lib" ON)
#https://github.com/assimp/assimp.git

#option(HWCPipe_LIB "Use HWCPipe lib(Only for Android Target)" ON)
#https://github.com/ARM-software/HWCPipe.git

option(SPIRV_INSTALL_FROM_GITHUB "implicity install SpirV" OFF)
