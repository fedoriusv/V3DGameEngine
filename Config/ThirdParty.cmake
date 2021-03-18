#ThirdParty
##############################################################

option(STB_HEADERS "Use Stb headers" ON)
#https://github.com/nothings/stb.git

option(GIL_HEADERS "Use gli headers. Uses glm" ON)
#https://github.com/g-truc/gli.git
#https://github.com/g-truc/glm.git

##############################################################

set(THIRD_PARTY_LIB_DEFINE "")
set(THIRD_PARTY_LIB_INCLUDE_SUFFIX "")

#Stb
if(STB_HEADERS)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_STB")
endif()

#Gli
if(GIL_HEADERS)
    set(THIRD_PARTY_LIB_DEFINE "${THIRD_PARTY_LIB_DEFINE} -DUSE_GLI")
    set(THIRD_PARTY_LIB_INCLUDE_SUFFIX "${THIRD_PARTY_LIB_INCLUDE_SUFFIX}"  "glm")
endif()