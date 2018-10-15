workspace "gl_tests"
    configurations { "Debug", "Release" }

project "precompiled_shaders"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    toolset "clang"

    targetdir "bin/%{cfg.buildcfg}"

    files { "**.hpp", "**.cpp", "shaders/**.vert", "shaders/**.frag" }
    
    configuration { "linux", "gmake" }
        buildoptions { "-Wall", "-Wextra", "-pedantic-errors", "-fdiagnostics-show-template-tree", "-fno-elide-type", "-ftemplate-backtrace-limit=0" } 
    
    includedirs { "inc/" }

    configuration { "linux" }
        links { "glfw", "GLEW", "GL" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter 'files:shaders/**.vert or files:shaders/**.frag'
        buildmessage 'Compiling %{file.relpath}'
        os.mkdir('compiled_shaders')
        buildcommands   {
            'glslangValidator -o compiled_shaders/%{file.name}.spirv -G450 %{file.relpath}'
        }
        buildoutputs { 'compiled_shaders/%{file.name}.spirv' }
