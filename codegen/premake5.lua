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
    kind "ConsoleApp"
    language "C#" -- Sets the project language
    framework "net10.0" -- Example framework version
    clr ("NetCore")

    filter "configurations:Debug"
        defines { "DEBUG" } -- Add a DEBUG symbol for debug builds
        symbols "On" -- Enable debug symbols

    filter "configurations:Release"
        defines { "NDEBUG" } -- Add NDEBUG symbol for release builds
        optimize "On" -- Enable optimization

    filter {}

    location "./"
    targetdir "../tool/"
    files {"*.cs"}

    entrypoint "codegen.Program"  -- Specify the class with Main method

    links {
        "System",
        "System.Core",
        "Microsoft.CSharp"
    } -- Reference standard C# assemblies
