#pragma once 

#include <GL/glew.h>
#include "logger.hpp"

namespace gl_helpers
{
    struct gl_error_invalid_enum
    {
        static constexpr auto name = "invalid enum";
    };

    struct gl_error_invalid_value
    {
        static constexpr auto name = "invalid value";
    };

    struct gl_error_invalid_operation
    {
        static constexpr auto name = "invalid operation";
    };

    struct gl_error_invalid_framebuffer_operation
    {
        static constexpr auto name = "invalid framebuffer operation";
    };

    struct gl_error_out_of_memory
    {
        static constexpr auto name = "out of memory";
    };

    struct gl_error_stack_underflow
    {
        static constexpr auto name = "stack underflow";
    };

    struct gl_error_stack_overflow
    {
        static constexpr auto name = "stack overflow";
    };

    struct gl_error_unknown
    {
        static constexpr auto name = "unknown";
    };

    auto gl_error_enum_to_name( GLenum e )
    {
        switch ( e )
        {
            case GL_INVALID_ENUM:
                return gl_error_invalid_enum::name;
            case GL_INVALID_VALUE:
                return gl_error_invalid_value::name;
            case GL_INVALID_OPERATION:
                return gl_error_invalid_operation::name;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return gl_error_invalid_framebuffer_operation::name;
            case GL_OUT_OF_MEMORY:
                return gl_error_out_of_memory::name;
            case GL_STACK_OVERFLOW:
                return gl_error_stack_overflow::name;
            case GL_STACK_UNDERFLOW:
                return gl_error_stack_underflow::name;
            default:
                return gl_error_unknown::name;
        }
    }

    void report_gl_error( GLenum e )
    {
        logger::log( "Error calling OpenGL function: ", gl_error_enum_to_name( e ) );
    }

    void report_gl_dirty( GLenum e )
    {
        logger::log( "Error, uncaught OpenGL error: ", gl_error_enum_to_name( e ) );
    }

    template < typename R, typename... Args, typename... TArgs >
    auto gl_call( R ( *func )( Args... ), TArgs&&... args ) -> R
    {
        const auto e0 = glGetError();
        R ret{0};

        if ( e0 == GL_NO_ERROR )
        {
            ret = func( static_cast< Args >( std::forward< TArgs >( args ) )... );
        }
        else
        {
            report_gl_dirty( e0 );
            return ret;
        }

        const auto e1 = glGetError();
        if ( e1 != GL_NO_ERROR )
        {
            report_gl_error( e1 );
        }

        return ret;
    }

    template < typename... Args, typename... TArgs >
    void gl_call( void ( *func )( Args... ), TArgs&&... args )
    {
        const auto e0 = glGetError();

        if ( e0 == GL_NO_ERROR )
        {
            func( static_cast< Args >( std::forward< TArgs >( args ) )... );
        }
        else
        {
            report_gl_dirty( e0 );
            return;
        }

        const auto e1 = glGetError();
        if ( e1 != GL_NO_ERROR )
        {
            report_gl_error( e1 );
        }
    }

    void check_gl_errors()
    {
        const auto e0 = glGetError();

        if ( e0 != GL_NO_ERROR )
        {
            report_gl_dirty( e0 );
        }
    }

    void eat_gl_errors()
    {
        while ( glGetError() != GL_NO_ERROR )
            ;
    }
} // namespace gl_helpers
