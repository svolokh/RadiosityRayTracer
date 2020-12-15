#pragma once
#define NOMINMAX

#include "windows.h"

#include "FreeImagePlus.h"
#include "pinhole_ray_computer.h"
#include "math.h"
#include "prng.h"
#include "object_tracer.h"
#include "vec3.h"
#include "timer.h"
#include "scene.h"
#include "radiosity_scene.h"

#include <iostream>
#include <random>
#include <memory>
#include <cstdint>

namespace rt {

template <
	typename ObjectTracer = object_tracer,
	typename RayComputer = pinhole_ray_computer, // How to compute ray directions for image plane points
	bool SuperSampling = false, // Whether to use supersampling
	size_t SS_XSamples = 4,  // If SuperSampling, how many X-coordinates to supersample
	size_t SS_YSamples = 4 // If SuperSampling, how many Y-coordinates to supersample
>
void raytrace_scene(const scene &scn, const char *outname, size_t width, size_t height, const typename RayComputer::params &rc_params = {}) {
	static constexpr auto xsamples(SuperSampling ? SS_XSamples : 1);
	static constexpr auto ysamples(SuperSampling ? SS_YSamples : 1);

	Timer render_timer;
	render_timer.startTimer();

	fipImage img(FIT_BITMAP, width, height, 24);

	const auto w{float(width)};
	const auto h{float(height)};
	const auto aspect(w/h);

	RayComputer rc(scn.cam, aspect, rc_params);

	float xc[xsamples];
	float yc[ysamples];
	prng gen;
	std::uniform_real_distribution<float> cdist(-0.5f, 0.5f);

	ObjectTracer tracer(scn);
	for(std::size_t y(0); y != height; ++y) {
		for(std::size_t x(0); x != width; ++x) {
			vec3 color(0.0f, 0.0f, 0.0f);
			if(SuperSampling) {
				for(auto it(std::begin(xc)); it != std::end(xc); ++it) {
					*it = x + cdist(gen);
				}
				for(auto it(std::begin(yc)); it != std::end(yc); ++it) {
					*it = y + cdist(gen);
				}
			} else {
				xc[0] = float(x);
				yc[0] = float(y);
			}
			for(auto i(0); i != xsamples; ++i) {
				for(auto j(0); j != ysamples; ++j) {
					auto sx(float(xc[i] + 0.5f)/w);
					auto sy(float(yc[j] + 0.5f)/h);
					auto r(rc.compute_ray(sx, sy));
					auto L(tracer.trace(r));
					color += L;
				}
			}
			color /= xsamples*ysamples;
			RGBQUAD q;
			q.rgbRed = u08(color.r*255);
			q.rgbGreen = u08(color.g*255);
			q.rgbBlue = u08(color.b*255);
			q.rgbReserved = 255;
			img.setPixelColor(x, y, &q);
		}
	}

	render_timer.stopTimer();
	std::cout << "Rendered scene in " << render_timer.getTime() << " sec" << std::endl;

	std::cout << "Writing " << outname << std::endl;
	img.save(outname);

	std::cout << std::endl;
}

std::unique_ptr<scene> load_scene(const char *filename, const shader_bindings &bindings = {}) {
	Timer total_timer;
	Timer scn_timer;
	total_timer.startTimer();
	scn_timer.startTimer();
	auto scn_io(readScene(filename));
	std::unique_ptr<scene> scn(new scene(scn_io, bindings));
	std::cout << "Loading " << filename << std::endl;
	scn_timer.stopTimer();
	std::cout << "Loaded scene in " << scn_timer.getTime() << " sec" << std::endl;
	return scn;
}

std::unique_ptr<radiosity_scene> load_radiosity_scene(const char *filename, std::size_t steps = 8192, const radiosity_scene::params_type &params = {}, const shader_bindings &bindings = {}) {
	Timer scn_timer;
	Timer render_timer;
	Timer radiosity_timer;
	scn_timer.startTimer();
	auto scn_io(readScene(filename));
	std::unique_ptr<radiosity_scene> scn(new radiosity_scene(scn_io, params, bindings));
	std::cout << "Loading " << filename << std::endl;
	scn_timer.stopTimer();
	std::cout << "Loaded scene in " << scn_timer.getTime() << " sec" << std::endl;
	std::cout << "Running radiosity..." << std::endl;
	radiosity_timer.startTimer();
	for(auto i(0); i != steps; ++i) {
		scn->step();
	}
	radiosity_timer.stopTimer();
	std::cout << "Performed " << steps << " radiosity steps in " << radiosity_timer.getTime() << " sec" << std::endl;
	return scn;
}

}