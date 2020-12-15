#pragma once

#include "gl.h"

#include <map>
#include <memory>
#include <stdexcept>

struct gl_program {
	gl_program();
	gl_program(const gl_program &) = delete;
	~gl_program();

	static std::unique_ptr<gl_program> compile(const char *vert_src, const char *frag_src, const std::map<std::string, int> &frag_data_bindings);
	
	GLuint vert_shader;
	GLuint frag_shader;
	GLuint prog;
};