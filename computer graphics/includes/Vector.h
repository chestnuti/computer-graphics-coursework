#pragma once
#include "Matrix.h"
using namespace std;



class Vec3 {
public:
	float v[3];
	Vec3() : v{ 0.0f, 0.0f, 0.0f } {}
	Vec3(float x, float y, float z) : v{ x, y, z } {}

	Vec3 operator+(const Vec3& pVec) const
	{
		return Vec3(v[0] + pVec.v[0], v[1] + pVec.v[1], v[2] + pVec.v[2]);
	}
	Vec3& operator+=(const Vec3& pVec)
	{
		v[0] += pVec.v[0];
		v[1] += pVec.v[1];
		v[2] += pVec.v[2];
		return *this;
	}
	Vec3& operator-=(const Vec3& pVec)
	{
		v[0] -= pVec.v[0];
		v[1] -= pVec.v[1];
		v[2] -= pVec.v[2];
		return *this;
	}
	Vec3 operator*(const Vec3& pVec) const
	{
		return Vec3(v[0] * pVec.v[0], v[1] * pVec.v[1], v[2] * pVec.v[2]);
	}
	Vec3 operator*(const float val) const
	{
		return Vec3(v[0] * val, v[1] * val, v[2] * val);
	}
	Vec3 operator/(const float val) const
	{
		return Vec3(v[0] / val, v[1] / val, v[2] / val);
	}
	Vec3 operator-() const
	{
		return Vec3(-v[0], -v[1], -v[2]);
	}
	Vec3 operator-(const Vec3& pVec) const
	{
		return Vec3(v[0] - pVec.v[0], v[1] - pVec.v[1], v[2] - pVec.v[2]);
	}

	Vec3 normalize(void)
	{
		float len = 1.0f / sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		return Vec3(v[0] * len, v[1] * len, v[2] * len);
	}
	float normalize_GetLength()
	{
		float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		float len = 1.0f / length;
		v[0] *= len; v[1] *= len; v[2] *= len;
		return length;
	}

	float Dot(const Vec3& pVec) const
	{
		return v[0] * pVec.v[0] + v[1] * pVec.v[1] + v[2] * pVec.v[2];
	}

	Vec3 cross(const Vec3& v1) const
	{
		return Vec3(v1.v[1] * v[2] - v1.v[2] * v[1],
			v1.v[2] * v[0] - v1.v[0] * v[2],
			v1.v[0] * v[1] - v1.v[1] * v[0]);
	}

};

float Dot(const Vec3& v1, const Vec3& v2)
{
	return v1.v[0] * v2.v[0] + v1.v[1] * v2.v[1] + v1.v[2] * v2.v[2];
}

Vec3 Max(const Vec3& v1, const Vec3& v2)
{
	return Vec3(max(v1.v[0], v2.v[0]),
		max(v1.v[1], v2.v[1]),
		max(v1.v[2], v2.v[2]));
}



class Vec4 {
public:
	float v[4];
	Vec4() : v{ 0.0f, 0.0f, 0.0f, 0.0f } {}
	Vec4(float x, float y, float z, float w) : v{ x, y, z, w } {}

	Vec4 operator+(const Vec4& pVec) const
	{
		return Vec4(v[0] + pVec.v[0], v[1] + pVec.v[1], v[2] + pVec.v[2], v[3] + pVec.v[3]);
	}
	Vec4 operator-(const Vec4& pVec) const
	{
		return Vec4(v[0] - pVec.v[0], v[1] - pVec.v[1], v[2] - pVec.v[2], v[3] - pVec.v[3]);
	}
	Vec4& operator+=(const Vec4& pVec)
	{
		v[0] += pVec.v[0];
		v[1] += pVec.v[1];
		v[2] += pVec.v[2];
		v[3] += pVec.v[3];
		return *this;
	}
	Vec4& operator-=(const Vec4& pVec)
	{
		v[0] -= pVec.v[0];
		v[1] -= pVec.v[1];
		v[2] -= pVec.v[2];
		v[3] -= pVec.v[3];
		return *this;
	}
	Vec4 operator*(const Vec4& pVec) const
	{
		return Vec4(v[0] * pVec.v[0], v[1] * pVec.v[1], v[2] * pVec.v[2], v[3] * pVec.v[3]);
	}
	Vec4 operator*(const float val) const
	{
		return Vec4(v[0] * val, v[1] * val, v[2] * val, v[3] * val);
	}
	Vec4 operator/(const float val) const
	{
		return Vec4(v[0] / val, v[1] / val, v[2] / val, v[3] / val);
	}
	Vec4 operator-() const
	{
		return Vec4(-v[0], -v[1], -v[2], -v[3]);
	}
	bool operator==(const Vec4& pVec) const
	{
		return (v[0] == pVec.v[0] && v[1] == pVec.v[1] && v[2] == pVec.v[2] && v[3] == pVec.v[3]);
	}

	Vec4 normalize(void) {
		float len = 1.0f / sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
		return Vec4(v[0] * len, v[1] * len, v[2] * len, v[3] * len);
	}

	float Dot(const Vec4& pVec) const
	{
		return v[0] * pVec.v[0] + v[1] * pVec.v[1] + v[2] * pVec.v[2] + v[3] * pVec.v[3];
	}

	Vec4 cross(const Vec4& v1) const
	{
		return Vec4(v1.v[1] * v[2] - v1.v[2] * v[1],
			v1.v[2] * v[0] - v1.v[0] * v[2],
			v1.v[0] * v[1] - v1.v[1] * v[0],
			0.0f);
	}

	float getLength() const
	{
		return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
	}

	Vec4 transform(const Mat4& mat)
	{
		Vec4 result;
		result.v[0] = mat.m[0][0] * v[0] + mat.m[0][1] * v[1] + mat.m[0][2] * v[2] + mat.m[0][3] * v[3];
		result.v[1] = mat.m[1][0] * v[0] + mat.m[1][1] * v[1] + mat.m[1][2] * v[2] + mat.m[1][3] * v[3];
		result.v[2] = mat.m[2][0] * v[0] + mat.m[2][1] * v[1] + mat.m[2][2] * v[2] + mat.m[2][3] * v[3];
		result.v[3] = mat.m[3][0] * v[0] + mat.m[3][1] * v[1] + mat.m[3][2] * v[2] + mat.m[3][3] * v[3];
		return result;
	}

};