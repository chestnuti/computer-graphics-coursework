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