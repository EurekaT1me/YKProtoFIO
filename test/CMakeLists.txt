cmake_minimum_required(VERSION 3.15)
project(protoFIO)

SET(testName functionalTest)
file(GLOB SRC ${PROJECT_SOURCE_DIR}/${testName}.cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

link_libraries(protoFIO)
link_directories(${LIBPATH})

add_executable(${testName} ${SRC})