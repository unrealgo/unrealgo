set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -std=c++11")

set(SRC_FILES
        AtomCurlTest.cc )

add_executable(AtomCurlTest ${SRC_FILES})

TARGET_LINK_LIBRARIES(AtomCurlTest
        z
        atomnet )