set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(SRC_FILES
        GtpEngine.cpp
        GtpInputStream.cpp
        GtpOutputStream.cpp
        )

include_directories(./ )

add_library(gtpengine ${SRC_FILES})