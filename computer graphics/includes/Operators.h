#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>


float clamp(float val, float minVal, float maxVal)
{
	using namespace std;
	return max(minVal, min(maxVal, val));
}

float remap(float val, float inMin, float inMax, float outMin, float outMax)
{
	return outMin + (outMax - outMin) * ((val - inMin) / (inMax - inMin));
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