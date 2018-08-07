#pragma once
#include <utility>
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

        if( e0 != GL_NO_ERROR )
        {
            report_gl_dirty( e0 );
        }
    }

    void eat_gl_errors()
    {
        while( glGetError() != GL_NO_ERROR );
    }
} // namespace gl_helpers

struct gl_resource
{
    constexpr static auto invalid_id = static_cast< GLuint >( -1 );

  public:
    gl_resource() : m_id( invalid_id )
    {
    }

    gl_resource( GLuint id ) : m_id( id )
    {
    }

    gl_resource( gl_resource&& other ) : m_id{std::exchange( other.m_id, invalid_id )}
    {
    }

    gl_resource& operator=( gl_resource&& other )
    {
        m_id = std::exchange( other.m_id, invalid_id );
        return *this;
    }

    GLuint id() const
    {
        return m_id;
    }

    bool is_valid() const
    {
        return m_id != invalid_id;
    }

  private:
    GLuint m_id;
};

struct vertex_shader_type
{
};
struct fragment_shader_type
{
};

template < typename T > struct shader : public gl_resource
{
    using shader_type = T;

    shader() : gl_resource(), m_is_set( false )
    {
    }

    shader( GLuint id ) : gl_resource( id ), m_is_set( true )
    {
    }

    shader( shader&& other )
        : gl_resource( std::move( other ) ), m_is_set{
                                                 std::exchange( other.m_is_set, false )}
    {
    }

    shader& operator=( shader&& other )
    {
        gl_resource::operator=( std::move( other ) );
        m_is_set             = std::exchange( other.m_is_set, false );
        return *this;
    }

    ~shader()
    {
        if ( m_is_set )
        {
            logger::log( "Delete shader" );
            gl_helpers::gl_call( glDeleteShader, id() );
        }
    }

  private:
    bool m_is_set;
};

using vertex_shader   = shader< vertex_shader_type >;
using fragment_shader = shader< fragment_shader_type >;

struct program : public gl_resource
{
};

struct pipeline
{
};

struct pass
{
};


namespace gl_device
{
    template < typename T > static inline shader< T > make_shader( T&& );

    static inline shader< vertex_shader_type > make_shader( vertex_shader_type&& )
    {
        logger::log( "Create vertex shader... " );
        return shader< vertex_shader_type >(
            gl_helpers::gl_call( glCreateShader, GL_VERTEX_SHADER ) );
    }

    static inline shader< fragment_shader_type > make_shader( fragment_shader_type&& )
    {
        logger::log( "Create fragment shader... " );
        return shader< fragment_shader_type >(
            gl_helpers::gl_call( glCreateShader, GL_FRAGMENT_SHADER ) );
    }
}; // namespace gl_device

struct renderer
{
    void on_initialize()
    {
        gl_helpers::check_gl_errors();
        m_vertex_shader   = gl_device::make_shader( vertex_shader_type{} );
        m_fragment_shader = gl_device::make_shader( fragment_shader_type{} );
    }

    void on_render()
    {
    }

    const vertex_shader& get_vertex_shader() const
    {
        return m_vertex_shader;
    }

    const fragment_shader& get_fragmetn_shader() const
    {
        return m_fragment_shader;
    }

  private:
    vertex_shader m_vertex_shader;
    fragment_shader m_fragment_shader;
};
