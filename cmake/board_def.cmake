#
# Copyright (c) Ericsson AB 2020, all rights reserved
#

#message(STATUS "++++++++++ board_def.cmake start.")

if (${BOARD} STREQUAL "nrf9160_pca10090" OR ${BOARD} STREQUAL "nrf9160_pca10090ns")
        message(STATUS "++++++++++ Compiling for Nordic 9160DK")
	set (ARD_BOARD_NAME 9160DevKit)
endif()

if (${BOARD} STREQUAL "nrf9160_pca20035" OR ${BOARD} STREQUAL "nrf9160_pca20035ns")
        message(STATUS "++++++++++ Compiling for Nordic Thingy91")
	set (ARD_BOARD_NAME Thingy91)
endif()

if (${BOARD} STREQUAL "nrf9160_ard0011A" OR ${BOARD} STREQUAL "nrf9160_ard0011Ans")
        message(STATUS "++++++++++ Compiling for Ardesco Prototype.")
	set (ARD_BOARD_NAME Prototype)
endif()

if (${BOARD} STREQUAL "nrf9160_ard0021B" OR ${BOARD} STREQUAL "nrf9160_ard0021Bns")
        message(STATUS "++++++++++ Compiling for Ardesco Combi.")
	set (ARD_BOARD_NAME Combi)
endif()

if (${BOARD} STREQUAL "nrf9160_ard0022B" OR ${BOARD} STREQUAL "nrf9160_ard0022Bns")
        message(STATUS "++++++++++ Compiling for Ardesco Combi Dev.")
	set (ARD_BOARD_NAME Combi_Dev)
endif()

if (${BOARD} STREQUAL "nrf9160_ard0031A" OR ${BOARD} STREQUAL "nrf9160_ard0031Ans")
        message(STATUS "++++++++++ Compiling for Ardesco Mini.")
	set (ARD_BOARD_NAME Combi_Mini)
endif()

if(NOT ARD_BOARD_NAME)
        message(STATUS "++++++++++ Compiling for Unknown board.")
	set (ARD_BOARD_NAME Unknown)
endif()

zephyr_compile_definitions(ARD_BOARD_NAME=${ARD_BOARD_NAME})

#message(STATUS "++++++++++ board name: ${ARD_BOARD_NAME}")
#message(STATUS "++++++++++ board_def.cmake end.")
