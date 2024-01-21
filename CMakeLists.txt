#---------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(DRS4bin2root)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_PLATFORM_INDEPENDENT_CODE ON)

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
FIND_PACKAGE(ROOT COMPONENTS Geom TMVA Eve Gui RGL EG)
IF (ROOT_FOUND)
    INCLUDE_DIRECTORIES(${ROOT_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${ROOT_INCLUDE_DIR})
    LINK_LIBRARIES(${ROOT_LIBRARIES})
    LINK_DIRECTORIES(${ROOT_LIBRARY_DIR})
ENDIF ()


include_directories(./)

file(GLOB files ./read_v5.3.cpp)

add_executable(DRS4bin2root ${files})

#----------------------------------------------------------------------------
# Install the executable to the directory of CMAKE_INSTALL_PREFIX
#
install(TARGETS DRS4bin2root DESTINATION)
