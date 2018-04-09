set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(SRC_FILES
        SgDebug.cpp
        SgException.cpp
        SgPlatform.cpp
        SgProcess.cpp
        SgTime.cpp
        SgSystem.cpp)

include_directories(.
        ${PROJECT_SOURCE_DIR} )

add_library(platform ${SRC_FILES})

target_link_libraries(platform
        libboost_filesystem.a)