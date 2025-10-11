#======Mobile=========
set(MOBILE_BASE_DIR "/usr/local/include/MobileClient" CACHE STRING "MOBICORE INSTALLATION DIR")
target_include_directories(${TARGET_NAME} PUBLIC
	"${MOBILE_BASE_DIR}/Core"
	"${MOBILE_BASE_DIR}/Raw"
	"${CMAKE_CURRENT_SOURCE_DIR}/mobigame"
	"${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_definitions(${TARGET_NAME} PRIVATE __MOBICORE__)
target_link_libraries(${TARGET_NAME} PUBLIC MobileClient iconv)
#======Mobile=========
