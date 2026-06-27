set(FID_SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/src/fid.c)
set(FID_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/inc)

if(DEFINED FID_LEVEL)
	message(STATUS "FID_LEVEL: ${FID_LEVEL}")
	add_definitions(-DFID_LEVEL=${FID_LEVEL})
else()
	message(FATAL_ERROR "FID_LEVEL is not defined")
endif()