#pragma once
#include <utility>
#include <vector>

#include <GL/glew.h>

#include "gl/gl_helpers.hpp"
#include "logger.hpp"
#include "binary_loader.hpp"

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

    gl_resource( gl_resource&& other )
        : m_id{std::exchange( other.m_id, invalid_id )}
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

    GLuint* ptr()
    {
        return &m_id;
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
        : gl_resource( std::move( other ) ), m_is_set{std::exchange(
                                                 other.m_is_set, false )}
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

    static inline shader< vertex_shader_type >
    make_shader( vertex_shader_type&& )
    {
        logger::log( "Create vertex shader... " );
        return shader< vertex_shader_type >(
            gl_helpers::gl_call( glCreateShader, GL_VERTEX_SHADER ) );
    }

    static inline shader< fragment_shader_type >
    make_shader( fragment_shader_type&& )
    {
        logger::log( "Create fragment shader... " );
        return shader< fragment_shader_type >(
            gl_helpers::gl_call( glCreateShader, GL_FRAGMENT_SHADER ) );
    }

    template < typename T >
    static inline shader< T >
    make_shader_from_binary( T&&,
                             const std::vector< unsigned char >& data,
                             const std::string_view& entry_point )
    {
        shader< T > vs = make_shader( T{} );
        assert( vs.is_valid() && "Shader is not valid!" );

        gl_helpers::gl_call( glShaderBinary, 1, vs.ptr(),
                             GL_SHADER_BINARY_FORMAT_SPIR_V, data.data(),
                             data.size() );

        gl_helpers::gl_call( glSpecializeShader, vs.id(), entry_point.data(), 0,
                             nullptr, nullptr );

        GLint is_compiled = 0;
        gl_helpers::gl_call( glGetShaderiv, vs.id(), GL_COMPILE_STATUS,
                             &is_compiled );

        if ( is_compiled == GL_FALSE )
        {
            GLint max_len = 0;
            gl_helpers::gl_call( glGetShaderiv, vs.id(), GL_INFO_LOG_LENGTH,
                                 &max_len );

            std::vector< GLchar > info_log( max_len );
            gl_helpers::gl_call( glGetShaderInfoLog, vs.id(), max_len, &max_len,
                                 &info_log[0] );
            
            logger::log( "failed to create shader!!!", info_log.data() );
            return shader< T >{};
        }

        return vs;
    }
}; // namespace gl_device

struct renderer
{
    void on_initialize()
    {
        gl_helpers::check_gl_errors();

        const auto vs =
            load_binary_data( "compiled_shaders/triangle.vert.spirv" );
        const auto fs =
            load_binary_data( "compiled_shaders/triangle.frag.spirv" );

        m_vertex_shader = gl_device::make_shader_from_binary(
            vertex_shader_type{}, vs, "main" );
        m_fragment_shader = gl_device::make_shader_from_binary(
            fragment_shader_type{}, fs, "main" );
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
