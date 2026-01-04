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
        ["Header Files/*"] = { "include/**.h", "include/**.hpp", "src/**.h", "src/**.hpp"},
        ["Source Files/*"] = { "src/**.cpp"},
    }
    files {"src/**.hpp", "src/**.h", 
	"src/file_manager.cpp",
	"src/file_name_manager.cpp",
	"src/idl_gen_text.cpp",
	"src/idl_parser.cpp",
	"src/reflection.cpp",
	"src/util.cpp",
	}

    includedirs { "./" }
    includedirs { "./src" }
    includedirs { "./include" }

    include_raylib()