# Christopher Patton
# John Muir Institute for the Environment
# University of California, Davis
# June 2012

# This is the root ITK CMakeLists file.
cmake_minimum_required(VERSION 2.4)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)


# This project is designed to be built outside the Insight source tree.
project(salamander)

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(binmorph binary_morphology.cxx)
add_executable(threshold threshold.cxx)
add_executable(binthresh binary_threshold.cxx)
add_executable(filter filter.cxx)
add_executable(detect detect.cxx)
add_executable(test test.cxx)
add_library(salamander SHARED files.h
                              chunks.h
                              salamander.h
                              files.cxx
                              chunks.cxx
                              salamander.cxx)
#add_executable(hi hi.cxx)

target_link_libraries(binmorph ${ITK_LIBRARIES} salamander)
target_link_libraries(threshold ${ITK_LIBRARIES} salamander)
target_link_libraries(binthresh ${ITK_LIBRARIES} salamander)
target_link_libraries(filter ${ITK_LIBRARIES} salamander)
target_link_libraries(detect ${ITK_LIBRARIES} salamander)
target_link_libraries(test ${ITK_LIBRARIES} salamander)
#target_link_libraries(hi ${ITK_LIBRARIES} salamander)

install (TARGETS detect binmorph threshold binthresh filter DESTINATION bin)
install (TARGETS salamander DESTINATION lib)
