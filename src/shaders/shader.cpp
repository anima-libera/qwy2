
#include "shader.hpp"
#include "embedded.hpp"
#include "opengl.hpp"
#include "utils.hpp"
#include <iostream>

namespace qwy2 {

/* Creates an OpenGL shader object with the given GLSL source code, then
 * compiles it and returns its id.
 * Returns 0 if compilation fails, debug_info being included in the error message. */
static GLuint shader_src_compile(const char* shader_src, GLuint shader_type,
	const char* debug_info)
{
	GLuint shader_openglid = glCreateShader(shader_type);
	glShaderSource(shader_openglid, 1, (const GLchar* const*)&shader_src, nullptr);
	glCompileShader(shader_openglid);
	GLint compile_status = 0;
	glGetShaderiv(shader_openglid, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		GLint length = 0;
		glGetShaderiv(shader_openglid, GL_INFO_LOG_LENGTH, &length);
		if (length == 0)
		{
			std::cerr << "OpenGL shader compilation error ";
			if (debug_info != nullptr)
			{
				std::cerr << "(" << debug_info << ")";
			}
			std::cerr << " without an OpenGL error message" << std::endl;
			/* Is it even possible to reach that point? */
		}
		else
		{
			GLchar* message = new GLchar[length];
			glGetShaderInfoLog(shader_openglid, length, &length, message);
			if (message[length-1] == '\n')
			{
				/* It looks better without this. */
				message[length-1] = '\0';
			}
			std::cerr << "OpenGL shader compilation error ";
			if (debug_info != nullptr)
			{
				std::cerr << "(" << debug_info << ")";
			}
			std::cerr << " \x1b[31m\"" << message << "\"\x1b[39m" << std::endl;
			delete[] message;
		}
		glDeleteShader(shader_openglid);
		return 0;
	}
	return shader_openglid;
}

/* Creates an OpenGL shader program object and attaches the given shaders to
 * it, then links it and returns its id.
 * The src_geom parameter can be nullptr as a geometry shader is optional in the
 * OpenGL rendering pipeline. */
static GLuint shader_program_build(
	const char* src_vert, const char* src_geom, const char* src_frag,
	const char* debug_info_vert, const char* debug_info_geom, const char* debug_info_frag,
	const char* debug_info)
{
	GLuint shader_vert_openglid =
		shader_src_compile(src_vert, GL_VERTEX_SHADER, debug_info_vert);
	if (shader_vert_openglid == 0)
	{
		return 0;
	}
	GLuint shader_geom_openglid;
	if (src_geom != nullptr)
	{
		shader_geom_openglid =
			shader_src_compile(src_geom, GL_GEOMETRY_SHADER, debug_info_geom);
		if (shader_geom_openglid == 0)
		{
			glDeleteShader(shader_vert_openglid);
			return 0;
		}
	}
	GLuint shader_frag_openglid =
		shader_src_compile(src_frag, GL_FRAGMENT_SHADER, debug_info_frag);
	if (shader_frag_openglid == 0)
	{
		glDeleteShader(shader_vert_openglid);
		if (src_geom != nullptr)
		{
			glDeleteShader(shader_geom_openglid);
		}
		return 0;
	}

	GLuint shader_program_openglid = glCreateProgram();
	glAttachShader(shader_program_openglid, shader_vert_openglid);
	if (src_geom != nullptr)
	{
		glAttachShader(shader_program_openglid, shader_geom_openglid);
	}
	glAttachShader(shader_program_openglid, shader_frag_openglid);

	glLinkProgram(shader_program_openglid);
	GLint link_status = 0;
	glGetProgramiv(shader_program_openglid, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		GLint length = 0;
		glGetProgramiv(shader_program_openglid, GL_INFO_LOG_LENGTH, &length);
		if (length == 0)
		{
			std::cerr << "OpenGL shader program linkage error ";
			if (debug_info != nullptr)
			{
				std::cerr << "(" << debug_info << ")";
			}
			std::cerr << " without an OpenGL error message" << std::endl;
			/* Is it even possible to reach that point? */
		}
		else
		{
			GLchar* message = new GLchar[length];
			glGetProgramInfoLog(shader_program_openglid, length, &length, message);
			if (message[length-1] == '\n')
			{
				/* It looks better without this. */
				message[length-1] = '\0';
			}
			std::cerr << "OpenGL shader program linkage error ";
			if (debug_info != nullptr)
			{
				std::cerr << "(" << debug_info << ")";
			}
			std::cerr << " \x1b[31m\"" << message << "\"\x1b[39m" << std::endl;
			delete[] message;
		}
		glDetachShader(shader_program_openglid, shader_vert_openglid);
		glDeleteShader(shader_vert_openglid);
		if (src_geom != nullptr)
		{
			glDetachShader(shader_program_openglid, shader_geom_openglid);
			glDeleteShader(shader_geom_openglid);
		}
		glDetachShader(shader_program_openglid, shader_frag_openglid);
		glDeleteShader(shader_frag_openglid);
		glDeleteProgram(shader_program_openglid);
		return 0;
	}

	glDetachShader(shader_program_openglid, shader_vert_openglid);
	glDeleteShader(shader_vert_openglid);
	if (src_geom != nullptr)
	{
		glDetachShader(shader_program_openglid, shader_geom_openglid);
		glDeleteShader(shader_geom_openglid);
	}
	glDetachShader(shader_program_openglid, shader_frag_openglid);
	glDeleteShader(shader_frag_openglid);
	return shader_program_openglid;
}

ErrorCode ShaderProgram::compile_given_src(
	const char* src_vert, const char* src_geom, const char* src_frag,
	const char* debug_info_vert, const char* debug_info_geom, const char* debug_info_frag,
	const char* debug_info)
{
	this->openglid = shader_program_build(src_vert, src_geom, src_frag,
		debug_info_vert, debug_info_geom, debug_info_frag, debug_info);
	return this->openglid == 0 ? ErrorCode::ERROR : ErrorCode::OK;
}

void ShaderProgram::cleanup()
{
	glDeleteProgram(this->openglid);
}

} /* qwy2 */
