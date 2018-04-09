set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

SET(SRC_FILES
        Main.cpp
        MainEngine.cpp
        Version.cpp )

include_directories(./
        ${PROJECT_SOURCE_DIR}
        ../search
        ../gtpengine
        ../gouct
        ../go )

string(TOLOWER ${APP_NAME} executable)
add_executable(${executable} ${SRC_FILES})

target_link_libraries(${executable}
        gouct
        go
        board
        platform
        search
        gtpengine
        funcapproximator)

target_link_libraries(${executable}
        libboost_program_options.a
        libboost_system.a
        libboost_thread.a
        libboost_filesystem.a)

#if(LINK_SHARED_TENSORFLOW)
#    target_link_libraries(${executable} TensorflowCC::Shared)
#else()
#    target_link_libraries(${executable} TensorflowCC::Static)
#endif()

target_link_libraries(${executable} TensorflowCC::Shared)