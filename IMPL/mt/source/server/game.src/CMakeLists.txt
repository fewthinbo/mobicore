################################ NOTES ##################################
#IF YOU ARE IN THE X32 MACHINE AND HAVE External/library/libssl.a External/library/libcrypto.a /NOT CRYPTOPP/
#in your cmakelists.txt, use system libs instead of them:
#target_link_libraries(${TARGET_NAME} 
#	PUBLIC
#	Boost::system
#	Boost::thread
#	OpenSSL::Crypto
#	OpenSSL::SSL
#)
##########
#IF YOU ARE IN THE X32 MACHINE AND USING EXTERNAL BOOST LIBRARY WITH VENDOR APPROACH, DELETE THEM AND USE SYSTEM ONES
#DELETE THAT LINE IF YOU'VE SOMETHING LIKE THIS
#target_include_directories(${TARGET_NAME} PUBLIC
#	"Extern/include/boost"
#	"Extern/include/openssl"
#)
##########
#IF YOU ARE IN THE X64 MACHINE AND TRYING TO BUILD X32 GAME SERVER:
#search PLEASE_PROVIDE_YOUR_EXTERN_FOLDER in this file and fill out.
#Then you must be sure about you've libssl.a and libcrypto.a files in your external folder
#If you don't have follow these steps:
#Execute "uname -m -v" command on your freebsd and check out your current freebsd version with 
#Find and install x32 version of your freebsd (Must be same architecture and version)
#Run "pkg install -y openssl libiconv" then enter the /usr/local/lib folder copy libiconv.a file to your external folder.
#Enter the /usr/lib and copy those files to your external folder: libssl.a, libcrypto.a
#You've done.
################################ EOF NOTES ##################################

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
target_link_options(${TARGET_NAME} PUBLIC -Wl,-Bstatic -lMobileClient -Wl,-Bdynamic)

if (IS_64_BIT)
	target_link_directories(${TARGET_NAME} PUBLIC "PLEASE_PROVIDE_YOUR_EXTERN_FOLDER")
	target_link_libraries(${TARGET_NAME} PUBLIC iconv ssl crypto)
else()
	target_link_options(${TARGET_NAME} PUBLIC -Wl,-Bstatic -liconv -Wl,-Bdynamic)
endif()

#==========DEFINE FLAGS==========# (should be synced with cmakelists.txt in mobicore lib)
target_compile_definitions(${TARGET_NAME} PRIVATE 
	__MOBICORE__=1
	PLATFORM_FREEBSD=1
	PLATFORM_WINDOWS=0
	__BUILD_FOR_GAME__=1
	__MT_DB_INFO__=1
	__MOBI_PACKET_ENCRYPTION__=1
	__OFFSHOP__=1
)
#===================================================Mobile======================================================

