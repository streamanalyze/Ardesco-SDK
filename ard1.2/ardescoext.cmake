# 
# Copyright (c) Ericsson AB 2020, all rights reserved
#

if($<EQUAL:${DCM},"enable">)
message(STATUS "++++ Processing ${CMAKE_CURRENT_LIST_DIR}/Ardescoext.cmake")
endif()

# -----------------------------------------------------------------------
# Continue build with standard NRF build tree
# -----------------------------------------------------------------------

# Include NRF boilerplate
if($<EQUAL:${DCM},"enable">)
message(STATUS "++++ Calling NRF boilerplate.cmake")
endif()
include($ENV{ZEPHYR_BASE}/../nrf/cmake/boilerplate.cmake)

# include Zephyr boilerplate
if($<EQUAL:${DCM},"enable">)
message(STATUS "++++ Calling Zephyr boilerplate.cmake")
endif()
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

