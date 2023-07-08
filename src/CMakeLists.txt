cmake_minimum_required(VERSION 3.15)
project(protoFIO)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.cc)

add_library(protoFIO STATIC ${SRC})
