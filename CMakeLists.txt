cmake_minimum_required(VERSION 3.9)
project(CRF_Rescheduling)

set(CMAKE_CXX_STANDARD 11)

add_executable(Scheduling main.cpp db_interface.h planning_horizon.h csv_utils.h debug_utils.h events.h string_utils.h component.h problem.h problem.cpp LocalSearch.h LocalSearch.cpp debug_utils.cpp string_utils.cpp planning_horizon.cpp truck.h)

#CMAKE TWEAKS
set(CMAKE_CXX_STANDARD 11)

MESSAGE("BUILD TYPE:        " ${CMAKE_BUILD_TYPE})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-no-pie -g -pg -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_WINDOWSRELEASE "-O3 -static -static-libgcc -static-libstdc++")

#Set WindowsRelease Custom Commands
if (CMAKE_BUILD_TYPE STREQUAL "WindowsRelease")
    MESSAGE("Setting Windows Release Flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_WINDOWSRELEASE}")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.exe")
endif (CMAKE_BUILD_TYPE STREQUAL  "WindowsRelease")


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")





#Generic Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lmysqlclient -lz")

target_link_libraries(Scheduling mysqlclient)


MESSAGE("COMPILER FLAGS:" ${CMAKE_CXX_FLAGS})
MESSAGE("TARGET:" ${TARGET})
#MESSAGE("COMPILER LINK_EXECUTABLE FLAGS:" ${CMAKE_CXX_LINK_EXECUTABLE})
#MESSAGE("COMPILER EXE_LINKER_FLAGS:" ${CMAKE_EXE_LINKER_FLAGS})
