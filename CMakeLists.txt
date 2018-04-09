cmake_minimum_required (VERSION 3.2)

set(APP_NAME "UnrealGo")
project (${APP_NAME})

add_definitions(-DAPP_NAME="${APP_NAME}")
add_definitions(-DAPP_VERSION="1.0")
add_definitions(-DCHECKTREECONSISTENCY)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_definitions(-DGO_DEFINE_MAX_SIZE=19)
add_definitions(-DUSE_NNEVALTHREAD)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_definitions(-DHOST_CPU="${CMAKE_SYSTEM_PROCESSOR}")

add_definitions(-DSHOW_BOARD)
add_definitions(-DDEBUG_BOARD)

set(BUILD_STATIC_EXECUTABLES false)

if(BUILD_STATIC_EXECUTABLES)
    set(CMAKE_EXE_LINKER_FLAGS -static)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
    set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC
    set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
else(BUILD_STATIC_EXECUTABLES)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LOFAR_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif(BUILD_STATIC_EXECUTABLES)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(tensorflow)
include(boost)

add_subdirectory (lib)
add_subdirectory (funcapproximator)
add_subdirectory (gtpengine)
add_subdirectory (search)
add_subdirectory (board)
add_subdirectory (go)
add_subdirectory (gouct)
add_subdirectory (platform)
add_subdirectory (engine)
add_subdirectory (unittestmain)
add_subdirectory (network)
add_subdirectory (msg)

add_subdirectory (funcapproximator/test)
add_subdirectory (network/test)
add_subdirectory (lib/test)
