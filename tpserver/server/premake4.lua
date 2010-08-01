-- A solution contains projects, and defines the available configurations
solution "tpserver"
   configurations { "debug", "release" }
   
   dofile("tpserver.lua")
 
newaction {
   trigger     = "doc",
   description = "Run doxygen",
   execute     = function ()
      os.execute("doxygen")
   end
}

