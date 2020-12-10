# 
# Copyright (c) Ericsson AB 2020, all rights reserved
#
message(STATUS "++++ Processing Ardesco.cmake")

# Set DBG_CMAKE environment variable to help debug cmake process.
if(NOT DEFINED DCM)
        set(DCM $ENV{DBG_CMAKE})
endif()
# -----------------------------------------------------------------------
# Versioning support
# -----------------------------------------------------------------------

if(NOT APP_VERSION)
  set(APP_VERSION "?")
endif()

# These version numbers must match the release version
set (ARDESCO_SDK_VERSION_MAJOR "1")
set (ARDESCO_SDK_VERSION_MINOR "6")
set (ARDESCO_SDK_VERSION_PATCH "0")

#
# if ARDESCO_BASE defined, base everything Ardesco on it.
#
set(AR $ENV{ARDESCO_ROOT})
if(DEFINED AR)
        # Need to make sure ARDESCO_ROOT doesn't have a trailing \
        # Unfortunately, the VS Code extension does this.
        # I'm sure there is an easier way to do this, but this works.
        file(TO_CMAKE_PATH $ENV{ARDESCO_ROOT} ard_test)
        file(TO_NATIVE_PATH ${ard_test} ARDESCO_ROOT)
        set(ENV{ARDESCO_ROOT} ${ARDESCO_ROOT})

        message(STATUS "ARDESCO_ROOT defined as >$ENV{ARDESCO_ROOT}<")
else()
        #
        # ARDESCO_ROOT not defined. Set up based on current build directory
        #
        message(STATUS "ARDESCO_ROOT NOT defined. Setting relative to current build directory.")

        # Define ARDESCO_ROOT relative to the current build directory.
        set(ARDESCO_ROOT ${CMAKE_CURRENT_LIST_DIR})
        set(ENV{ARDESCO_ROOT} ${ARDESCO_ROOT})
endif()

#
# ZEPHYR_BASE is important. If defined as environment variable, 
# use it to locate the NCS and NRF directories. 
# We will then base the NCS versioning values on ZEPHYR_BASE.
# If not defined, set ZEPHYR_BASE relative to ARDESCO_ROOT.
# To find the NCS proper tree, we look in order:
#   1. value of ARD_NCS_DIRECTORY variable
#   2. contents of NCSDIR file in ardesco directory
#
set(ZB $ENV{ZEPHYR_BASE})
if(DEFINED ZB)
        # If ZEPHYR_BASE defined, use it
        message(STATUS "ZEPHYR_BASE is defined as >$ENV{ZEPHYR_BASE}<")

        # Point to NCS root directory above ZEPHYR_BASE.
        set(NCS_DIR $ENV{ZEPHYR_BASE}/..)
else()
        message(STATUS "ZEPHYR_BASE NOT defined. Base on ARDESCO_ROOT")

        #
        # If ARD_NCS_DIRECTORY not defined, read the name of the NCS
        # directory from the NCSDIR file in the ardesco root directory.
        # This will be used to define ZEPHYR_BASE.
        #
        # This can be overridden in the application cmakelist.txt file by
        # setting the ARD_NCS_DIRECTORY value. For example:
        # set(ARD_NCS_DIRECTORY v1.2.0)
        #
        if (NOT DEFINED ARD_NCS_DIRECTORY)
                file(READ ${CMAKE_CURRENT_LIST_DIR}NCSDIR ARD_NCS_DIRECTORY)
                string(STRIP ${ARD_NCS_DIRECTORY} ARD_NCS_DIRECTORY)
                message(STATUS "++++++++ ARD_NCS_DIRECTORY  >${ARD_NCS_DIRECTORY}<")
        endif()

        # Point to NCS root directory.
        set(NCS_DIR ${ARDESCO_ROOT}/${ARD_NCS_DIRECTORY})

        set(ENV{ZEPHYR_BASE} ${ARDESCO_ROOT}/${ARD_NCS_DIRECTORY}/zephyr)
endif()

#
# Attempt to discover NCS version by reading VERSION file in 
# the ncs\nrf directory. 
# If it doesn't exist, see if a VERSION file is in Ardesco Root.
# If neither file exists, assume 1.2.0
#
if (NOT DEFINED NCS_VER)
        if (EXISTS  ${NCS_DIR}/nrf/VERSION)
                file(READ ${NCS_DIR}/nrf/VERSION NCS_VER)
                string(STRIP ${NCS_VER} NCS_VER)
        else()
                # VERSION file doesn't exist in 1.2 or 1.2.1. 
                # See if we have a VERSION file in the Ardesco directory.
                if (EXISTS ${CMAKE_CURRENT_LIST_DIR}VERSION)
                        file(READ ${CMAKE_CURRENT_LIST_DIR}VERSION NCS_VER)
                        string(STRIP ${NCS_VER} NCS_VER)
                else()
                        # Can't find any information, Assume version 1.2
                        set (NCS_VER 1.2.0)
                endif()
        endif()
endif()

# Define NRF version numbers so we can use in code for compat ifdefs
string (SUBSTRING ${NCS_VER} 0 1 NRF_TARG_VER_MAJOR)
string (SUBSTRING ${NCS_VER} 2 1 NRF_TARG_VER_MINOR)
string (SUBSTRING ${NCS_VER} 4 1 NRF_TARG_VER_PATCH)

#
# Because the Board directories have incompatible settings between versions
# we need to have different board directories for different versions. For
# example there are incompatibilties between v1.2.x and v1.3.0.
# 
# Form the name of the Ardesco specific files from the target version.
# We first look for a directory using the patch value such as ard1.2.1 but
# if that doesn't exist, look for the more general ard1.2 directory to cover
# both 1.2.0 and 1.2.1. If that doesn't exist, assume ard1.2
#
if (NOT DEFINED ARDEXT_DIR)
        set (ARDEXT_DIR ard${NCS_VER})
        # check to see if folder exists such as 1.3.0
        if (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${ARDEXT_DIR})
                # Directory with patch included doesn't exist.
                # Strip the patch number and try again.
                string (SUBSTRING ${ARDEXT_DIR} 0 6 ARDEXT_DIR)
                if (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${ARDEXT_DIR})
                        # Neither exist, just use ard1.2 directory
                        set (ARDEXT_DIR  ard1.2)
                        message (STATUS "++++   ARDEXT_DIR directory assumed to be ${ARDEXT_DIR}")       
                        endif()
        endif()
else()
message (STATUS "++++  ARDEXT_DIR directory overridden to ${ARDEXT_DIR}")       
endif()

# -----------------------------------------------------------------------
# Establish basic directory structure
# -----------------------------------------------------------------------

# Point to NCS root directory.
set (NCS_ROOT ${NCS_DIR})
set (NRF_DIR ${NCS_DIR}/nrf)

#
# Dump some of the basic build info
#
if(DEFINED DCM)
if($<EQUAL:${DCM},"enable">)

message(STATUS "++++  ARDESCO_ROOT      ${ARDESCO_ROOT}")
message(STATUS "++++  NCS_DIR           ${NCS_DIR}")
message(STATUS "++++  ZEPHYR_BASE       ${ZEPHYR_BASE}")
message(STATUS "++++  ARDEXT_DIR        ${ARDEXT_DIR}")
message(STATUS "++++  board root dir    ${ARDESCO_ROOT}/${ARDEXT_DIR}")
message(STATUS "++++  CMAKE_SOURCE_DIR  ${CMAKE_SOURCE_DIR}")
message(STATUS "++++  NRF_TARG_VER_MAJOR  ${NRF_TARG_VER_MAJOR}")
message(STATUS "++++  NRF_TARG_VER_MINOR  ${NRF_TARG_VER_MINOR}")
message(STATUS "++++  NRF_TARG_VER_PATCH  ${NRF_TARG_VER_PATCH}")

endif()
endif()

# -----------------------------------------------------------------------
# Below adapted from NRF boilerplate.cmake
# -----------------------------------------------------------------------
if(NOT BOARD)
        set(BOARD $ENV{BOARD})
endif()

# Check if selected board is supported.
if(DEFINED ARD_SUPPORTED_BOARDS)
        if(NOT BOARD IN_LIST ARD_SUPPORTED_BOARDS)
                message(FATAL_ERROR "Ardesco board ${BOARD} is not supported")
        endif()
endif()

# Check if selected build type is supported.
if(DEFINED ARD_SUPPORTED_BUILD_TYPES)
        if(NOT CMAKE_BUILD_TYPE IN_LIST ARD_SUPPORTED_BUILD_TYPES)
                message(FATAL_ERROR "${CMAKE_BUILD_TYPE} variant is not supported in Ardesco")
        endif()
endif()

# -----------------------------------------------------------------------
# Extend the build environment to include Ardesco specific 
# board and DTS directories. Because the syntax changes between
# versions of NCS and Zephyr, we have version specific board
# directorys for 1.2, 1.3...
# -----------------------------------------------------------------------


# Add ARDESCO_ROOT as a BOARD_ROOT in case the board is an Ardesco specific board
list(APPEND BOARD_ROOT ${ARDESCO_ROOT}/${ARDEXT_DIR})

# Add NRF_DIR as a DTS_ROOT to include ardesco/dts
list(APPEND DTS_ROOT ${ARDESCO_ROOT}/${ARDEXT_DIR})

# -----------------------------------------------------------------------
# Add external modules.
#   We add the secureboot module so that 9160 apps can build with secure bootloader.
#   TODO:: Need to add ifdef for M33 build vs M4
# -----------------------------------------------------------------------
#set(ZEPHYR_EXTRA_MODULES
#       ${ARDESCO_ROOT}/modules/secure_apps)

#
# Define root of ardesco libraries
#
set(ARDESCO_LIB_DIR ${ARDESCO_ROOT}/lib)
set(ARDESCO_DRIVER_DIR ${ARDESCO_ROOT}/drivers)

# -----------------------------------------------------------------------
# Continue build with standard NRF build tree
# Since these steps differ for different versions of 
# the NCS tree, include a version specific cmake.
# -----------------------------------------------------------------------

# Include custom steps that differ between NCS versions.
if (EXISTS ${ARDESCO_ROOT}/${ARDEXT_DIR}/ardescoext.cmake)
include(${ARDESCO_ROOT}/${ARDEXT_DIR}/ardescoext.cmake)
else()
message (STATUS "---------------------")
message(FATAL_ERROR "Ardesco Compat cmake file >${ARDESCO_ROOT}/${ARDEXT_DIR}/ardescoext.cmake< not found.")
message (STATUS "-----------------------")
endif()

# -----------------------------------------------------------------------
# Add Adresco include dir in the default include directories
# -----------------------------------------------------------------------

# Add the src directory to look for .h files first before ours.
zephyr_include_directories(${CMAKE_SOURCE_DIR}/src)

zephyr_include_directories(${ARDESCO_ROOT})
zephyr_include_directories(${ARDESCO_ROOT}/include)
zephyr_include_directories(${ARDESCO_ROOT}/drivers)

# Include cmake file to set board string name
include(${ARDESCO_ROOT}/cmake/board_def.cmake)

# Indicate target NRF version 
zephyr_compile_definitions(NRF_VERSION_MAJOR=${NRF_TARG_VER_MAJOR})
zephyr_compile_definitions(NRF_VERSION_MINOR=${NRF_TARG_VER_MINOR})
zephyr_compile_definitions(NRF_VERSION_PATCH=${NRF_TARG_VER_PATCH})

# reflect application verison
zephyr_compile_definitions(APP_VERSION=${APP_VERSION})
