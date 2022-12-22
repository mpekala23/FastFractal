cmake_minimum_required(VERSION 2.8)

project( FractalCoding )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

file(GLOB SOURCES
  ../include/*.h
  ../compress.cpp
  ../decompress.cpp
  ../metrics.cpp
)

add_executable( benchmark_file benchmark_file.cpp ${SOURCES} )
target_link_libraries( benchmark_file ${OpenCV_LIBS} Threads::Threads )

#add_executable( benchmark_scales benchmark_scales.cpp ${SOURCES} )
#target_link_libraries( benchmark_scales ${OpenCV_LIBS} Threads::Threads )

#add_executable( benchmark_parallel benchmark_parallel.cpp ${SOURCES} )
#target_link_libraries( benchmark_parallel ${OpenCV_LIBS} Threads::Threads )
