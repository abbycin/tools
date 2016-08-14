cmake_minimum_required(VERSION 2.6)

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_library(logger SHARED 
        file.cpp
        logbase.cpp
        logging.cpp
        scopedlog.cpp
        stream_wrapper.cpp
        timefmt.cpp)
