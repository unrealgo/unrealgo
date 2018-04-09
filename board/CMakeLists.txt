set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(SRC_FILES
        GoBoardConst.cpp
        SgUtil.cpp
        GoPoint.cpp
        GoPointSet.cpp
        GoPointSetUtil.cpp
        SgWrite.cpp
        GoRect.cpp
        GoConnCompIterator.cpp
        GoNbIterator.cpp)

include_directories(.
        ${PROJECT_SOURCE_DIR}
        ../platform)

add_library(board ${SRC_FILES})