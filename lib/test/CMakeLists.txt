set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(libTest libTest.cc)

target_link_libraries(libTest
        unreallib)