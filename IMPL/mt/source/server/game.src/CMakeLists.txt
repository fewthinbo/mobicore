#===================================================Mobile======================================================
set(IS_64_BIT FALSE)
include(CheckTypeSize)
check_type_size("void*" SIZEOF_VOID_P)
if(SIZEOF_VOID_P EQUAL 8)
	message(STATUS "64-bit")
	set(IS_64_BIT TRUE)
elseif(SIZEOF_VOID_P EQUAL 4)
	message(STATUS "32-bit")
	set(IS_64_BIT FALSE)
endif()

#==========INCLUDE DIRS==========#
set(MOBILE_BASE_DIR "/usr/local/include/MobileClient" CACHE STRING "MOBICORE INSTALLATION DIR")
target_include_directories(${TARGET_NAME} PUBLIC
	"${MOBILE_BASE_DIR}/Core"
	"${MOBILE_BASE_DIR}/Raw"
	"${CMAKE_CURRENT_SOURCE_DIR}/mobigame"
	"${CMAKE_CURRENT_SOURCE_DIR}"
)
-Wl,-Bstatic -liconv -Wl,-Bdynamic
#==========LIBRARIES==========#
if (IS_64_BIT)
	target_link_directories(${TARGET_NAME} PUBLIC
		"/usr/lib32"
		"/usr/local/lib32"
	)
else()
	target_link_directories(${TARGET_NAME} PUBLIC
		"/usr/lib"
		"/usr/local/lib"
	)
endif()

set(Iconv_IS_BUILT_IN FALSE CACHE BOOL "" FORCE)
target_link_options(${TARGET_NAME} PUBLIC -Wl,-Bstatic -lMobileClient -liconv -Wl,-Bdynamic)

#==========DEFINE FLAGS==========# (should be synced with cmakelists.txt in mobicore lib)
target_compile_definitions(${TARGET_NAME} PRIVATE 
	__MOBICORE__=1
	PLATFORM_FREEBSD=1
	PLATFORM_WINDOWS=0
	__BUILD_FOR_GAME__=1
	__MT_DB_INFO__=1
	__MOBI_PACKET_ENCRYPTION__=1
	__OFFSHOP__=1
	__FIGHTER_SCORE_SYNC__=0
)
#===================================================Mobile======================================================

