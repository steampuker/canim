-- Game Premake by Jeffery Myers is marked CC0 1.0. To view a copy of this mark, visit https://creativecommons.org/publicdomain/zero/1.0/
newoption
{
    trigger = "graphics",
    value = "OPENGL_VERSION",
    description = "version of OpenGL to build raylib against",
    allowed = {
        { "opengl11", "OpenGL 1.1"},
        { "opengl21", "OpenGL 2.1"},
        { "opengl33", "OpenGL 3.3"},
        { "opengl43", "OpenGL 4.3"},
        { "opengles2", "OpenGLES 2.0"},
        { "opengles3", "OpenGLES 3.0"}
    },
    default = "opengl33"
}

newoption
{
    trigger = "backend",
    value = "BACKEND",
    description = "backend to use",
    allowed = {
        { "GLFW", "GLFW"},
        { "SDL2", "SDL2"},
        { "SDL3", "SDL3"},
        { "RGFW", "RGFW"}
    },
    default = "GLFW"
}

newoption
{
    trigger = "wayland",
    value = "WAYLAND",
    description = "build for wayland",
    allowed = {
        { "off", "Off"},
        { "on", "On"}
    },
    default = "off"
}
function string.starts(String,Start)
    return string.sub(String,1,string.len(Start))==Start
end

function link_to(lib)
    links (lib)
    includedirs ("../"..lib.."/include")
    includedirs ("../"..lib.."/" )
end

function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
end

function check_raylib()
    local libdir = "lib/raylib"
    local tmpdir
    if os.target() == "windows" then
        tmpdir = os.getenv("TEMP") or os.getenv("TMP") .. "/"
    else
        tmpdir = "/tmp/"
    end

    if(os.isdir(libdir) == false and os.isdir("lib/raylib-master") == false) then
        -- Create lib directory if it doesn't exist
        os.mkdir("lib")

        local zipfile = tmpdir .. "/raylib-master.zip"

        if(not os.isfile(zipfile)) then
            print("Raylib not found, downloading from github")
            local result_str, response_code = http.download(
                "https://github.com/raysan5/raylib/archive/refs/heads/master.zip",
                zipfile,
                {
                    progress = download_progress,
                    headers = { "From: Premake", "Referer: Premake" }
                }
            )
        end

        print("Unzipping to " .. os.getcwd() .. "/lib")
        zip.extract(zipfile, os.getcwd() .. "/lib")
        os.remove(zipfile)
    end
end

function use_library(libraryName, githubFolder, repoHead)
    libFolder = libraryName .. "-" .. repoHead
    zipFile = libFolder .. ".zip"

    baseName = path.getbasename(os.getcwd());

    links(libraryName);
    includedirs {"../" .. libFolder .. "/" }
    includedirs {"../" .. libFolder .."/src/" }
    includedirs {"../" .. libFolder .."/include/" }

    os.chdir("..")

    if(os.isdir(libFolder) == false) then
        if(not os.isfile(zipFile)) then
            print(libraryName .. " not found, downloading from github")
            local result_str, response_code = http.download("https://github.com/" .. githubFolder .. "/archive/refs/heads/" .. repoHead ..".zip", zipFile, {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract(zipFile, os.getcwd())
        os.remove(zipFile)
    end

    os.chdir(libFolder)

    project (libraryName)
        kind "StaticLib"
        location "./"
        targetdir "../bin/%{cfg.buildcfg}"

        filter "action:vs*"
            buildoptions { "/experimental:c11atomics" }

        vpaths
        {
            ["Header Files/*"] = { "include/**.h", "include/**.hpp",  "**.h", "**.hpp"},
            ["Source Files/*"] = { "src/**.cpp", "src/**.c", "**.cpp",  "**.c"},
        }
        files {"include/**.hpp", "include/**.h","src/**.hpp", "src/**.h", "src/**.cpp", "src/**.c"}

        includedirs { "./" }
        includedirs { "./src" }
        includedirs { "./include" }

    os.chdir(baseName)
end

workspace "canim"
    configurations { "debug", "sanitize", "release"}
    platforms { "x64", "x86", "ARM64"}

    defaultplatform ("x64")

    filter "configurations:debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:sanitize"
        defines { "DEBUG" }
        sanitize { "Address", "UndefinedBehavior" }
        symbols "On"

    filter "configurations:release"
        defines { "NDEBUG", "ALTARR_NO_DIAGNOSTICS" }
        optimize "On"

    filter { "platforms:x64" }
        architecture "x86_64"

    filter { "platforms:Arm64" }
        architecture "ARM64"

    filter {}

    targetdir "bin/%{cfg.buildcfg}/"
    objdir "bin/%{cfg.buildcfg}/.build/obj"

    if(os.isdir("canim")) then
        startproject("canim")
    end

check_raylib();

include ("raylib_premake.lua")

project "canim"
    kind "WindowedApp"
    language "C"

    targetdir "bin/%{cfg.buildcfg}"
    files {"src/**.h", "src/**.c"}
    includedirs {"include/"}

    filter "toolset:gcc or toolset:clang"
        links { "m", "raylib" }

    filter {"options:wayland=off"}
        links { "X11" }

    postbuildcommands {
        "{MKDIR} %[bin/%{cfg.buildcfg}/include]",
        "{COPYFILE} src/core/canim.h %[bin/%{cfg.buildcfg}/include/canim.h]"
    }
