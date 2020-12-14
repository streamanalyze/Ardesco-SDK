# 
# Copyright (c) Ericsson AB 2020, all rights reserved
#

if($<EQUAL:${DCM},"enable">)
message(STATUS "++++ Processing ${CMAKE_CURRENT_LIST_DIR}/Ardescoext.cmake")
endif()

# -----------------------------------------------------------------------
# Continue build with standard NRF build tree
# -----------------------------------------------------------------------

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

