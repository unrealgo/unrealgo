set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type -std=c++11")

find_package(TensorflowCC REQUIRED)

SET(SRC_FILES
        Allocator.cpp
        SgDfpnSearch.cpp
        SgEvaluatedMoves.cpp
        SgGameReader.cpp
        SgGameWriter.cpp
        SgGtpClient.cpp
        SgGtpCommands.cpp
        SgGtpUtil.cpp
        SgIncrementalStack.cpp
        SgInit.cpp
        SgMemCheck.cpp
        SgMiaiMap.cpp
        SgMiaiStrategy.cpp
        SgNode.cpp
        SgNodeUtil.cpp
        SgProbCut.cpp
        SgProp.cpp
        SgABSearch.cpp
        SgSearchControl.cpp
        SgSearchStatistics.cpp
        SgSearchTracer.cpp
        SgSearchValue.cpp
        SgStrategy.cpp
        MpiSynchronizer.cpp
        SgTimeControl.cpp
        SgTimeRecord.cpp
        SgTimeSettings.cpp
        UctSearch.cpp
        UctSearchTree.cpp
        UctTreeUtil.cpp
        UctBoardEvaluator.cpp
        UctDeepPlayer.cpp
        UctDeepTrainer.cpp
        UctEvalStatServer.cc)

include_directories(./
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/msg
        ../lib
        ../config
        ../go
        ../gouct
        ../gtpengine)

add_library(search ${SRC_FILES})

TARGET_LINK_LIBRARIES(search
        funcapproximator
        libzmq.a
        gouct
        msgif
        atomnet
        unreallib)

#if(LINK_SHARED_TENSORFLOW)
#    target_link_libraries(search TensorflowCC::Shared)
#else()
#    target_link_libraries(search TensorflowCC::Static)
#endif()

target_link_libraries(search TensorflowCC::Shared)