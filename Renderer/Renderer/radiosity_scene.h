#pragma once

#include "camera.h"
#include "patch.h"
#include "scene_io.h"
#include "scene.h"
#include "shader_bindings.h"
#include "gl.h"
#include "gl_program.h"

#include <optional.hpp>

#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

namespace rt {

using std::experimental::optional;

struct radiosity_scene : scene {
	struct params_type {
		float patch_area = 0.05f; // The desired patch area.
		std::size_t hc_res = 512; // Number of cells in the X and Y direction on the hemicube face
	};

	// io: Scene information.
	// bindings: Shader bindings.
	// Each pair of triangle primitives will be treated as a quad for subdivision purposes (if the pair does not form a valid quad it will be discarded).
	radiosity_scene(SceneIO *io, const params_type &params = {}, const shader_bindings &bindings = {});
	~radiosity_scene();
	radiosity_scene(const radiosity_scene &) = delete;
	
	void debug_render_patches(std::size_t highlight, float aspect); // Render patch locations to the current OpenGL context. `highlight` specifies the index of the patch to highlight (or 0 not to highlight any patches).
	void debug_render_hemicube(std::size_t patch_index, std::size_t face, std::size_t highlight); // Render a face of a patch's hemicube to the current OpenGL context. `face` order is [top, left, right, back, front].
	
	// Perform a light bouncing step.
	void step();

	// Set random colors (seeded by index) on all patches.
	void random_colors();

private:
	void init_patches();
	void init_hemicube(); // prepares the necessary OpenGL objects to render hemicube faces
	void compute_form_factors(const patch &p, std::vector<float> &buf, optional<std::function<void(std::size_t face)>> debug_fn = {});

public:
	const params_type params;

	std::vector<std::unique_ptr<patch>> patches; // Array of patches in the scene. The 0th index is reserved as a "null" patch (where light escapes to infinity).
	std::vector<std::unique_ptr<patch_state>> states; // Array of patch states in the scene. The 0th index is reserved for the null patch's state.

private:
	std::unique_ptr<gl_program> hc_prog; // shader program for rendering hemicube faces
	GLuint hc_vao; // patch vertex array for rendering hemicube faces
	GLuint hc_vbo; // patch vertex buffer for rendering hemicube faces
	std::size_t hc_num_indices; // number of indices in hc_vao
	GLuint hc_fbo; // framebuffer for rendering hemicube faces
	GLuint hc_index_tex; // texture to which patch indices are rendered when rendering hemicube faces
	GLuint hc_depth_tex; // renderbuffer used as z-buffer when rendering hemicube faces

	// debug fields
	std::unique_ptr<gl_program> debug_prog; // shader program for debug_render_patches
	std::unique_ptr<gl_program> hc_debug_prog; // shader program for debug_render_hemicube
	GLuint hc_debug_vao; // vertex array for debug_render_hemicube
	GLuint hc_debug_vbo; // vertex buffer for debug_render_hemicube
	GLuint hc_debug_colors_tex; // 1D texture for storing patch colors

	std::vector<float> ffs_buf; // buffer used for storing form factors
	std::vector<GLuint> pixel_buf; // buffer used for reading pixels from hemicube face framebuffers
};

}