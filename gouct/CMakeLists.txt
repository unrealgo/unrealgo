set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type -std=c++11")

find_package(TensorflowCC REQUIRED)

SET(SRC_FILES
        GoUctBoard.cpp
        GoUctCommands.cpp
        GoUctDefaultMoveFilter.cpp
        GoUctEstimatorStat.cpp
        GoUctGlobalSearch.cpp
        GoUctMoveFilter.cpp
        GoUctSearchObject.cpp
        GoUctPlayer.cpp
        GoUctPlayoutPolicy.cpp
        GoUctSearch.cpp
        GoUctUtil.cpp)

include_directories(./
        ${PROJECT_SOURCE_DIR}
        ../go/
        ../gtpengine
        ../search/)

add_library(gouct ${SRC_FILES})

#if(LINK_SHARED_TENSORFLOW)
#    target_link_libraries(gouct TensorflowCC::Shared)
#else()
#    target_link_libraries(gouct TensorflowCC::Static)
#endif()

target_link_libraries(gouct
        go
        msgif
        TensorflowCC::Shared)