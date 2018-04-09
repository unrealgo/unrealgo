set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(./ ${PROJECT_SOURCE_DIR})

function(ADD_LIBRARY_WITH_PROTOBUF)
    cmake_parse_arguments(_AT "" "TARGET" "SOURCES;OBJECTS;LIBS;DATA;DEPENDS" ${ARGN})

    RELATIVE_PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS
            ${PROJECT_SOURCE_DIR} ${_AT_DATA})

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    add_library(${_AT_TARGET}
            ${_AT_SOURCES}
            ${PROTO_SRCS}
            ${PROTO_HDRS})
    target_link_libraries(${_AT_TARGET} TensorflowCC::Shared)
endfunction()

file(GLOB_RECURSE tf_protos_cc_srcs RELATIVE ${PROJECT_SOURCE_DIR}
        "${PROJECT_SOURCE_DIR}/msg/command.proto")

set(CC_SRCS
        MinioStub.h
        ZmqUtil.h
        MinioStub.cc
        ZmqUtil.cpp
        )

ADD_LIBRARY_WITH_PROTOBUF(
        TARGET msgif
        SOURCES ${CC_SRCS}
        DATA ${tf_protos_cc_srcs}
)

#[[set(SRC_FILES
        MinioStub.cc
        ZmqUtil.cpp)

if(${PROTOBUF_VERSION} STREQUAL 3003000)
    set(SRC_FILES ${SRC_FILES} command-3.3.pb.cc)
elseif(${PROTOBUF_VERSION} STREQUAL 3004000)
    set(SRC_FILES ${SRC_FILES} command-3.4.pb.cc)
elseif(${PROTOBUF_VERSION} STREQUAL 3005000)
    set(SRC_FILES ${SRC_FILES} command-3.5.pb.cc)
endif()

add_library(msgif ${SRC_FILES})]]

#if(LINK_SHARED_TENSORFLOW)
#    target_link_libraries(zmqprotobuf TensorflowCC::Shared)
#else()
#    target_link_libraries(zmqprotobuf TensorflowCC::Static)
#endif()

# target_link_libraries(msgif TensorflowCC::Shared)