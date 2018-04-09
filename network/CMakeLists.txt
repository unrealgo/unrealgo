set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -std=c++11")

set(SRC_FILES
        zip.c
        unzip.c
        ioapi.c
        AtomZIP.cc
        AtomUNZIP.cc
        CCZLib.cc
        AtomHash.cc
        AtomCurl.cc)

include_directories(${PROJECT_SOURCE_DIR})

add_library(atomnet ${SRC_FILES})

TARGET_LINK_LIBRARIES(atomnet
        z
        curl
        crypto)