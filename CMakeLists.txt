cmake_minimum_required(VERSION 3.7)
project(static_analysis_is_feeble)

file(GLOB SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(demo_src_file ${SRC_FILES})
    string(REPLACE ".cpp" "" demo_name ${demo_src_file})
    add_executable(${demo_name} ${demo_src_file})
endforeach()
