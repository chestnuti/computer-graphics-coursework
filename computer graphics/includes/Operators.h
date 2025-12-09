#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>

void DebugPrint(const std::string& message) {
	OutputDebugStringA((message + "\n").c_str());
}


float clamp(float val, float minVal, float maxVal)
{
	using namespace std;
	return max(minVal, min(maxVal, val));
}

template<typename t>
t remap(t val, t inMin, t inMax, t outMin, t outMax)
{
	return outMin + (outMax - outMin) * ((val - inMin) / (inMax - inMin));
}

template<typename t>
t remap_clamp(t val, t inMin, t inMax, t outMin, t outMax)
{
	return clamp(remap(val, inMin, inMax, outMin, outMax), outMin, outMax);
}

Vec4 slerp(Vec4 v0, Vec4 v1, float t)
{
	// normalize input vectors
	Vec4 nv0 = v0.normalize();
	Vec4 nv1 = v1.normalize();
	// compute the cosine of the angle between the two vectors
	float dot = v0.Dot(v1);
	if (dot < 0.0f) {
		nv1 = -nv1;
		dot = -dot;
	}
	// if the dot product is very close to 1, use linear interpolation
	if (dot > 0.9995f)
		return (nv0 + (nv1 - nv0) * t).normalize();
	// compute the angle between the two vectors
	float theta_0 = acosf(dot);
	float theta = acosf(dot) * t;
	// compute the sin of the angles
	float sin_theta = sinf(theta);
	float sin_theta_0 = sinf(theta_0);
	// compute the coefficients
	float s0 = cosf(theta) - dot * sin_theta / sin_theta_0;
	float s1 = sin_theta / sin_theta_0;
	// compute the interpolated vector
	return (nv0 * s0 + nv1 * s1).normalize();
}

template<typename t>
t perspectiveCorrectInterpolateAttribute(t a0, t a1, t a2, float v0_w, float v1_w, float v2_w, float alpha, float beta, float gamma)
{
	float frag_w = ((alpha * v0_w) + (beta * v1_w) + (gamma * v2_w));
	t attrib[3];
	attrib[0] = a0 * alpha * v0_w;
	attrib[1] = a1 * beta * v1_w;
	attrib[2] = a2 * gamma * v2_w;
	return ((attrib[0] + attrib[1] + attrib[2]) / frag_w);
}


Vec3 anyOrthogonal(Vec3 vec) {
	if (abs(vec.v[0]) > abs(vec.v[2]))
		return Vec3(-vec.v[1], vec.v[0], 0.0);
	else
		return Vec3(0.0, -vec.v[2], vec.v[1]);
}



Vec4 quatFromTo(Vec3 a, Vec3 b) {
	Vec3 u = a.normalize();
	Vec3 v = b.normalize();
	float c = u.Dot(v);

	if (c > 0.999999) {
		return Vec4(0, 0, 0, 1);
	}
	if (c < -0.999999) {
		Vec3 axis = anyOrthogonal(u).normalize();
		return Vec4(axis.v[0], axis.v[1], axis.v[2], 0); // 180бу
	}

	Vec3 w = v.cross(u);
	Vec4 q(w.v[0], w.v[1], w.v[2], 1 + c);
	return q.normalize();
}
