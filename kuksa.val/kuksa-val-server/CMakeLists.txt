#
# ******************************************************************************
# Copyright (c) 2018-2022 Robert Bosch GmbH and others.
#
# All rights reserved. This configuration file is provided to you under the
# terms and conditions of the Eclipse Distribution License v1.0 which
# accompanies this distribution, and is available at
# http://www.eclipse.org/org/documents/edl-v10.php
#
#  Contributors:
#      Robert Bosch GmbH - initial API and functionality
# *****************************************************************************

cmake_minimum_required(VERSION 3.12.0)
project(kuksa-val VERSION 2.0.1 LANGUAGES CXX)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

# Compiler setting
add_compile_options(-Wfatal-errors)
find_program(CCACHE_BIN ccache)                                                 
if(CCACHE_BIN)                                                                  
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_BIN})             
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_BIN}) 
  message("Use ccache from ${CCACHE_BIN}")
endif()

#####
# Create buildinfo
#
set(PRE_CONFIGURE_FILE "buildinfo.h.in")
set(POST_CONFIGURE_FILE "buildinfo.h")
include(git_watcher.cmake)

######
# Root CMake file responsible for setting up mandatory dependencies
# and any other global information

###
# Default option setup

# If not provided otherwise, set build type as 'Release'
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type" FORCE)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


###
# System library includes

# Prefer pthread as threading library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Boost
include(boost.cmake)
include(grpc.cmake)

# OpenSSL
find_package(OpenSSL REQUIRED)
message(STATUS "openssl libraries ${OPENSSL_LIBRARIES}")

FIND_PACKAGE(PkgConfig)

# mosquitto
find_path(MOSQUITTO_INCLUDE_DIR 	NAMES mosquitto.h ) 
find_library(MOSQUITTO_LIBRARY 	NAMES mosquitto )
include(FindPackageHandleStandardArgs) 
# handle the QUIETLY and REQUIRED arguments and set VILLASNODE_FOUND to TRUE 
# if all listed variables are TRUE 
find_package_handle_standard_args(Mosquitto DEFAULT_MSG MOSQUITTO_LIBRARY MOSQUITTO_INCLUDE_DIR)

###
# 3rd-party library includes already available in git repo

add_library(jsoncons INTERFACE)
target_include_directories(jsoncons SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party-libs/jsoncons/include/)

add_library(jsonpath INTERFACE)
target_include_directories(jsonpath SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party-libs/jsoncons/include/jsoncons_ext)

add_library(jwt-cpp INTERFACE)
target_include_directories(jwt-cpp SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party-libs/jwt-cpp/include)

add_library(turtle INTERFACE)
target_include_directories(turtle SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party-libs/turtle/include)

###
# codecov
if (ENABLE_COVERAGE)
  FetchContent_Declare(
    cmake-codecov
    GIT_REPOSITORY https://github.com/RWTH-HPC/CMake-codecov.git
    GIT_TAG        1974181d8515441329e3a03ccf47a99518402491
  )
  FetchContent_Populate(cmake-codecov)
  set(CMAKE_MODULE_PATH "${cmake-codecov_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
  set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
  find_package(codecov)
endif()

###
# Add project subdirectories to build

add_subdirectory(src)
enable_testing()
include(CTest)
add_subdirectory(test/unit-test)


###
# codecov
if (ENABLE_COVERAGE)
  list(APPEND LCOV_REMOVE_PATTERNS "'/usr/*'")
  coverage_evaluate()
endif()
