set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(SRC_FILES
        SgVectorUtil.cpp
        FileUtil.cc
        SgFastLog.cpp
        SgRandom.cpp
        SgUUID.cpp
        StringUtil.cc )

include_directories(.
        ${PROJECT_SOURCE_DIR}
        ../platform)

add_library(unreallib ${SRC_FILES})

target_link_libraries(unreallib
        uuid
        libboost_system.a
        libboost_thread.a
        libboost_filesystem.a )