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
    targetdir "build/autograder"

    files {"OpenGL/ScreenGrabLib.cpp"}

    includedirs {"OpenGL"}

    links {"glfw", "dl"}


project "RunSubmission"
    kind "ConsoleApp"
    language "C"
    targetdir "build/autograder"

    files {"run_submission.c"}

    includedirs "OpenGL"

    links {"seccomp"}


project "OpenGLProject"
    kind "ConsoleApp"
    language "C++"
    targetdir "build/autograder"

    files {"OpenGL/main.cpp", "OpenGL/glad/src/gl.c"}

    includedirs {"OpenGL", "OpenGL/glad/include"}

    links {"glfw"}
