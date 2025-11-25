#pragma once

#include "Vector.h"
#include "Operators.h"



class Triangle {
public:
	Vec4 v0;
	Vec4 v1;
	Vec4 v2;
	Vec3 color0;
	Vec3 color1;
	Vec3 color2;
	Vec4 normal;
	Vec4 tangent;

	Triangle() : v0(), v1(), v2() {
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		normal = v0v1.cross(v0v2).normalize();
		tangent = normal.cross(v0v1).normalize();
		color0 = Vec3(255, 255, 255);
		color1 = Vec3(255, 255, 255);
		color2 = Vec3(255, 255, 255);
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2) : v0(pV0), v1(pV1), v2(pV2) {
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		normal = v0v1.cross(v0v2).normalize();
		tangent = normal.cross(v0v1).normalize();
		color0 = Vec3(255, 255, 255);
		color1 = Vec3(255, 255, 255);
		color2 = Vec3(255, 255, 255);
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2, const Vec3& pColor0, const Vec3& pColor1, const Vec3& pColor2)
		: v0(pV0), v1(pV1), v2(pV2), color0(pColor0), color1(pColor1), color2(pColor2) {
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		normal = v0v1.cross(v0v2).normalize();
		tangent = normal.cross(v0v1).normalize();
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2, const Vec3& pColor0, const Vec3& pColor1, const Vec3& pColor2, const Vec4& pNormal, const Vec4& pTangent)
		: v0(pV0), v1(pV1), v2(pV2), color0(pColor0), color1(pColor1), color2(pColor2), normal(pNormal), tangent(pTangent) {
	}

	Vec4 checkPointInside(const Vec4& point)
	{
		//Calculate area of the triangle
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		//v0v1.v[2] = 0.0f;
		//v0v2.v[2] = 0.0f;
		Vec4 n = v0v1.cross(v0v2);
		float area = sqrtf(n.v[0] * n.v[0] + n.v[1] * n.v[1] + n.v[2] * n.v[2]) / 2.0f;
		//Calculate area of sub-triangles
		Vec4 v0p = point - v0;
		Vec4 v1p = point - v1;
		Vec4 v2p = point - v2;
		//v0p.v[2] = 0.0f;
		//v1p.v[2] = 0.0f;
		//v2p.v[2] = 0.0f;
		Vec4 n0 = v0p.cross(v1p);
		Vec4 n1 = v0p.cross(v2p);
		Vec4 n2 = v1p.cross(v2p);
		float area0 = sqrtf(n0.v[0] * n0.v[0] + n0.v[1] * n0.v[1] + n0.v[2] * n0.v[2]) / 2.0f;
		float area1 = sqrtf(n1.v[0] * n1.v[0] + n1.v[1] * n1.v[1] + n1.v[2] * n1.v[2]) / 2.0f;
		float area2 = sqrtf(n2.v[0] * n2.v[0] + n2.v[1] * n2.v[1] + n2.v[2] * n2.v[2]) / 2.0f;
		float areaSum = area0 + area1 + area2;
		//Barycentric coordinates for color interpolation
		float alpha = area1 / area;
		float beta = area2 / area;
		float gamma = area0 / area;

		if (fabs(area - areaSum) <= 0.1f)
			return Vec4(alpha, beta, gamma, 1.0f); // inside
		else
			return Vec4(alpha, beta, gamma, 0.0f); // outside
	}

};