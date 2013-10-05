##############################
#
# WELCOME
#
# This is the main cmake configuration file
# associate to your project
# You can define here specific compilation options
#
# More about cmake : www.cmake.org
#
# Using cmake command line :
# 1. Create a build directory : mkdir build
# 2. Configure cmake project : cd build; cmake ..
#   You can also generate project with your favorite IDE :
#   Example : Under Mac osx use "cmake .. -G XCode"
#   see cmake help for more information
# 3. Edit project option : ccmake .
# 4. Build it : make

############################ PROJECT NAME
PROJECT(camera)

############################ VERBOSITY
SET(CMAKE_VERBOSE_MAKEFILE on)

############################ VERSION
# Check cMake version
CMAKE_MINIMUM_REQUIRED( VERSION 2.6.0 )

############################ COLORIZE MAKEFILE
# Colorized output
SET( CMAKE_COLOR_MAKEFILE TRUE )

############################ CHOOSE COMPILE MODE
SET(CMAKE_CXXFLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXXFLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")
SET(CMAKE_BUILD_TYPE "Release")


############################ SUBDIRECTORIES INCLUSION
SET(AGENT_SOURCE_DIR ${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(./)
ADD_SUBDIRECTORY( camera )

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/RoBoIO/LINUX)
#SET_TARGET_PROPERTIES(bioloid PROPERTIES LINKER_LANGUAGE CXX)

ADD_EXECUTABLE(cameratest main.cpp)

TARGET_LINK_LIBRARIES(cameratest
camera
pthread
)
SET( CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin/")
INSTALL(TARGETS cameratest
RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
)
INSTALL(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})
#INSTALL(FILES formation.conf DESTINATION ${CMAKE_INSTALL_PREFIX})
#INSTALL( PROGRAMS start.sh kill.sh goalkeeper.sh oneattacker.sh DESTINATION ${CMAKE_INSTALL_PREFIX})


