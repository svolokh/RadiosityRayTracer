#include "material.h"

namespace rt {

material material::interpolate(const material &m0, float w0, const material &m1, float w1, const material &m2, float w2) {
	material m;
	m.diff_color = m0.diff_color*w0 + m1.diff_color*w1 + m2.diff_color*w2;
	m.amb_color = m0.amb_color*w0 + m1.amb_color*w1 + m2.amb_color*w2;
	m.spec_color = m0.spec_color*w0 + m1.spec_color*w1 + m2.spec_color*w2;
	m.emiss_color = m0.emiss_color*w0 + m1.emiss_color*w1 + m2.emiss_color*w2;
	m.shininess = m0.shininess*w0 + m1.shininess*w1 + m2.shininess*w2;
	m.ktran = m0.ktran*w0 + m1.ktran*w1 + m2.ktran*w2;
	return m;
}

material material::interpolate(const material &m0, float w0, const material &m1, float w1, const material &m2, float w2, const material &m3, float w3) {
	material m;
	m.diff_color = m0.diff_color*w0 + m1.diff_color*w1 + m2.diff_color*w2 + m3.diff_color*w3;
	m.amb_color = m0.amb_color*w0 + m1.amb_color*w1 + m2.amb_color*w2 + m3.amb_color*w3;
	m.spec_color = m0.spec_color*w0 + m1.spec_color*w1 + m2.spec_color*w2 + m3.spec_color*w3;
	m.emiss_color = m0.emiss_color*w0 + m1.emiss_color*w1 + m2.emiss_color*w2 + m3.emiss_color*w3;
	m.shininess = m0.shininess*w0 + m1.shininess*w1 + m2.shininess*w2 + m3.shininess*w3;
	m.ktran = m0.ktran*w0 + m1.ktran*w1 + m2.ktran*w2 + m3.ktran*w3;
	return m;
}

}