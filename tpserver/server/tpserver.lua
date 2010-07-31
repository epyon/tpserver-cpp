-- A solution contains projects, and defines the available configurations
project "tpserver"
	kind "ConsoleApp"
    language "C++"
    files { "**.h", "**.cpp" }
	flags { "ExtraWarnings", "NoPCH" }
	includedirs { ".", "tpserver" }
	targetname "http-proxy"
 
    configuration "debug"
		defines { "DEBUG" }
		flags { "Symbols" }
  		targetdir "bin"
		objdir "temp/debug"

 
    configuration "release"
		defines { "NDEBUG" }
		flags { "Optimize" }
		targetdir "bin"
		objdir "temp/release"
