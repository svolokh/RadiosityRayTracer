#include "scene.h"
#include "sphere.h"
#include "triangle.h"
#include "vec3.h"
#include "math.h"
#include "config.h"

#include <optional.hpp>
#include <any.hpp>

#include <iostream>

namespace rt {

using std::experimental::optional;

static triangle_uv compute_triangle_uv(std::size_t tri_index, PolygonIO &t0, PolygonIO &t1) {
	// Decides on a UVs for a pair of triangles (assumed to be a quad) by first finding the vertices of the adjacent edge, then making the outer vertex uv for the first triangle {0,0}, outer vertex uv for the second triangle {1,1}, and finally picking the UVs for the adjacent edge vertices based on the order of indices
	std::array<std::array<vec3, 3>, 2> verts;
	verts[0] = {vec3(t0.vert[0].pos), vec3(t0.vert[1].pos), vec3(t0.vert[2].pos)};
	verts[1] = {vec3(t1.vert[0].pos), vec3(t1.vert[1].pos), vec3(t1.vert[2].pos)};
	std::array<std::array<std::size_t, 2>, 2> same;
	std::size_t not_same;
	std::size_t k(0);
	for(std::size_t i(0); i != 3 && k != 2; ++i) {
		for(std::size_t j(0); j != 3; ++j) {
			if(verts[0][i] == verts[1][j]) {
				same[0][k] = i;
				same[1][k++] = j;
				break;
			}
		}
	}
	if(k != 2 || same[0][0] == same[0][1] || same[1][0] == same[1][1]) {
		// Not a quad, return null UVs
		return {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};
	}
	for(std::size_t i(0); i != 3; ++i) {
		for(std::size_t j(0); j != 2; ++j) {
			if(same[tri_index][j] == i) {
				goto next;
			}
		}
		not_same = i;
		goto done;
next:
		continue;
	}
	assert(false);
done:
	;
	std::array<vec2, 3> uvs;
	uvs[not_same] = tri_index == 0 ? vec2(0.0f, 0.0f) : vec2(1.0f, 1.0f);
	uvs[same[tri_index][0]] = same[0][0] < same[0][1] ? vec2(1.0f, 0.0f) : vec2(0.0f, 1.0f);
	uvs[same[tri_index][1]] = same[0][0] >= same[0][1] ? vec2(1.0f, 0.0f) : vec2(0.0f, 1.0f);
	return triangle_uv(uvs[0], uvs[1], uvs[2]);
}

scene::scene(SceneIO *io, const shader_bindings &bindings) {
	// Parse camera.
	cam.pos = io->camera->position;
	cam.dir = normalize(vec3(io->camera->viewDirection));
	cam.focal_dist = io->camera->focalDistance;
	cam.ortho_up = normalize(vec3(io->camera->orthoUp));
	cam.fov = io->camera->verticalFOV;
	// Parse lights.
	for(LightIO *light_io(io->lights); light_io != nullptr; light_io = light_io->next) {
		light l;
		switch(light_io->type) {
			case POINT_LIGHT:
				l.info = point_light_info{light_io->position};
				break;
			case DIRECTIONAL_LIGHT: 
				l.info = directional_light_info{normalize(vec3(light_io->direction))};
				break;
			default:
				assert(!"Unsupported light type");
		}
		l.color = light_io->color;
		lights.emplace_back(std::move(l));
	}
	// Parse objects.
	std::size_t obj_id(0);
	for(auto obj_io(io->objects); obj_io != nullptr; obj_io = obj_io->next) {
		std::unique_ptr<object> obj(new object(obj_id++));
		{
			auto it(bindings.find(obj->id));
			if(it != bindings.end()) {
				obj->intersect_shader = it->second.first;
				obj->mat_shader = it->second.second;
				assert(obj->intersect_shader);
				assert(obj->mat_shader);
			}
		}
		if(obj_io->name != nullptr) {
			obj->name = std::string(obj_io->name);
		}
		for(auto i(0); i != obj_io->numMaterials; ++i) {
			auto &mat_io(obj_io->material[i]);
			material mat;
			mat.diff_color = mat_io.diffColor;
			mat.amb_color = mat_io.ambColor;
			mat.spec_color = mat_io.specColor;
			mat.emiss_color = mat_io.emissColor;
			mat.shininess = mat_io.shininess;
			mat.ktran = mat_io.ktran;
			obj->materials.emplace_back(std::move(mat));
		}
		switch(obj_io->type) {
			case SPHERE_OBJ: {
				auto sph_io(static_cast<SphereIO *>(obj_io->data));
				std::unique_ptr<sphere> sph(new sphere(obj.get(), vec3(sph_io->origin), sph_io->radius));
				obj->primitives.emplace_back(sph.get());
				primitives.emplace_back(std::move(sph));
				break;
			}
			case POLYSET_OBJ: {
				auto ps_io(static_cast<PolySetIO *>(obj_io->data));
				assert(ps_io->type == POLYSET_TRI_MESH);
				assert(!ps_io->hasTextureCoords);
				for(std::size_t i(0); i != ps_io->numPolys; ++i) {
					auto &poly_io(ps_io->poly[i]);
					assert(poly_io.numVertices == 3);
					std::unique_ptr<triangle_normal_base> tri_normal;
					std::unique_ptr<triangle_material_base> tri_mat;
					vec3 a(poly_io.vert[0].pos);
					vec3 b(poly_io.vert[1].pos);
					vec3 c(poly_io.vert[2].pos);
					if(ps_io->normType == PER_FACE_NORMAL) {
						tri_normal.reset(new triangle_normal<false>(normalize(cross(b - a, c - a))));
					} else if(ps_io->normType == PER_VERTEX_NORMAL) {
						tri_normal.reset(new triangle_normal<true>(normalize(vec3(poly_io.vert[0].norm)), normalize(vec3(poly_io.vert[1].norm)), normalize(vec3(poly_io.vert[2].norm))));
					} else {
						assert(!"Unrecognized normType");
					}
					if(ps_io->materialBinding == PER_OBJECT_MATERIAL) {
						tri_mat.reset(new triangle_material<false>());
					} else if(ps_io->materialBinding == PER_VERTEX_MATERIAL) {
						tri_mat.reset(new triangle_material<true>(poly_io.vert[0].materialIndex, poly_io.vert[1].materialIndex, poly_io.vert[2].materialIndex));
					} else {
						assert(!"Unrecognized materialBinding");
					}

					// Quad hack for texture mapping triangles: we consider each pair of triangles to be a quad
					auto i0(i - (i % 2));
					auto i1(i - (i % 2) + 1);
					if(i1 == ps_io->numPolys) {
						// If we got an odd number of triangles, use the previous triangle as the second triangle in the pair
						i1 = i0 - 1;
					}
					auto uv(compute_triangle_uv(i % 2, ps_io->poly[i0], ps_io->poly[i1]));
					std::unique_ptr<triangle> tri(new triangle(obj.get(), a, b, c, std::move(tri_normal), uv, std::move(tri_mat)));
					obj->primitives.emplace_back(tri.get());
					primitives.emplace_back(std::move(tri));
				}
				break;
			}
		}
		objects.emplace_back(std::move(obj));
	}
#ifdef RT_TREE
	// Build primitive tree.
	{
		std::vector<primitive *> prims;
		prims.reserve(primitives.size());
		std::transform(primitives.begin(), primitives.end(), std::back_inserter(prims), [](const std::unique_ptr<primitive> &p) {
			return p.get();
		});
		tree_.emplace(prims);
	}
#endif
}

bool scene::intersect(const ray &r, intersect_info &info, primitive *&pr) const {
#ifdef RT_TREE
	return tree_->intersect(r, info, pr);
#else
	auto hit(false);
	intersect_info info_min;
	info_min.t = std::numeric_limits<float>::infinity();
	const primitive *pr_min(nullptr);
	{
		intersect_info info;
		for(auto &pr : primitives) {
			if(pr->intersect(r, info)) {
				hit = true;
				if(info.t < info_min.t) {
					info_min = info;
					pr_min = pr.get();
				}
			}
		}
	}
	if(hit) {
		assert(pr_min != nullptr);
		pr = const_cast<primitive *>(pr_min);
		info = info_min;
		return true;
	} else {
		return false;
	}
#endif
}

}