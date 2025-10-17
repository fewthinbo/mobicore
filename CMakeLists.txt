cmake_minimum_required(VERSION 3.20)
project(MobileClient VERSION 1.0.0 LANGUAGES C CXX)

set(TARGET_NAME MobileClient)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to be used")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "Disable C++ extensions")

# Detect platform
if(WIN32)
	set(PLATFORM_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	set(PLATFORM_FREEBSD TRUE)
endif()

message(STATUS "System processor is: ${CMAKE_SYSTEM_PROCESSOR}")

# Check if the system is 64-bit or 32-bit
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


set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps" CACHE PATH "FetchContent cache directory")

if(PLATFORM_WINDOWS)
	message(STATUS "Using default MSVC compiler")
elseif(PLATFORM_FREEBSD)
	set(CMAKE_C_COMPILER "clang")
	set(CMAKE_CXX_COMPILER "clang++")
	# Set installation prefix to /usr/local if not specified
	set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation prefix" FORCE)
	message(STATUS "Using Clang compiler on FreeBSD")
endif()


# Define custom configurations
# In addition to standard Debug and Release, add Dist configuration
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

# Create Source directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/Source)


# Define source files
file(GLOB_RECURSE SOURCES "Source/*.cpp" "Source/*.h" "Source/*.hpp")
file(GLOB_RECURSE HEADERS "Source/*.h" "Source/*.hpp")

if (PLATFORM_WINDOWS)
	set(MOBIGAME_BASE_DIR "${CMAKE_SOURCE_DIR}/IMPL/mt/source/server/game.src/mobigame/")

	file(GLOB_RECURSE MOBIGAME_SOURCES "${MOBIGAME_BASE_DIR}/*.cpp")
	file(GLOB_RECURSE MOBIGAME_HEADERS "${MOBIGAME_BASE_DIR}/*.h" "${MOBIGAME_BASE_DIR}/*.hpp")

	list(APPEND SOURCES ${MOBIGAME_SOURCES})
	list(APPEND HEADERS ${MOBIGAME_HEADERS})
endif()


# Create target based on platform
if(PLATFORM_WINDOWS)
	add_executable(${TARGET_NAME} ${SOURCES})
else()
	add_library(${TARGET_NAME} STATIC ${SOURCES})
endif()

# Platform-specific compiler settings
if(PLATFORM_WINDOWS)    
	# Keep source groups matching to disk
	set_property(GLOBAL PROPERTY USE_SOURCE_GROUPS ON)
		
	# Generic paths for Visual Studio
	set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD ON)
		
	# Use folders in Solution Explorer
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
		
	# Keep paths generic and consistent
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "${TARGET_NAME}")
		
	# Set MSVC runtime library
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
	else()
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
	endif()
	# Set default properties for Visual Studio
	add_compile_options(
		"$<$<CONFIG:Debug>:/Od>"
		"$<$<CONFIG:Debug>:/Zi>"
		"$<$<NOT:$<CONFIG:Debug>>:/O2>"
		#/MP  # Enable multi-processor compilation
		/EHsc  # Enable C++ exception handling
		/Zc:__cplusplus  # Make __cplusplus macro accurate
		#/Zc:preprocessor  # Use new preprocessor
	)
	# Define common preprocessor definitions from VCXPROJ
	add_compile_definitions(
		"$<$<CONFIG:Debug>:DEBUG=1;_DEBUG=1>"
		"$<$<CONFIG:Release>:NDEBUG=1;RELEASE=1>"
		"$<$<CONFIG:Dist>:NDEBUG=1;DIST=1>"
		NOMINMAX
		_WIN32_WINNT=0x0601
		WIN32_LEAN_AND_MEAN
		PLATFORM_WINDOWS=1
	)
	add_compile_definitions(
		TEST_CONSOLE=1	
		LOCAL_DB_MANAGER=0
		__BUILD_FOR_GAME__=0
	)

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_compile_options(-g -fno-inline -fno-omit-frame-pointer)
		message(STATUS "Added debugging compile options")
	endif()

	if(MSVC)
		add_compile_options(/W4)
	else() # if clang on Windows
		add_compile_options(-Wall -Wextra)
	endif()


elseif(PLATFORM_FREEBSD)
	add_compile_definitions(PLATFORM_FREEBSD=1)

	# FreeBSD için statik bağlama ayarları
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")  # Sadece statik kütüphaneler
	set(BUILD_SHARED_LIBS OFF)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")  # Tam statik binary

	add_compile_definitions(
		_POSIX_C_SOURCE=200809L
		__BSD_VISIBLE=1
		__XSI_VISIBLE=1
		_THREAD_SAFE
	)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_compile_options(-O0 -g)
		add_compile_definitions(DEBUG=1 _DEBUG=1)
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		add_compile_options(-O2)
		add_compile_definitions(NDEBUG=1 RELEASE=1)
	elseif(CMAKE_BUILD_TYPE STREQUAL "Dist")
		add_compile_options(-O3)
		add_compile_definitions(NDEBUG=1 DIST=1)
	endif()

	add_compile_options(
		-m32
		-fno-strict-aliasing
		#-fno-exceptions
		#-fno-rtti
		-Wall 
		-Wextra 
		-Wpedantic
		-Wno-gnu-zero-variadic-macro-arguments
		-Wno-unused-variable
		-Wno-unused-parameter
		-Wno-c++20-extensions
		-Wno-invalid-utf8
	)

	add_compile_definitions(
		TEST_CONSOLE=0
		__BUILD_FOR_GAME__=1
	)
endif()

add_compile_definitions(__MOBICORE__=1)
add_compile_definitions(__MT_DB_INFO__=1)
add_compile_definitions(__MOBI_PACKET_ENCRYPTION__=1)
add_compile_definitions(__FIGHTER_SCORE_SYNC__=0)
add_compile_definitions(__OFFSHOP__=1)

# Set output directories
if(PLATFORM_WINDOWS)
	set_target_properties(${TARGET_NAME} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Shits/$<CONFIG>/${TARGET_NAME}"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Libs/$<CONFIG>/${TARGET_NAME}"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Exec/$<CONFIG>/${TARGET_NAME}"
		PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/Binaries/Shits/$<CONFIG>/${TARGET_NAME}"
		COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Shits/$<CONFIG>/${TARGET_NAME}"
	)
	if(MSVC)
		set_target_properties(${TARGET_NAME} PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Exec/$<CONFIG>/${TARGET_NAME}"
		)
	endif()	
else()
	set_target_properties(${TARGET_NAME} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Shits/${CMAKE_BUILD_TYPE}/${TARGET_NAME}"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Libs/${CMAKE_BUILD_TYPE}/${TARGET_NAME}"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Exec/${CMAKE_BUILD_TYPE}/${TARGET_NAME}"
	)
endif()


# Set include directories
target_include_directories(${TARGET_NAME} PUBLIC
	"${CMAKE_SOURCE_DIR}/Source"
	"${CMAKE_SOURCE_DIR}/Source/Core/"
	"${CMAKE_SOURCE_DIR}/Source/Raw/"
)

if (PLATFORM_WINDOWS)
	target_include_directories(${TARGET_NAME} PUBLIC
		"${MOBIGAME_BASE_DIR}"
	)
elseif(PLATFORM_FREEBSD)
	target_include_directories(${TARGET_NAME} SYSTEM PUBLIC
		/usr/include/
		/usr/local/include/
	)
endif()

if (PLATFORM_FREEBSD)
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
endif()

if (PLATFORM_FREEBSD)
	if (FALSE) #bsd icin mt bagimsiz test executable; hepsini statik linkle
		message(STATUS "FreeBSD: Using static system libraries only ")
		set(LINK_LIBRARIES "")
		
		list(APPEND LINK_LIBRARIES 
			boost_system     # Boost system
			boost_thread     # Boost thread
			ssl             # OpenSSL SSL
			crypto          # OpenSSL crypto
			z               # ZLIB
			iconv           # Iconv
			pthread         # POSIX threads
		)
		
		message(STATUS "Static libraries to link: ${LINK_LIBRARIES}")
		target_link_libraries(${TARGET_NAME} PUBLIC ${LINK_LIBRARIES})
	else()
		set(Iconv_IS_BUILT_IN FALSE CACHE BOOL "" FORCE)  #libiconv'ye gectim, Iconv::Iconv
		find_package(Boost CONFIG REQUIRED COMPONENTS system thread)
		find_package(nlohmann_json CONFIG REQUIRED)
		find_package(OpenSSL REQUIRED)
		find_package(ZLIB REQUIRED)
		find_package(Iconv REQUIRED)

		
		target_link_libraries(${TARGET_NAME} 
			PUBLIC
			Boost::system
			Boost::thread
			nlohmann_json::nlohmann_json
			OpenSSL::Crypto
			OpenSSL::SSL
			ZLIB::ZLIB
			Iconv::Iconv
		)
	endif()
endif()

if(PLATFORM_WINDOWS)
	# Diğer platformlar için zorunlu bağımlılıklar
	find_package(Boost REQUIRED COMPONENTS system thread)
	find_package(nlohmann_json CONFIG REQUIRED)
	find_package(OpenSSL REQUIRED)
	find_package(ZLIB REQUIRED)
	
	target_link_libraries(${TARGET_NAME} 
		PUBLIC
		Boost::system
		Boost::thread
		nlohmann_json::nlohmann_json
		OpenSSL::Crypto
		OpenSSL::SSL
		ZLIB::ZLIB
	)
endif()

# Configure as console application if windows
if(MSVC)
	set_target_properties(${TARGET_NAME} PROPERTIES 
		LINK_FLAGS "/SUBSYSTEM:CONSOLE"
	)

	# Set link-time options
	set_property(TARGET ${TARGET_NAME} PROPERTY 
		VS_GLOBAL_LinkIncremental "$<IF:$<CONFIG:Debug>,true,false>"
	)
endif()

# Installation rules

# Create include directory in build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/${TARGET_NAME})

# Copy headers to include directory preserving directory structure
foreach(HEADER ${HEADERS})
	file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/Source" ${HEADER})
	get_filename_component(HEADER_DIR ${REL_PATH} DIRECTORY)
	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/${TARGET_NAME}/${HEADER_DIR})
	file(COPY ${HEADER} DESTINATION ${CMAKE_BINARY_DIR}/include/${TARGET_NAME}/${HEADER_DIR})
endforeach()

if(PLATFORM_FREEBSD)
	file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/include/${TARGET_NAME})

	# Install the library
	install(TARGETS ${TARGET_NAME}
		ARCHIVE DESTINATION lib  # static library (.a)
		LIBRARY DESTINATION lib  # shared library (.so)
		RUNTIME DESTINATION bin  # executables
	)

	# Install headers to /usr/local/include/${TARGET_NAME}
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/Source/
		DESTINATION include/${TARGET_NAME}
		FILES_MATCHING 
		PATTERN "*.h"
		PATTERN "*.hpp"
		PATTERN "*.inl"
	)
else()
	file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/include/${TARGET_NAME})
	# Windows installation rules
	install(TARGETS ${TARGET_NAME}
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
	)

	# Install headers
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/Source/
		DESTINATION include/${TARGET_NAME}
		FILES_MATCHING 
		PATTERN "*.h"
		PATTERN "*.hpp"
	)
endif()