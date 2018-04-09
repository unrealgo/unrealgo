set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(SRC_FILES
        DlTfStatusHelper.cc
        DlTFRecordWriter.cc
        DlPyRecordReader.cc
        DlPyRecordWriter.cc
        DlTensorUtil.cc
        DlGraphUtil.cc
        DlTFNetworkEvaluator.cc
        DlCheckPoint.cc
        DlConfig.cc)

include_directories(./
        ${PROJECT_SOURCE_DIR}
        ../config
        ../lib)

add_library(funcapproximator ${SRC_FILES})
target_link_libraries(funcapproximator unreallib)

#if(LINK_SHARED_TENSORFLOW)
#    target_link_libraries(funcapproximator TensorflowCC::Shared)
#else()
#    target_link_libraries(funcapproximator TensorflowCC::Static)
#endif()

target_link_libraries(funcapproximator TensorflowCC::Shared)