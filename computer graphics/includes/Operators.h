#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>


double clamp(double val, double minVal, double maxVal)
{
	using namespace std;
	return max(minVal, min(maxVal, val));
}

double remap(double val, double inMin, double inMax, double outMin, double outMax)
{
	return outMin + (outMax - outMin) * ((val - inMin) / (inMax - inMin));
}

template<typename t>
t perspectiveCorrectInterpolateAttribute(t a0, t a1, t a2, double v0_w, double v1_w, double v2_w, double alpha, double beta, double gamma)
{
	double frag_w = ((alpha * v0_w) + (beta * v1_w) + (gamma * v2_w));
	t attrib[3];
	attrib[0] = a0 * alpha * v0_w;
	attrib[1] = a1 * beta * v1_w;
	attrib[2] = a2 * gamma * v2_w;
	return ((attrib[0] + attrib[1] + attrib[2]) / frag_w);
}