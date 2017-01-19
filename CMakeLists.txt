project(vfs.ipfs)

cmake_minimum_required(VERSION 2.6)

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(p8-platform REQUIRED)
find_package(kodiplatform REQUIRED)

include_directories(${KODI_INCLUDE_DIR}
                    ${kodiplatform_INCLUDE_DIRS}
                    ${p8-platform_INCLUDE_DIRS}
)

set(IPSF_SOURCES src/addon.cpp)

set(DEPLIBS ${kodiplatform_LIBRARIES})

build_addon(vfs.ipfs IPSF DEPLIBS)
