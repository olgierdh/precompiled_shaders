// Copyright (C) 2017-2018 ChaosForge Ltd
// http://chaosforge.org/
//
// This file is part of Nova libraries.
// For conditions of distribution and use, see copying.txt file in root folder.

#pragma once
#include <utility>
#include <vector>

#include <GL/glew.h>

#include "gl/gl_helpers.hpp"
#include "logger.hpp"
#include "binary_loader.hpp"

#include "meta.hpp"
#include "type_system.hpp"
#include "renderer_types.hpp"
#include "gl/gl_vao_channels.hpp"

struct gl_resource
{
    constexpr static auto invalid_id = static_cast< GLuint >( -1 );

  public:
    gl_resource() noexcept
        : m_id( invalid_id )
        , m_is_set( false )
    {
    }

    gl_resource( GLuint id ) noexcept
        : m_id( id )
        , m_is_set( true )
    {
    }

    gl_resource( gl_resource&& other ) noexcept
        : m_id{std::exchange( other.m_id, invalid_id )}
        , m_is_set{std::exchange( other.m_is_set, false )}
    {
    }

    gl_resource& operator=( gl_resource&& other ) noexcept
    {
        m_id     = std::exchange( other.m_id, invalid_id );
        m_is_set = std::exchange( other.m_is_set, false );
        return *this;
    }

    GLuint id() const noexcept { return m_id; }

    GLuint* ptr() noexcept { return &m_id; }

    bool is_valid() const noexcept { return m_id != invalid_id; }

    bool is_set() const noexcept { return m_is_set; }

  private:
    GLuint m_id;
    bool m_is_set;
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

    shader() noexcept
        : gl_resource()
    {
    }

    shader( GLuint id ) noexcept
        : gl_resource( id )
    {
    }

    shader( shader&& other ) noexcept
        : gl_resource( std::move( other ) )
    {
    }

    shader& operator=( shader&& other ) noexcept
    {
        gl_resource::operator=( std::move( other ) );
        return *this;
    }

    ~shader() noexcept
    {
        if ( is_set() )
        {
            logger::log( "Delete shader" );
            gl_helpers::gl_call( glDeleteShader, id() );
        }
    }
};

using vertex_shader   = shader< vertex_shader_type >;
using fragment_shader = shader< fragment_shader_type >;

struct program : public gl_resource
{
    program() noexcept
        : gl_resource()
        , m_vs()
        , m_fs()
    {
    }

    program( GLuint id, vertex_shader&& vs, fragment_shader&& fs ) noexcept
        : gl_resource( id )
        , m_vs{std::exchange( vs, vertex_shader{} )}
        , m_fs{std::exchange( fs, fragment_shader{} )}
    {
    }

    program& operator=( program&& other ) noexcept
    {
        gl_resource::operator=( std::move( other ) );
        m_vs                 = std::exchange( other.m_vs, vertex_shader{} );
        m_fs                 = std::exchange( other.m_fs, fragment_shader{} );
        return *this;
    }

    ~program() noexcept
    {
        if ( is_set() )
        {
            gl_helpers::gl_call( glDetachShader, id(), m_vs.id() );
            gl_helpers::gl_call( glDetachShader, id(), m_fs.id() );
            gl_helpers::gl_call( glDeleteProgram, id() );
        }
    }

  private:
    vertex_shader m_vs;
    fragment_shader m_fs;
};

struct vertex_array_object : public gl_resource
{
    vertex_array_object() noexcept
        : gl_resource()
    {
    }

    vertex_array_object( GLuint id ) noexcept
        : gl_resource( id )
    {
    }

    vertex_array_object( vertex_array_object&& other ) noexcept
        : gl_resource( std::move( other ) )
    {
    }

    vertex_array_object& operator=( vertex_array_object&& other ) noexcept
    {
        gl_resource::operator=( std::move( other ) );
        return *this;
    }

    ~vertex_array_object() noexcept
    {
        if ( is_set() )
        {
            logger::log( "Delete vao" );
            gl_helpers::gl_call( glDeleteVertexArrays, 1, ptr() );
        }
    }
};

struct vertex_buffer_object : public gl_resource
{
    vertex_buffer_object() noexcept
        : gl_resource()
    {
    }

    vertex_buffer_object( GLuint id ) noexcept
        : gl_resource( id )
    {
    }

    vertex_buffer_object( vertex_buffer_object&& other ) noexcept
        : gl_resource( std::move( other ) )
    {
    }

    vertex_buffer_object& operator=( vertex_buffer_object&& other ) noexcept
    {
        gl_resource::operator=( std::move( other ) );
        return *this;
    }

    ~vertex_buffer_object() noexcept
    {
        if ( is_set() )
        {
            logger::log( "Delete vbo" );
            gl_helpers::gl_call( glDeleteBuffers, 1, ptr() );
        }
    }
};

template < typename T > static inline void bind( const T& );
template < typename T > struct scope_binder;

namespace gl_device
{
    static inline void bind( const vertex_buffer_object& vbo ) noexcept
    {
        gl_helpers::gl_call( glBindBuffer, GL_ARRAY_BUFFER, vbo.is_set() ? vbo.id() : 0 );
    }

    static inline void bind( const vertex_array_object& vao ) noexcept
    {
        gl_helpers::gl_call( glBindVertexArray, vao.is_set() ? vao.id() : 0 );
    }

    static inline void bind( const program& p ) noexcept
    {
        gl_helpers::gl_call( glUseProgram, p.is_set() ? p.id() : 0 );
    }
} // namespace gl_device

template < typename T > struct scope_binder
{
    scope_binder( T& v ) noexcept { gl_device::bind( v ); }

    ~scope_binder() noexcept { gl_device::bind( T{} ); }

    scope_binder( const scope_binder& ) noexcept = delete;
    scope_binder( scope_binder&& ) noexcept      = delete;
    scope_binder& operator=( const scope_binder& ) noexcept = delete;
    scope_binder& operator=( scope_binder&& ) noexcept = delete;
};

template < typename T >
[[nodiscard]] inline scope_binder< T > scope_bind( T& resource ) noexcept
{
    return scope_binder( resource );
}

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
    make_shader( vertex_shader_type&& ) noexcept
    {
        logger::log( "Create vertex shader... " );
        return shader< vertex_shader_type >(
            gl_helpers::gl_call( glCreateShader, GL_VERTEX_SHADER ) );
    }

    static inline shader< fragment_shader_type >
    make_shader( fragment_shader_type&& ) noexcept
    {
        logger::log( "Create fragment shader... " );
        return shader< fragment_shader_type >(
            gl_helpers::gl_call( glCreateShader, GL_FRAGMENT_SHADER ) );
    }

    static inline vertex_buffer_object make_vbo() noexcept
    {
        logger::log( "Create vbo" );
        GLuint vbo = 0;
        gl_helpers::gl_call( glGenBuffers, 1, &vbo );
        return vertex_buffer_object( vbo );
    }


    template < template < typename... > class T, typename D >
    static inline void
    write_data( vertex_buffer_object& vbo, const T< D >& data ) noexcept
    {
        static_assert( std::is_standard_layout_v< D >, "Data must be std layout" );
        static_assert( std::is_trivially_copyable_v< D >,
                       "Data must be trivially copyable" );

        const auto data_size = data.size() * sizeof( D );
        const auto s         = scope_bind( vbo );

        logger::log( "Copying data to GPU VBO size: ", data_size );

        gl_helpers::gl_call( glBufferData, GL_ARRAY_BUFFER, data_size, data.data(),
                             GL_STATIC_DRAW );
    }

    static inline vertex_array_object make_vao() noexcept
    {
        logger::log( "Create vao" );
        GLuint vao = gl_resource::invalid_id;
        gl_helpers::gl_call( glGenVertexArrays, 1, &vao );
        return vertex_array_object( vao );
    }

    template < typename T >
    static inline void
    configure_vao( vertex_buffer_object& vbo, vertex_array_object& vao ) noexcept
    {
        const auto s0 = scope_bind( vbo );
        const auto s1 = scope_bind( vao );

        using the_channels  = gl_vao_channel_desc_generator< T >;
        const auto channels = the_channels::generate_channels();
        constexpr auto size = sizeof( typename T::value_type );

        logger::log( "data size: ", size );

        int index = 0;
        for ( const auto& c : channels )
        {
            logger::log( "channel index: ", index, " channel size: ", c.m_size,
                         " channel len: ", c.m_len, " channel offset: ", c.m_offset );
            const GLenum type = gl_type_to_gl_enum( c.m_type );
            logger::log( "Type: ", static_cast< int >( type ) );

            gl_helpers::gl_call( glVertexAttribPointer, index, c.m_len, type, GL_FALSE,
                                 size, ( void* )c.m_offset );
            gl_helpers::gl_call( glEnableVertexAttribArray, index );

            index += 1;
        }
    }

    /**
     * This is not the best way of initialization.
     * @TODO Think about adding intermediate stages of processing for programs
     * a) before attach
     * b) after attach
     * c) after link
     */
    static inline program
    make_program( vertex_shader&& vs, fragment_shader&& fs ) noexcept
    {
        logger::log( "Create program" );
        GLuint program_id = gl_helpers::gl_call( glCreateProgram );

        gl_helpers::gl_call( glAttachShader, program_id, vs.id() );
        gl_helpers::gl_call( glAttachShader, program_id, fs.id() );

        gl_helpers::gl_call( glLinkProgram, program_id );

        GLint is_linked = 0;
        gl_helpers::gl_call( glGetProgramiv, program_id, GL_LINK_STATUS,
                             ( int* )&is_linked );

        if ( is_linked == GL_FALSE )
        {
            GLint max_len = 0;
            gl_helpers::gl_call( glGetProgramiv, program_id, GL_INFO_LOG_LENGTH,
                                 &max_len );
            std::vector< char > info_log( max_len );
            gl_helpers::gl_call( glGetProgramInfoLog, program_id, max_len, &max_len,
                                 &info_log[0] );

            gl_helpers::gl_call( glDeleteProgram, program_id );

            logger::log( "Failed to create a program: ", info_log.data() );

            return program{};
        }

        return program{program_id, std::move( vs ), std::move( fs )};
    }

    template < typename T >
    static inline shader< T >
    make_shader_from_binary( T&&, const std::vector< unsigned char >& data,
                             const std::string_view& entry_point ) noexcept
    {
        shader< T > vs = make_shader( T{} );
        assert( vs.is_valid() && "Shader is not valid!" );

        gl_helpers::gl_call( glShaderBinary, 1, vs.ptr(), GL_SHADER_BINARY_FORMAT_SPIR_V,
                             data.data(), data.size() );

        gl_helpers::gl_call( glSpecializeShader, vs.id(), entry_point.data(), 0, nullptr,
                             nullptr );

        GLint is_compiled = 0;
        gl_helpers::gl_call( glGetShaderiv, vs.id(), GL_COMPILE_STATUS, &is_compiled );

        if ( is_compiled == GL_FALSE )
        {
            GLint max_len = 0;
            gl_helpers::gl_call( glGetShaderiv, vs.id(), GL_INFO_LOG_LENGTH, &max_len );

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
    void on_initialize() noexcept
    {
        gl_helpers::check_gl_errors();

        const auto vs_data = load_binary_data( "compiled_shaders/triangle.vert.spirv" );
        const auto fs_data = load_binary_data( "compiled_shaders/triangle.frag.spirv" );

        auto vs =
            gl_device::make_shader_from_binary( vertex_shader_type{}, vs_data, "main" );
        auto fs =
            gl_device::make_shader_from_binary( fragment_shader_type{}, fs_data, "main" );

        m_program = gl_device::make_program( std::move( vs ), std::move( fs ) );
        m_vbo     = gl_device::make_vbo();
        gl_device::write_data( m_vbo, triangle_data );

        m_vao = gl_device::make_vao();
        gl_device::configure_vao< vertex_desc >( m_vbo, m_vao );
    }

    void on_render() noexcept
    {
        gl_helpers::gl_call( glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        const auto s2 = scope_binder( m_program );
        const auto s1 = scope_binder( m_vao );

        gl_helpers::gl_call( glDrawArrays, GL_TRIANGLES, 0, 3 );
    }


  private:
    program m_program;
    vertex_buffer_object m_vbo;
    vertex_array_object m_vao;

  private:
    static const std::vector< vertex > triangle_data;
};

const std::vector< vertex > renderer::triangle_data = std::vector< vertex >{
    vertex{{-1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    vertex{{1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    vertex{{0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
};
