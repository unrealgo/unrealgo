set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


function(addTest)
    cmake_parse_arguments(_AT "" "TARGET" "SOURCES;LIBS;DEPENDS" ${ARGN})
    add_executable(${_AT_TARGET} ${_AT_SOURCES})

    if (_AT_DEPENDS)
        add_dependencies(${_AT_TARGET} ${_AT_DEPENDS} googletest)
    endif ()

    target_link_libraries(${_AT_TARGET} ${_AT_LIBS})
endfunction()


SET(BASE_SRC_FILES
        ../DlTfStatusHelper.cc
        ../DlTFRecordWriter.cc
        ../DlPyRecordReader.cc
        ../DlPyRecordWriter.cc
        ../DlTensorUtil.cc
        ../DlGraphUtil.cc
        ../DlTFNetworkEvaluator.cc
        ../DlConfig.cc)

include_directories(./ ../ ${PROJECT_SOURCE_DIR})

addTest(
        TARGET DlTFEvalTest
        SOURCES ${BASE_SRC_FILES} DlTFTest.cc
        LIBS TensorflowCC::Shared funcapproximator
)

addTest(
        TARGET DlTFVariableBatchSize
        SOURCES ${BASE_SRC_FILES} DlTFTestVariableBatchSize.cc
        LIBS TensorflowCC::Shared funcapproximator
)

addTest(
        TARGET DlGraphTest
        SOURCES ${BASE_SRC_FILES} DlGraphTest.cc
        LIBS TensorflowCC::Shared funcapproximator
)

addTest(
        TARGET DlGraphPruneTest
        SOURCES ${BASE_SRC_FILES} DlGraphPruneTest.cc
        LIBS TensorflowCC::Shared funcapproximator
)

addTest(
        TARGET DlGraphPruneTest2
        SOURCES ${BASE_SRC_FILES} DlGraphReversePruneTest.cc
        LIBS TensorflowCC::Shared funcapproximator
)

addTest(
        TARGET DlGraphPruneNodesReachableToTest
        SOURCES ${BASE_SRC_FILES} DlGraphPruneNodesReachableToTest.cc
        LIBS TensorflowCC::Shared funcapproximator
)

addTest(
        TARGET DlGraphPruneNodesUnReachableToTest
        SOURCES ${BASE_SRC_FILES} DlGraphPruneNodesUnReachableToTest.cc
        LIBS TensorflowCC::Shared funcapproximator
)

addTest(
        TARGET DlGraphPruneNodesReachableFromTest
        SOURCES ${BASE_SRC_FILES} DlGraphPruneReachableFromTest.cc
        LIBS TensorflowCC::Shared funcapproximator
)

#add_executable(tensorflowTest ${BASE_SRC_FILES} DlTFTest.cc)
#target_link_libraries( tensorflowTest funcapproximator )
#target_link_libraries(tensorflowTest TensorflowCC::Shared)