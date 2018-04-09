set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(SRC_FILES
        GoBensonSolver.cpp
        GoBlock.cpp
        GoBoard.cpp
        GoBoardHistory.cpp
        GoBoardRestorer.cpp
        GoBoardSynchronizer.cpp
        GoBoardUpdater.cpp
        GoBoardUtil.cpp
        GoChain.cpp
        GoEyeCount.cpp
        GoEyeUtil.cpp
        GoGame.cpp
        GoGtpCommandUtil.cpp
        GoGtpEngine.cpp
        GoInfluence.cpp
        GoInit.cpp
        GoKomi.cpp
        GoLadder.cpp
        GoMotive.cpp
        GoNodeUtil.cpp
        GoOpeningKnowledge.cpp
        GoPlayer.cpp
        GoPlayerMove.cpp
        GoRegion.cpp
        GoRegionBoard.cpp
        GoRegionUtil.cpp
        GoRules.cpp
        GoGtpExtraCommands.cpp
        GoSafetyCommands.cpp
        GoSafetySolver.cpp
        GoSafetyUtil.cpp
        GoSearch.cpp
        GoSetupUtil.cpp
        GoStaticLadder.cpp
        GoStaticSafetySolver.cpp
        GoTimeControl.cpp
        GoUtil.cpp
        )


include_directories(./
        ${PROJECT_SOURCE_DIR}
        ../search/
        ../gtpengine/
        ../gouct/)

add_library(go ${SRC_FILES})

#if(LINK_SHARED_TENSORFLOW)
#    target_link_libraries(go TensorflowCC::Shared)
#else()
#    target_link_libraries(go TensorflowCC::Static)
#endif()

target_link_libraries(go
        platform
        TensorflowCC::Shared)