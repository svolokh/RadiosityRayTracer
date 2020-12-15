#define NOMINMAX

#include <windows.h>
#include <stdio.h>

#include "raytracer.h"
#include "radiosity_scene.h"
#include "radiosity_object_tracer.h"
#include "gl.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>

typedef unsigned char u08;

int main(int argc, char *argv[]) {
	using namespace rt;

	if(!glfwInit()) {
		std::cerr << "glfwInit() failed" << std::endl;
		return 1;
	}

	// Create an invisible window so we can use OpenGL.
	auto window(glfwCreateWindow(800, 800, "Radiosity", NULL, NULL));
	glfwMakeContextCurrent(window);
	if(glewInit() != GLEW_OK) {
		std::cerr << "glewInit() failed" << std::endl;
		return 1;
	}
	glfwHideWindow(window);

	radiosity_scene::params_type params;
	params.patch_area = 0.01f;
	params.hc_res = 700;

	// Examples
	{
		auto scn(load_radiosity_scene("../Scenes/box.ascii", 4096, params));
		raytrace_scene<radiosity_object_tracer<true>>(*scn, "../Output/box.png", 1500, 1500);
		raytrace_scene<radiosity_object_tracer<false>>(*scn, "../Output/box-nointerp.png", 1500, 1500);
	}

	{
		auto scn(load_radiosity_scene("../Scenes/sphere.ascii", 4096, params));
		raytrace_scene<radiosity_object_tracer<true>>(*scn, "../Output/sphere.png", 1500, 1500);
		raytrace_scene<radiosity_object_tracer<false>>(*scn, "../Output/sphere-nointerp.png", 1500, 1500);
	}

	{
		auto scn(load_radiosity_scene("../Scenes/specular.ascii", 4096, params));
		raytrace_scene<radiosity_object_tracer<true>>(*scn, "../Output/specular.png", 1500, 1500);
		raytrace_scene<radiosity_object_tracer<false>>(*scn, "../Output/specular-nointerp.png", 1500, 1500);
	}

	{
		auto mat_shader([](material &mat, const shader_params &params) {
			auto r((1.0f + std::cos(4.0f*RT_PI*params.uv[0]))/2.0f);
			auto g((1.0f + std::sin(4.0f*RT_PI*params.uv[1]))/2.0f);
			auto b(1.0f);
			mat.diff_color = {r, g, b};
		});
		auto scn(load_radiosity_scene("../Scenes/box.ascii", 4096, params, {
			{4, {&default_intersection_shader, mat_shader}},
			{6, {&default_intersection_shader, mat_shader}},
			{7, {&default_intersection_shader, mat_shader}}
		}));
		raytrace_scene<radiosity_object_tracer<true>>(*scn, "../Output/box-shaders.png", 1500, 1500);
		raytrace_scene<radiosity_object_tracer<false>>(*scn, "../Output/box-shaders-nointerp.png", 1500, 1500);
	}

	{
		auto scn(load_radiosity_scene("../Scenes/banner.ascii", 4096, params));
		raytrace_scene<radiosity_object_tracer<true>>(*scn, "../Output/banner.png", 1500, 1500);
		raytrace_scene<radiosity_object_tracer<false>>(*scn, "../Output/banner-nointerp.png", 1500, 1500);
	}

	if(glfwGetWindowAttrib(window, GLFW_VISIBLE)) {
		glfwSwapBuffers(window);
		while(!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}
	glfwTerminate();
	return 0;
}