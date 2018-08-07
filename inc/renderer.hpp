#pragma once
#include <utility>
#include <GL/glew.h>

#include "gl/gl_helpers.hpp"
#include "logger.hpp"


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
