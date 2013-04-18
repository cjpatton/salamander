# Christopher Patton
# John Muir Institute for the Environment
# University of California, Davis
# June 2012

cmake_minimum_required(VERSION 2.8)
find_package( OpenCV REQUIRED )

# This project is designed to be built outside the Insight source tree.
project(salamander)

add_executable(binmorph binary_morphology.cpp)
add_executable(binthresh binary_threshold.cpp)
add_executable(segment segment.cpp)
add_executable(filter filter.cpp)
add_executable(detect detect.cpp)
add_executable(test test.cpp)
add_library(salamander SHARED files.h
                              salamander.h
                              files.cpp
                              salamander.cpp
                              chunks.h
                              blobs.h
                              chunks.cpp
                              blobs.cpp)

target_link_libraries(salamander ${OpenCV_LIBS})
target_link_libraries(binmorph ${OpenCV_LIBS} salamander)
target_link_libraries(binthresh ${OpenCV_LIBS} salamander)
target_link_libraries(filter ${OpenCV_LIBS} salamander)
target_link_libraries(segment ${OpenCV_LIBS} salamander)
target_link_libraries(detect ${OpenCV_LIBS} salamander)
target_link_libraries(test ${OpenCV_LIBS} salamander)

#install (TARGETS detect binmorph segment binthresh filter DESTINATION bin)
install (TARGETS salamander DESTINATION lib)
install (TARGETS segment filter detect binmorph binthresh DESTINATION bin)
