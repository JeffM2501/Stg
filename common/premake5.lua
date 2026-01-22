-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event 
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial 
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you 
--  wrote the original software. If you use this software in a product, an acknowledgment 
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

baseName = path.getbasename(os.getcwd());

project (baseName)
    kind "StaticLib"
    location "./"
    targetdir "../bin/%{cfg.buildcfg}"

    vpaths 
    {
        ["Header Files/*"] = { "include/**.h", "include/**.hpp", "**.h", "**.hpp"},
        ["Source Files/*"] = { "src/**.cpp", "src/**.c", "**.cpp","**.c"},
        ["Schema Files/*"] = { "Schema/**.schema"},
    }
    files {"**.hpp", "**.h", "**.cpp","**.c"}
    
    filter {"action:vs*"}
        files {"**.schema"}
    dependson('codegen')
	
    filter {"action:vs*", "files:**.schema"}
    buildmessage 'Generating Header for Schema %(Filename)'
    -- One or more commands to run (required)
    -- buildcommands '../codegen/bin/debug/net10.0/codegen.exe c++ $(SolutionDir)common/schemas $(SolutionDir)common/messages'
	
    buildcommands {"$(SolutionDir)codegen\\bin\\debug\\net10.0\\codegen.exe c++ %(Fullpath) $(SolutionDir)common\\messages\\"}
    -- One or more outputs resulting from the build (required)
    buildoutputs { "$(SolutionDir)common\\messages\\%(Filename).h" }
    filter {}

    includedirs { "./" }
    includedirs { "./src" }
    includedirs { "./include" }