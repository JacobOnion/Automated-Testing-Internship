workspace "Container"
    configurations { "Debug", "Release" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter {}

    
project "ScreenGrab"
    kind "SharedLib"
    language "C++"
    targetdir "build"

    files {"OpenGL/ScreenGrabLib.cpp"}

    includedirs {"OpenGL"}

    links {"glfw", "dl"}


project "RunPristine"
    kind "ConsoleApp"
    language "C"
    targetdir "build"

    files {"run_pristine.c"}

    includedirs "OpenGL"

    links {"seccomp"}


project "RunSubmission"
    kind "ConsoleApp"
    language "C"
    targetdir "build"

    files {"run_submission.c"}

    includedirs "OpenGL"

    links {"seccomp"}


project "Pristine"
    kind "ConsoleApp"
    language "C++"
    targetdir "build"

    files {"OpenGL/main.cpp", "OpenGL/glad/src/gl.c"}

    includedirs {"OpenGL", "OpenGL/glad/include"}

    links {"glfw"}
