cmake_minimum_required(VERSION 2.8)
PROJECT(lib-uvcCapture CXX)

find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})

#ADD_LIBRARY( capture capture.cpp )
ADD_EXECUTABLE(test test1.cpp)

target_link_libraries(test ${ImageMagick_LIBRARIES})
