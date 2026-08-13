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

    files {"Pristine/ScreenGrabLib.cpp"}

    includedirs {"Pristine"}

    links {"glfw", "dl"}


project "RunPristine"
    kind "ConsoleApp"
    language "C"
    targetdir "build"

    files {"run_pristine.c"}

    includedirs "Pristine"

    links {"seccomp"}


project "RunSubmission"
    kind "ConsoleApp"
    language "C"
    targetdir "build"

    files {"run_submission.c"}

    includedirs "Submission"

    links {"seccomp"}


project "Pristine"
    kind "ConsoleApp"
    language "C++"
    targetdir "build"

    files {"Pristine/main.cpp", "Pristine/glad/src/gl.c"}

    includedirs {"Pristine", "Pristine/glad/include"}

    links {"glfw"}


project "Submission"
    kind "ConsoleApp"
    language "C++"
    targetdir "build"

    files {"Submission/main.cpp", "Submission/glad/src/gl.c"}

    includedirs {"Submission", "Submission/glad/include"}

    links {"glfw"}

