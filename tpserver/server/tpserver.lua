-- A solution contains projects, and defines the available configurations
project "tpserver"
  kind "ConsoleApp"
  language "C++"
  files { "**.h", "**.cpp" }
  flags { "ExtraWarnings", "NoPCH" }
  includedirs { "../../", ".", "/opt/local/include/" }
  targetname "http-proxy"
  links { "boost_system", "boost_thread-mt" }
  libdirs { "/opt/local/lib/" }

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
