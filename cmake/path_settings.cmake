function(configure_mt)
	if (NOT PLATFORM_FREEBSD)
		message(STATUS "Platform is not freebsd -> Skipping configure mt")
		return()
	endif()

	message(STATUS "Configuring for mt")
	if (NOT TEST_EXECUTABLE)
		add_compile_definitions(
			__MOBICORE__
		)
	endif()

	#mt src paths: ADJUST THESE LINES @deprecated
	#[===[
	set(MAIN_SRC_DIR "/usr/home/src/server" CACHE STRING "Mt main source directory")
	set(MT_DIRS
		"/usr/home/src/extern/include"	# extern include dir
		"${MAIN_SRC_DIR}/common"			# common folder
		"${MAIN_SRC_DIR}/game/src"			# game src dir
		"${MAIN_SRC_DIR}/liblua/include"	# 5.0 etc
		"${MAIN_SRC_DIR}/libsql"			# libsql
	)
	
	foreach(mt_dir IN LISTS MT_DIRS)
		if(EXISTS ${mt_dir})
			target_include_directories(${TARGET_NAME} PUBLIC ${mt_dir})
			message(STATUS "Added mt include directory: ${mt_dir}")
		else()
			message(WARNING "mt directory not found: ${mt_dir}")
		endif()
	endforeach()
	]===]
	target_include_directories(${TARGET_NAME} SYSTEM PUBLIC
		/usr/include/
		/usr/local/include/
	)
endfunction()