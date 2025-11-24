------------------------------------------------------------------ NOTES --------------------------------------------------------------------
--IF YOU ARE IN THE X32 MACHINE AND HAVE External/library/libssl.a External/library/libcrypto.a /NOT CRYPTOPP/
--or something in your premake, use system libs instead of them:
--linkoptions {
--   crypto,
--   ssl,
--}
--------------------
--IF YOU ARE IN THE X32 MACHINE AND USING EXTERNAL BOOST LIBRARY WITH VENDOR APPROACH, DELETE THEM AND USE SYSTEM ONES
--DELETE THAT LINE IF YOU'VE SOMETHING LIKE THIS
--includedirs {
-- Extern/include/boost
-- Extern/include/openssl
--}
--------------------
--IF YOU ARE IN THE X64 MACHINE AND TRYING TO BUILD X32 GAME SERVER:
--search PLEASE_PROVIDE_YOUR_EXTERN_FOLDER in this file and fill out.
--Then you must be sure about you've libssl.a and libcrypto.a files in your external folder
--If you don't have follow these steps:
--Execute "uname -m -v" command on your freebsd and check out your current freebsd version with 
--Find and install x32 version of your freebsd (Must be same architecture and version)
--Run "pkg install -y openssl libiconv" then enter the /usr/local/lib folder copy libiconv.a file to your external folder.
--Enter the /usr/lib and copy those files to your external folder: libssl.a, libcrypto.a
--You've done.
---------------------------------------------------------------- EOF NOTES --------------------------------------------------------------------

		project "game"
			...
			-- =========== MOBICORE ===========
			filter "system:bsd"
				includedirs {
					"%{prj.group}/%{prj.name}/",
					"%{prj.group}/%{prj.name}/mobigame/",
					"/usr/local/include/MobileClient/Core/",
					"/usr/local/include/MobileClient/Raw/"
				}
				defines {
					"__MOBICORE__=1",
					"PLATFORM_FREEBSD=1",
					"PLATFORM_WINDOWS=0",
					"__BUILD_FOR_GAME__=1",
					"__MT_DB_INFO__=1",
					"__MOBI_PACKET_ENCRYPTION__=1",
					"__OFFSHOP__=1"
				}
				files {
					"mobigame/**.cpp"
				}
				linkoptions {
					"-Wl,-Bstatic",
					"-lMobileClient",
					"-Wl,-Bdynamic"
				}

			filter {"system:bsd", "platforms:*64"}
				libdirs {
					"/usr/lib32",
					"/usr/local/lib32"
				}
				links {
					"ssl",
					"crypto",
					"iconv",
				}

			filter {"system:bsd", "platforms:*32"}
				libdirs {
					"/usr/lib",
					"/usr/local/lib"
				}
				linkoptions {
					"-Wl,-Bstatic",
					"-liconv",
					"-Wl,-Bdynamic"
				}

			filter {}
			-- =========== MOBICORE ===========
