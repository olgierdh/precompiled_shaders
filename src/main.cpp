// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries. 
// For conditions of distribution and use, see copying.txt file in root folder.

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cassert>

#include "logger.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "gl/gl_helpers.hpp"
#include "renderer.hpp"

namespace
{
    static void error_callback( int, const char* const description )
    {
        logger::log( "Error: ", description );
    }
} // namespace

struct glfw_context
{
    ~glfw_context()
    {
        if ( is_initialized() )
        {
            logger::log( "Destroying glfw context" );
            glfwTerminate();
        }
    }

    template < typename T > static glfw_context make( T&& error_callback )
    {
        logger::log( "Creating glfw context..." );
        glfwSetErrorCallback( std::forward< T >( error_callback ) );

        if ( glfwInit() )
        {
            logger::log( "glfw context created succesfully" );
            return glfw_context( true );
        }

        logger::log( "glfw context creation failed" );
        return glfw_context( false );
    }

    glfw_context( const glfw_context& ) = default;
    glfw_context( glfw_context&& )      = default;
    glfw_context& operator=( const glfw_context& ) = default;
    glfw_context& operator=( glfw_context&& ) = default;

    bool is_initialized() const
    {
        return m_is_initialized;
    }

  private:
    glfw_context( bool initialized ) : m_is_initialized( initialized )
    {
    }

  private:
    bool m_is_initialized;
};

struct glfw_window
{
    static glfw_window make( int ver_maj, int ver_min, int width, int height )
    {
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, ver_maj );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, ver_min );

        GLFWwindow* window =
            glfwCreateWindow( width, height, "Default name", nullptr, nullptr );

        logger::log( "Creating window" );

        if ( window )
        {
            glfwMakeContextCurrent( window );
            logger::log( "Window creation succesfull" );
        }
        else
        {
            logger::log( "Failed to create window" );
        }
        return glfw_window( window, ver_min, ver_maj, width, height );
    }

    glfw_window( const glfw_window& ) = default;
    glfw_window( glfw_window&& )      = default;
    glfw_window& operator=( const glfw_window& ) = default;
    glfw_window& operator=( glfw_window&& ) = default;

    ~glfw_window()
    {
        if ( is_initialized() )
        {
            logger::log( "Destroying window" );
            glfwDestroyWindow( m_window );
        }
    }

    bool is_initialized() const
    {
        return m_window != nullptr;
    }

    bool frame_begin() const
    {
        const bool ret = glfwWindowShouldClose( m_window );
        return ret;
    }

    void frame_end() const
    {
        glfwSwapBuffers( m_window );
        glfwPollEvents();
    }

    int get_ver_maj() const
    {
        return m_ver_maj;
    }

    int get_ver_min() const
    {
        return m_ver_min;
    }

    int get_width() const
    {
        return m_width;
    }

    int get_height() const
    {
        return m_height;
    }

  private:
    glfw_window(
        GLFWwindow* window, int ver_maj, int ver_min, int width, int height )
        : m_window( window ), m_ver_maj( ver_maj ), m_ver_min( ver_min ),
          m_width( width ), m_height( height )
    {
    }

  private:
    mutable GLFWwindow* m_window;
    int m_ver_maj;
    int m_ver_min;
    int m_width;
    int m_height;
};

struct glew_context
{
    static glew_context make()
    {
        glewExperimental = GL_TRUE;
        const GLenum err = glewInit();
        logger::log( "Initializing glew" );

        if ( err != GLEW_OK )
        {
            logger::log( "glew initialization failed: ",
                         glewGetErrorString( err ) );
            return glew_context( false );
        }

        if ( GLEW_VERSION_4_6 )
        {
            logger::log( "glew initialization succesfull" );
            return glew_context( true );
        }

        logger::log( "glew initialization failed no OpenGL 4.5" );
        return glew_context( false );
    }

    ~glew_context()
    {
    }

    bool is_initialized() const
    {
        return m_is_initialized;
    }

  private:
    glew_context( bool status ) : m_is_initialized( status )
    {
    }

  private:
    bool m_is_initialized;
};

int main()
{
    const auto ctx = glfw_context::make( error_callback );
    if ( !ctx.is_initialized() )
    {
        exit( EXIT_FAILURE );
    }

    const auto w = glfw_window::make( 4, 6, 1280, 720 );
    if ( !w.is_initialized() )
    {
        exit( EXIT_FAILURE );
    }

    gl_helpers::check_gl_errors();

    const auto g = glew_context::make();
    if ( !g.is_initialized() )
    {
        exit( EXIT_FAILURE );
    }

    auto r = renderer();
    r.on_initialize();

    while ( !w.frame_begin() )
    {
        r.on_render();
        w.frame_end();
    }
}
