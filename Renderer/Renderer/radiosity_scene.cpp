#include "radiosity_scene.h"
#include "triangle.h"
#include "sphere.h"
#include "prng.h"
#include "sphere_patch.h"
#include "quad_patch.h"
#include "patch_vertex.h"
#include "gl.h"
#include "mat4.h"
#include "shadow_tracer.h"

#include <cstddef>
#include <vector>
#include <numeric>

namespace rt {

// hemicube top face vertex shader
static const char *hc_vert_shader = R"GLSL(
#version 140
uniform mat4 vp; // view-projection transform
in vec3 pos;
in uint index;
flat out uint v_index;

void main() {
	v_index = index;
	gl_Position = vp * vec4(pos, 1.0);
}
)GLSL";

// hemicube face fragment shader
static const char *hc_frag_shader = R"GLSL(
#version 140
flat in uint v_index;
out uint f_index;

void main() {
	f_index = v_index;
}
)GLSL";

static const char *hc_debug_vert_shader = R"GLSL(
#version 140
in vec2 pos;
in vec2 texcoord;
out vec2 v_texcoord;

void main() {
	v_texcoord = texcoord;
	gl_Position = vec4(pos, 0, 1);
}
)GLSL";

static const char *hc_debug_frag_shader = R"GLSL(
#version 140
uniform usampler2D tex;
uniform sampler1D colors;
uniform uint highlight;
in vec2 v_texcoord;
out vec4 f_color;

void main() {
	uint index = texture(tex, v_texcoord).r;
	if(index > 0u) {
		float s = highlight != 0u ? (index == highlight ? 1.0 : 0.3) : 1.0;
		f_color = vec4(s*texelFetch(colors, int(index), 0).rgb, 1.0);
	} else {
		f_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
)GLSL";

static const char *debug_vert_shader = R"GLSL(
#version 140
uniform sampler1D colors;
uniform mat4 vp; // view-projection transform
uniform uint highlight;
in vec3 pos;
in uint index;
out vec3 v_color;

void main() {
	float s = highlight != 0u ? (index == highlight ? 1.0 : 0.3) : 1.0;
	v_color = s*texelFetch(colors, int(index), 0).rgb;
	gl_Position = vp*vec4(pos, 1.0);
}
)GLSL";

static const char *debug_frag_shader = R"GLSL(
#version 140
in vec3 v_color;
out vec4 f_color;

void main() {
	f_color = vec4(v_color, 1.0f);
}
)GLSL";

struct viewport_guard {
	viewport_guard(GLint x, GLint y, GLint width, GLint height) {
		XGL(glGetIntegerv(GL_VIEWPORT, orig_));
		XGL(glViewport(x, y, width, height));
	}

	~viewport_guard() {
		XGL(glViewport(orig_[0], orig_[1], orig_[2], orig_[3]));
	}

private:
	GLint orig_[4];
};

radiosity_scene::radiosity_scene(SceneIO *io, const params_type &params, const shader_bindings &bindings) :
	scene(io, bindings),
	params(params)
{
	init_patches();
	init_hemicube();
	ffs_buf.resize(patches.size());
	pixel_buf.resize(params.hc_res * params.hc_res);
}

radiosity_scene::~radiosity_scene() {
	XGL(glDeleteVertexArrays(1, &hc_vao));
	XGL(glDeleteBuffers(1, &hc_vbo));
	XGL(glDeleteFramebuffers(1, &hc_fbo));
	XGL(glDeleteTextures(1, &hc_index_tex));
	XGL(glDeleteTextures(1, &hc_depth_tex));

	XGL(glDeleteVertexArrays(1, &hc_debug_vao));
	XGL(glDeleteBuffers(1, &hc_debug_vbo));
}

void radiosity_scene::debug_render_patches(std::size_t highlight, float aspect) {
	XGL(glUseProgram(debug_prog->prog));
	XGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	XGL(glEnable(GL_DEPTH_TEST));
	auto colors_loc(glGetUniformLocation(debug_prog->prog, "colors"));
	auto vp_loc(glGetUniformLocation(debug_prog->prog, "vp"));
	auto highlight_loc(glGetUniformLocation(debug_prog->prog, "highlight"));
	auto proj(mat4::perspective(radians(90.0f), aspect, 0.1f, 10.0f));
	auto vp(proj * mat4::look_at(cam.pos, cam.pos + cam.dir, cam.ortho_up));
	XGL(glActiveTexture(GL_TEXTURE0));
	XGL(glBindTexture(GL_TEXTURE_1D, hc_debug_colors_tex));
	{
		std::vector<float> colors_buf;
		colors_buf.resize(3*patches.size());
		for(auto i(1); i != states.size(); ++i) {
			auto c(3*i);
			auto &s(states[i]);
			colors_buf[c] = std::min(s->energy.r, 1.0f);
			colors_buf[c+1] = std::min(s->energy.g, 1.0f);
			colors_buf[c+2] = std::min(s->energy.b, 1.0f);
		}
		XGL(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, colors_buf.size()/3, 0, GL_RGB, GL_FLOAT, colors_buf.data()));
	}
	XGL(glUniform1i(colors_loc, 0));
	XGL(glUniformMatrix4fv(vp_loc, 1, GL_FALSE, reinterpret_cast<float *>(&vp.data)));
	XGL(glUniform1ui(highlight_loc, GLuint(highlight)));
	XGL(glBindVertexArray(hc_vao));
	XGL(glDrawArrays(GL_TRIANGLES, 0, hc_num_indices));
	XGL(glBindVertexArray(0));
	XGL(glDisable(GL_DEPTH_TEST));
}

void radiosity_scene::debug_render_hemicube(std::size_t patch_index, std::size_t face, std::size_t highlight) {
	compute_form_factors(*patches[patch_index], ffs_buf, std::function<void(std::size_t)>([&](std::size_t f) {
		if(f != face) {
			return;
		}
		XGL(glUseProgram(hc_debug_prog->prog));
		auto tex_loc(glGetUniformLocation(hc_debug_prog->prog, "tex"));
		auto colors_loc(glGetUniformLocation(hc_debug_prog->prog, "colors"));
		auto highlight_loc(glGetUniformLocation(hc_debug_prog->prog, "highlight"));
		XGL(glActiveTexture(GL_TEXTURE0));
		XGL(glBindTexture(GL_TEXTURE_2D, hc_index_tex)); 
		XGL(glActiveTexture(GL_TEXTURE1));
		XGL(glBindTexture(GL_TEXTURE_1D, hc_debug_colors_tex));
		{
			std::vector<float> colors_buf;
			colors_buf.resize(3*patches.size());
			for(auto i(1); i != states.size(); ++i) {
				auto c(3*i);
				auto &s(states[i]);
				colors_buf[c] = s->energy.r;
				colors_buf[c+1] = s->energy.g;
				colors_buf[c+2] = s->energy.b;
			}
			XGL(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, colors_buf.size(), 0, GL_RGB, GL_FLOAT, colors_buf.data()));
		}
		XGL(glActiveTexture(GL_TEXTURE0));
		XGL(glUniform1i(tex_loc, 0));
		XGL(glUniform1i(colors_loc, 1));
		XGL(glUniform1ui(highlight_loc, highlight));
		XGL(glBindVertexArray(hc_debug_vao));
		XGL(glDrawArrays(GL_TRIANGLES, 0, 6));
		XGL(glBindVertexArray(0));
	}));
	auto total(0.0f);
	for(auto i(0); i != patches.size(); ++i) {
		total += ffs_buf[i];
	}
}

void radiosity_scene::step() {
	// "A Progressive Refinement Approach to Fast Radiosity Image Generation"
	if(patches.size() < 2) {
		return;
	}
	// Find patch with the most unshot energy
	patch_state *shooter(states[1].get());
	std::size_t shooter_index(1);
	for(auto i(2); i != states.size(); ++i) {
		auto &st(states[i]);
		if(st->unshot_energy_total*st->p->area() > shooter->unshot_energy_total*st->p->area()) {
			shooter = st.get();
			shooter_index = i;
		}
	}
	// Compute form factors for that patch
	compute_form_factors(*shooter->p, ffs_buf);
	// Shoot
	auto shooter_area(shooter->p->area());
	auto shoot([&](std::size_t j) {
		auto &st(states[j]);
		auto k(ffs_buf[j]*shooter_area/st->p->area());
		vec3 delta{
			shooter->unshot_energy.r*st->p->mat().diff_color.r*k,
			shooter->unshot_energy.g*st->p->mat().diff_color.g*k,
			shooter->unshot_energy.b*st->p->mat().diff_color.b*k
		};
		st->energy += delta;
		st->unshot_energy += delta;
		st->unshot_energy_total += delta.r + delta.g + delta.b;
	});
	for(auto j(1); j != shooter_index; ++j) {
		shoot(j);
	}
	for(auto j(shooter_index+1); j != states.size(); ++j) {
		shoot(j);
	}
	shooter->unshot_energy = shooter->unshot_energy*ffs_buf[shooter_index];
	shooter->unshot_energy_total = shooter->unshot_energy.r + shooter->unshot_energy.g + shooter->unshot_energy.b;
}

void radiosity_scene::random_colors() {
	prng rng(1775203, 1631191, 2512649, 1160207);
	for(auto i(1); i != states.size(); ++i) {
		auto c(3*i);
		auto &s(states[i]);
		s->energy.r = (rng() % 10000)/10000.0f; 
		s->energy.g = (rng() % 10000)/10000.0f;
		s->energy.b = (rng() % 10000)/10000.0f;
	}
}

void radiosity_scene::init_patches() {
	patches.emplace_back(nullptr);
	states.emplace_back(new patch_state());
	// Create patches and patch states
	for(auto &&obj : objects) {
		if(obj->primitives.size() == 0) {
			continue;
		}
		switch(obj->primitives[0]->type()) {
			case type_triangle: {
				if(obj->primitives.size() >= 2) {
					for(std::size_t i(0); i < obj->primitives.size(); i += 2) {
						auto patch_states(std::make_shared<std::vector<std::vector<patch_state *>>>());
						// Consider every two triangles to be a quad (same algorithm as in scene.cpp)
						auto i0(i - (i % 2));
						auto i1(i - (i % 2) + 1);
						if(i1 == obj->primitives.size()) {
							// If we got an odd number of triangles, use the previous triangle as the second triangle in the pair
							i1 = i0 - 1;
						}
						auto t0(static_cast<triangle *>(obj->primitives[i0]));
						auto t1(static_cast<triangle *>(obj->primitives[i1]));
						if(!t0->uv.known) {
							continue;
						}
						assert(t1->uv.known);
						auto it_a(std::find(t0->uv.uvs.begin(), t0->uv.uvs.end(), vec2(0.0f, 0.0f)));
						auto it_b(std::find(t0->uv.uvs.begin(), t0->uv.uvs.end(), vec2(1.0f, 0.0f)));
						auto it_c(std::find(t1->uv.uvs.begin(), t1->uv.uvs.end(), vec2(1.0f, 1.0f)));
						auto it_d(std::find(t0->uv.uvs.begin(), t0->uv.uvs.end(), vec2(0.0f, 1.0f)));
						assert(it_a != t0->uv.uvs.end());
						assert(it_b != t0->uv.uvs.end());
						assert(it_c != t1->uv.uvs.end());
						assert(it_d != t0->uv.uvs.end());

						auto ia(it_a - t0->uv.uvs.begin());
						auto va((&t0->a)[ia]);
						auto ma(t0->mat->mat(obj.get(), ia));
						auto na(t0->normal->normal(ia));

						auto ib(it_b - t0->uv.uvs.begin());
						auto vb((&t0->a)[ib]);
						auto mb(t0->mat->mat(obj.get(), ib));
						auto nb(t0->normal->normal(ib));

						auto ic(it_c - t1->uv.uvs.begin());
						auto vc((&t1->a)[ic]);
						auto mc(t1->mat->mat(obj.get(), ic));
						auto nc(t1->normal->normal(ic));

						auto id(it_d - t0->uv.uvs.begin());
						auto vd((&t0->a)[id]);
						auto md(t0->mat->mat(obj.get(), id));
						auto nd(t0->normal->normal(id));

						auto pmn([&](float u, float v, vec3 &pos, material &mat, vec3 &normal) {
							if(u < 1.0f - v) {
								auto alpha(1.0f - u - v);
								auto beta(u);
								auto gamma(v);
								pos = va*alpha + vb*beta + vd*gamma;
								mat = material::interpolate(ma, alpha, mb, beta, md, gamma);
								normal = na*alpha + nb*beta + nd*gamma;
							} else {
								auto alpha(u + v - 1.0f);
								auto beta(1.0f - u);
								auto gamma(1.0f - v);
								pos = vc*alpha + vd*beta + vb*gamma;
								mat = material::interpolate(mc, alpha, md, beta, mb, gamma);
								normal = nc*alpha + nd*beta + nb*gamma;
							}
						});
						vec3 pos1; material mat1; vec3 normal1;
						vec3 pos2; material mat2; vec3 normal2;
						vec3 pos3; material mat3; vec3 normal3;
						vec3 pos4; material mat4; vec3 normal4;
						std::size_t udiv, vdiv;
						std::tie(udiv, vdiv) = quad_patch::subdivisions(va, vb, vc, vd, params.patch_area);
						patch_states->resize(vdiv);
						auto u_step(1.0f/udiv);
						auto v_step(1.0f/vdiv);
						for(std::size_t vi(0); vi != vdiv; ++vi) {
							auto v(vi*v_step);
							(*patch_states)[vi].resize(udiv);
							for(std::size_t ui(0); ui != udiv; ++ui) {
								auto u(ui*u_step);
								pmn(u, v, pos1, mat1, normal1);
								pmn(u + u_step, v, pos2, mat2, normal2);
								pmn(u + u_step, v + v_step, pos3, mat3, normal3);
								pmn(u, v + v_step, pos4, mat4, normal4);
								auto c((pos1 + pos2 + pos3 + pos4)/4.0f);
								auto normal(normalize(normal1 + normal2 + normal3 + normal4));
								vec2 uv(u + u_step/2.0f, v + v_step/2.0f);
								shader_params par {
									c,
									normal,
									uv
								};
								auto mat(material::interpolate(mat1, 0.25f, mat2, 0.25f, mat3, 0.25f, mat4, 0.25f));
								obj->mat_shader(mat, par);
								std::unique_ptr<quad_patch> patch(new quad_patch(obj.get(), mat, normal, {pos1, pos2, pos3, pos4}));
								auto st(patch->new_state());
								(*patch_states)[vi][ui] = st.get();
								states.emplace_back(std::move(st));
								patches.emplace_back(std::move(patch));
							}
						}
						t0->radiosity_patches = patch_states;
						t0->patches_u_loop = false;
						t0->patches_v_loop = false;
						t1->radiosity_patches = patch_states;
						t1->patches_u_loop = false;
						t1->patches_v_loop = false;
					}
				}
				break;
			}
			case type_sphere: {
				for(auto i(0); i != obj->primitives.size(); ++i) {
					auto patch_states(std::make_shared<std::vector<std::vector<patch_state *>>>());
					auto sph(static_cast<sphere *>(obj->primitives[i]));
					std::size_t udiv, vdiv;
					std::tie(udiv, vdiv) = sphere_patch::subdivisions(sph->radius, params.patch_area);
					patch_states->resize(vdiv);
					auto u_step(1.0f/udiv);
					auto v_step(1.0f/vdiv);
					for(std::size_t vi(0); vi != vdiv; ++vi) {
						auto v(vi*v_step);
						(*patch_states)[vi].resize(udiv);
						for(std::size_t ui(0); ui != udiv; ++ui) {
							auto u(ui*u_step);
							auto pos(sphere::pos(sph->center, sph->radius, u + u_step/2.0f, v + v_step/2.0f));
							auto normal(normalize(pos - sph->center));
							vec2 uv(u + u_step/2.0f, v + v_step/2.0f);
							shader_params par {
								pos,
								normal,
								uv
							};
							auto mat(obj->materials[0]);
							obj->mat_shader(mat, par);
							std::unique_ptr<sphere_patch> patch(new sphere_patch(obj.get(), mat, u, u + u_step, v, v + v_step, sph->center, sph->radius));
							auto st(patch->new_state());
							(*patch_states)[vi][ui] = st.get();
							states.emplace_back(std::move(st));
							patches.emplace_back(std::move(patch));
						}
					}
					sph->radiosity_patches = patch_states;
					sph->patches_u_loop = true;
					sph->patches_v_loop = true;
				}
				break;
			}
		}
	}
}

void radiosity_scene::init_hemicube() {
	hc_prog = gl_program::compile(hc_vert_shader, hc_frag_shader, {{"f_index", 0}});
	XGL(glGenVertexArrays(1, &hc_vao));
	XGL(glGenBuffers(1, &hc_vbo));
	XGL(glGenFramebuffers(1, &hc_fbo));
	XGL(glGenTextures(1, &hc_index_tex));
	XGL(glGenTextures(1, &hc_depth_tex));
	
	debug_prog = gl_program::compile(debug_vert_shader, debug_frag_shader, {{"f_color", 0}});
	hc_debug_prog = gl_program::compile(hc_debug_vert_shader, hc_debug_frag_shader, {{"f_color", 0}});
	XGL(glGenVertexArrays(1, &hc_debug_vao));
	XGL(glGenBuffers(1, &hc_debug_vbo));
	XGL(glGenTextures(1, &hc_debug_colors_tex));

	auto pos_loc(glGetAttribLocation(hc_prog->prog, "pos"));
	XGL_POST();
	auto index_loc(glGetAttribLocation(hc_prog->prog, "index"));
	XGL_POST();

	XGL(glBindVertexArray(hc_vao));
	XGL(glBindBuffer(GL_ARRAY_BUFFER, hc_vbo));
	{
		std::vector<patch_vertex> verts;
		for(auto i(1); i != patches.size(); ++i) {
			patches[i]->render(verts, i);
		}
		XGL(glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(patch_vertex), verts.data(), GL_STATIC_DRAW));
		hc_num_indices = verts.size();
	}
	XGL(glEnableVertexAttribArray(pos_loc));
	XGL(glEnableVertexAttribArray(index_loc));
	XGL(glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(patch_vertex), reinterpret_cast<const GLvoid *>(offsetof(patch_vertex, pos))));
	XGL(glVertexAttribIPointer(index_loc, 1, GL_UNSIGNED_INT, sizeof(patch_vertex), reinterpret_cast<const GLvoid *>(offsetof(patch_vertex, index))));
	XGL(glBindVertexArray(0));

	XGL(glBindTexture(GL_TEXTURE_2D, hc_index_tex));
	XGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, params.hc_res, params.hc_res, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr));
	XGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	XGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	XGL(glBindTexture(GL_TEXTURE_2D, hc_depth_tex));
	XGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, params.hc_res, params.hc_res, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr));
	XGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	XGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	XGL(glBindFramebuffer(GL_FRAMEBUFFER, hc_fbo));
	XGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hc_index_tex, 0));
	XGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, hc_depth_tex, 0));
	XGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	{
		static float debug_verts[] = {
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f
		};

		auto pos_loc(glGetAttribLocation(hc_debug_prog->prog, "pos"));
		auto texcoord_loc(glGetAttribLocation(hc_debug_prog->prog, "texcoord"));

		XGL(glBindVertexArray(hc_debug_vao));
		XGL(glBindBuffer(GL_ARRAY_BUFFER, hc_debug_vbo));
		XGL(glBufferData(GL_ARRAY_BUFFER, sizeof debug_verts, debug_verts, GL_STATIC_DRAW));
		XGL(glEnableVertexAttribArray(pos_loc));
		XGL(glEnableVertexAttribArray(texcoord_loc));
		XGL(glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, reinterpret_cast<const GLvoid *>(0)));
		XGL(glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, reinterpret_cast<const GLvoid *>(sizeof(float)*2)));

		XGL(glBindVertexArray(0));
	}

	XGL(glBindTexture(GL_TEXTURE_1D, hc_debug_colors_tex));
	XGL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	XGL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

template <bool Side>
static void read_fb(std::vector<GLuint> &pixel_buf, std::vector<float> &ffs, const radiosity_scene::params_type &params) {
	assert(pixel_buf.size() >= params.hc_res*params.hc_res);
	auto s(params.hc_res);
	auto fs{float(params.hc_res)};
	auto da(4.0f/(fs*fs));
	XGL(glReadBuffer(GL_COLOR_ATTACHMENT0));
	XGL(glReadPixels(0, 0, s, s, GL_RED_INTEGER, GL_UNSIGNED_INT, pixel_buf.data()));
	for(std::size_t j(Side ? s/2 : 0); j != s; ++j) {
		for(std::size_t i(0); i != s; ++i) {
			auto x(2.0f*i/fs - 1.0f);
			auto y(2.0f*j/fs - 1.0f);
			if(Side) {
				assert(y >= 0.0f);
			}
			auto div(x*x + y*y + 1.0f);
			auto df(Side ? da*y/(RT_PI*div*div) : da/(RT_PI*div*div));
			ffs[pixel_buf[j*s + i]] += df;
		}
	}
}

void radiosity_scene::compute_form_factors(const patch &p, std::vector<float> &buf, optional<std::function<void(std::size_t face)>> debug_fn) {
	static const auto near(0.1f);
	static const auto far(10.0f);
	auto center(p.center());
	auto normal(p.normal());
	auto right(normalize(cross(normal, p.surface_dir()))); // the direction facing the right face of the hemicube (arbitrarily chosen to be perpendicular to the normal)
	auto front(normalize(cross(right, normal))); // the direction facing the front face of the hemicube
	auto proj(mat4::perspective(radians(90.0f), 1.0f, near, far));
	auto vp_loc(glGetUniformLocation(hc_prog->prog, "vp"));
	XGL_POST();
	assert(buf.size() == patches.size());
	std::fill(buf.begin(), buf.end(), 0.0f);
	// top 
	{
		auto vp(proj*mat4::look_at(center, center + normal, front));
		{
			viewport_guard vg(0, 0, params.hc_res, params.hc_res);
			XGL(glEnable(GL_DEPTH_TEST));
			XGL(glBindFramebuffer(GL_FRAMEBUFFER, hc_fbo));
			XGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			XGL(glUseProgram(hc_prog->prog));
			XGL(glUniformMatrix4fv(vp_loc, 1, GL_FALSE, reinterpret_cast<float *>(&vp.data)));
			XGL(glBindVertexArray(hc_vao));
			XGL(glDrawArrays(GL_TRIANGLES, 0, hc_num_indices));
			read_fb<false>(pixel_buf, buf, params);
			XGL(glBindVertexArray(0));
			XGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			XGL(glDisable(GL_DEPTH_TEST));
		}
		if(debug_fn) {
			(*debug_fn)(0);
		}
	}
	std::array<mat4, 4> side_vps = {
		proj * mat4::look_at(center, center - right, normal), // left
		proj * mat4::look_at(center, center + right, normal), // right
		proj * mat4::look_at(center, center - front, normal), // back
		proj * mat4::look_at(center, center + front, normal)  // front
	};
	std::size_t face(1);
	for(const auto &vp : side_vps) {
		{
			viewport_guard vg(0, 0, params.hc_res, params.hc_res);
			XGL(glEnable(GL_DEPTH_TEST));
			XGL(glBindFramebuffer(GL_FRAMEBUFFER, hc_fbo));
			XGL(glUseProgram(hc_prog->prog));
			XGL(glBindVertexArray(hc_vao));
			XGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			XGL(glUniformMatrix4fv(vp_loc, 1, GL_FALSE, reinterpret_cast<const float *>(&vp.data)));
			XGL(glDrawArrays(GL_TRIANGLES, 0, hc_num_indices));
			read_fb<true>(pixel_buf, buf, params); 
			XGL(glBindVertexArray(0));
			XGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			XGL(glDisable(GL_DEPTH_TEST));
		}
		if(debug_fn) {
			(*debug_fn)(face);
		}
		++face;
	}
}

}