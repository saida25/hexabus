# -*- mode: cmake; -*-
project(hexanode)
cmake_minimum_required(VERSION "2.8" FATAL_ERROR)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

if( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
   message(STATUS "Do not run cmake in the source directory")
   message(STATUS "create an extra binary directory")
   message(FATAL_ERROR "Exiting cmake here.")
endif( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

SET(ENABLE_LOGGING 1)

# use ctest
#ENABLE_TESTING()

set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 0)

# add a path where some libraries might be stored
set(CMAKE_ADDITIONAL_PATH "$ENV{CMAKE_ADDITIONAL_PATH}" CACHE PATH "Path where many locally installed libraries can be found")

###################################################################################################
# No user settings below
###################################################################################################

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

# Detect the system we're compiling on
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(HAS_MACOS 1)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(HAS_MACOS 0)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(HAS_LINUX 1)
  SET(__LINUX_ALSA__ 1)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(HAS_LINUX 0)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wno-non-virtual-dtor -g -fomit-frame-pointer")

set(CMAKE_CXX_FLAGS_EXPERIMENTAL "-O0 -g -ggdb -Wall -Werror -W -Wshadow")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused-variable")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused-parameter")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused-function")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wno-system-headers")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wno-non-virtual-dtor")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -pg -fprofile-generate ")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -fprofile-arcs -ftest-coverage")
# gprof and gcov support
set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -W -Wreturn-type -Wno-shadow")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wunused-variable -Wunused-parameter")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wunused-function -Wunused")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Woverloaded-virtual -Wno-system-headers")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-non-virtual-dtor -pg -fprofile-arcs")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -ftest-coverage")

# Releases are made with the release build. Optimize code.
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-non-virtual-dtor -Werror")

# allways set these flags
set(CMAKE_CXX_FLAGS "-Wall ")

# Check for some compiler stuff
include(CMakeBackwardCompatibilityCXX)

# find libklio
#find_package(Libklio)
#if( NOT LIBKLIO_FOUND)
#  message(FATAL_ERROR "libklio is required.You may set LIBKLIO_HOME")
#endif( NOT LIBKLIO_FOUND)

FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(CppNetlib REQUIRED)
FIND_PACKAGE(LibHexabus REQUIRED)
#FIND_PACKAGE(RtMidi)
# We depend on boost >= 1.46.1 because of the updated filesystem
# library.
SET(Boost_ADDITIONAL_VERSIONS "1.46.1" "1.47" "1.47.1" "1.48.0" "1.48"
  "1.49.0" "1.49" "1.50.0" "1.50")
SET(Boost_DETAILED_FAILURE_MSG true)
FIND_PACKAGE(Boost 1.46.1 REQUIRED COMPONENTS 
  test_exec_monitor program_options filesystem system thread)

# Reflect the package structure
add_subdirectory(libhexanode)
add_subdirectory(RtMidi)
add_subdirectory(src)

# add some files to the installation target
INSTALL(FILES README.md LICENSE.txt DESTINATION share/doc/hexanode)

# CPACK packaging
INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_GENERATOR "DEB" "STGZ" "TGZ" "TBZ2")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Mathias Dalheimer")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "hexanode")
SET(CPACK_PACKAGE_VENDOR "Fraunhofer ITWM")
set(CPACK_PACKAGE_CONTACT "support@mysmartgrid.de")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "support@mysmartgrid.de")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${V_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${V_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${V_PATCH})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CPACK_PACKAGE_EXECUTABLES "hexanode" "A virtual hexabus device.")
SET(CPACK_SOURCE_STRIP_FILES "")
SET(CPACK_STRIP_FILES "bin/hexanode")

set(CPACK_SET_DESTDIR On)
set(CPACK_SOURCE_STRIP_FILES "")
set(CPACK_GENERATOR "DEB;TBZ2")
set(CPACK_SECTION "net")

INCLUDE(CPack)

### print some output for the user
message("")
message("***** Build configuration *****")
message("  install prefix: ${CMAKE_INSTALL_PREFIX}")
message("  hexabus include: ${HXB_INCLUDE_DIR}")
message("  hexabus library: ${HXB_LIBRARIES}")
message("  boost include: ${Boost_INCLUDE_DIRS}")
message("  boost libraries: ${Boost_LIBRARIES}")
message("  extended logging: ${ENABLE_LOGGING}")

