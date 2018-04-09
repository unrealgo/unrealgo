set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

SET ( SRC_FILES
        ../go/test/GoBoardTest.cpp
        ../go/test/GoBoardTest2.cpp
        ../go/test/GoBoardTestLiberties.cpp
        ../go/test/GoBoardSynchronizerTest.cpp
        ../go/test/GoBoardUpdaterTest.cpp
        ../go/test/GoBoardUtilTest.cpp
        ../go/test/GoEvalArrayTest.cpp
        ../go/test/GoEyeUtilTest.cpp
        ../go/test/GoGameTest.cpp
        ../go/test/GoGtpCommandUtilTest.cpp
        ../go/test/GoGtpEngineTest.cpp
        ../go/test/GoInfluenceTest.cpp
        ../go/test/GoKomiTest.cpp
        ../go/test/GoOpeningKnowledgeTest.cpp
        ../go/test/GoRegionTest.cpp
        ../go/test/GoRegionBoardTest.cpp
        ../go/test/GoSetupUtilTest.cpp
        ../go/test/GoTimeControlTest.cpp
        ../go/test/GoUtilTest.cpp
        ../gouct/test/GoUctBoardTest.cpp
        ../gouct/test/GoUctUtilTest.cpp
        ../gtpengine/test/GtpEngineTest.cpp
        ../search/test/SgArrayTest.cpp
        ../search/test/SgArrayListTest.cpp
        ../search/test/SgBlackWhiteTest.cpp
        ../search/test/SgBoardColorTest.cpp
        ../search/test/SgBoardConstTest.cpp
        ../search/test/SgBWArrayTest.cpp
        ../search/test/SgBWSetTest.cpp
        ../search/test/SgConnCompIteratorTest.cpp
        ../search/test/SgEBWArrayTest.cpp
        ../search/test/SgEvaluatedMovesTest.cpp
        ../search/test/SgFastLogTest.cpp
        ../search/test/SgGameReaderTest.cpp
        ../search/test/SgGtpUtilTest.cpp
        ../search/test/SgHashTest.cpp
        ../search/test/SgMarkerTest.cpp
        ../search/test/SgMathTest.cpp
        ../search/test/SgMiaiMapTest.cpp
        ../search/test/SgMiaiStrategyTest.cpp
        ../search/test/SgNbIteratorTest.cpp
        ../search/test/SgNodeTest.cpp
        ../search/test/SgNodeUtilTest.cpp
        ../search/test/SgPointArrayTest.cpp
        ../search/test/SgPointTest.cpp
        ../search/test/SgPointSetTest.cpp
        ../search/test/SgPropTest.cpp
        ../search/test/SgRandomTest.cpp
        ../search/test/SgRectTest.cpp
        ../search/test/SgRestorerTest.cpp
        ../search/test/SgSearchTest.cpp
        ../search/test/SgSortedArrayTest.cpp
        ../search/test/SgSortedMovesTest.cpp
        ../search/test/SgStackTest.cpp
        ../search/test/SgStatisticsTest.cpp
        ../search/test/SgStringUtilTest.cpp
        ../search/test/SgSystemTest.cpp
        ../search/test/SgTimeControlTest.cpp
        ../search/test/SgTimeSettingsTest.cpp
        ../search/test/UctSearchTest.cpp
        ../search/test/UctTreeTest.cpp
        ../search/test/UctTreeUtilTest.cpp
        ../search/test/UctValueTest.cpp
        ../search/test/SgUtilTest.cpp
        ../search/test/SgVectorTest.cpp
        ../search/test/SgVectorUtilTest.cpp
        ../search/test/SgWriteTest.cpp
        ../unittestmain/UnitTestMain.cpp
)

include_directories(./
        ${PROJECT_SOURCE_DIR}
        ../go
        ../gouct
        ../gtpengine
        ../search )

add_executable(unittestmain ${SRC_FILES})

TARGET_LINK_LIBRARIES( unittestmain
        gouct
        board
        platform
        go
        search
        gtpengine
        funcapproximator
        boost_program_options
        boost_system
        boost_thread
        boost_filesystem
        boost_unit_test_framework )