cmake_minimum_required(VERSION 3.15)
project(protoFIO)

include_directories(${PROJECT_SOURCE_DIR}/include)

link_libraries(protobuf)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/deploy/lib)
SET(LIBPATH ${PROJECT_SOURCE_DIR}/deploy/lib)



add_subdirectory(src)
add_subdirectory(test)