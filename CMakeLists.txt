project(replayTraceMultiThread)
cmake_minimum_required( VERSION 2.8 )

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER g++)

set(SRC  
    ${PROJECT_SOURCE_DIR}/src/replayTraceMultiThread.cpp
)

link_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(src/traceItem)
add_subdirectory(src/utils)

add_definitions(-std=c++11)

add_executable(replayTraceMultiThread src/replayTraceMultiThread.cpp)

target_link_libraries(replayTraceMultiThread traceItem utils)
