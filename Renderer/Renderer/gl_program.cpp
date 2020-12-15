#include "gl_program.h"

#include <sstream>

gl_program::gl_program() {
	vert_shader = glCreateShader(GL_VERTEX_SHADER);
	XGL_POST();
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	XGL_POST();
	prog = glCreateProgram();
	XGL_POST();
}

gl_program::~gl_program() {
	XGL(glDeleteProgram(prog));
	XGL(glDeleteShader(frag_shader));
	XGL(glDeleteShader(vert_shader));
}

std::unique_ptr<gl_program> gl_program::compile(const char *vert_src, const char *frag_src, const std::map<std::string, int> &frag_data_bindings) {
	std::unique_ptr<gl_program> prog(new gl_program());
	XGL(glShaderSource(prog->vert_shader, 1, &vert_src, nullptr));
	XGL(glShaderSource(prog->frag_shader, 1, &frag_src, nullptr));
	XGL(glCompileShader(prog->vert_shader));
	GLint res;
	XGL(glGetShaderiv(prog->vert_shader, GL_COMPILE_STATUS, &res));
	if(res == GL_FALSE) {
		char buf[1024];
		XGL(glGetShaderInfoLog(prog->vert_shader, (sizeof buf)-1, nullptr, buf));
		std::ostringstream oss;
		oss << "failed to compile vertex shader:\n" << buf;
		throw std::runtime_error(oss.str());
	}
	XGL(glCompileShader(prog->frag_shader));
	XGL(glGetShaderiv(prog->frag_shader, GL_COMPILE_STATUS, &res));
	if(res == GL_FALSE) {
		char buf[1024];
		XGL(glGetShaderInfoLog(prog->frag_shader, (sizeof buf)-1, nullptr, buf));
		std::ostringstream oss;
		oss << "failed to compile fragment shader:\n" << buf;
		throw std::runtime_error(oss.str());
	}
	XGL(glAttachShader(prog->prog, prog->vert_shader));
	XGL(glAttachShader(prog->prog, prog->frag_shader));
	for(const auto &p : frag_data_bindings) {
		XGL(glBindFragDataLocation(prog->prog, p.second, p.first.c_str()));
	}
	XGL(glLinkProgram(prog->prog));
	XGL(glGetProgramiv(prog->prog, GL_LINK_STATUS, &res));
	if(res == GL_FALSE) {
		char buf[1024];
		XGL(glGetProgramInfoLog(prog->prog, (sizeof buf)-1, nullptr, buf));
		std::ostringstream oss;
		oss << "failed to link gl_program:\n" << buf;
		throw std::runtime_error(oss.str());
	}
	return prog;
}
