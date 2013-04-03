cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
find_package( OpenCV REQUIRED )

add_executable( binmorph binmorph.cpp )
target_link_libraries( binmorph ${OpenCV_LIBS} )

add_executable( delta delta.cpp )
target_link_libraries( delta ${OpenCV_LIBS} )

add_executable( binthresh binthresh.cpp )
target_link_libraries( binthresh ${OpenCV_LIBS} )

add_executable( detect detect.cpp )
target_link_libraries( detect ${OpenCV_LIBS} )

add_executable( cc cc.cpp )
target_link_libraries( cc ${OpenCV_LIBS} )
