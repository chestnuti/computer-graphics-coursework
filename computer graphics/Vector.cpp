#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include "GamesEngineeringBase.h"
using namespace std;

class Camera;

class Mat4 {
public:
	float m[4][4];
	Mat4()
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = 0.0f;
	}

	Mat4 _Identity()
	{
		Mat4 mat;
		for (int i = 0; i < 4; i++)
			mat.m[i][i] = 1.0f;
		return mat;
	}

	float& operator[](int index)
	{
		int row = index / 4;
		int col = index % 4;
		return m[row][col];
	}
	float& operator()(int row, int col)
	{
		return m[row][col];
	}

	Mat4 operator*(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][0] * mat.m[0][j] +
					m[i][1] * mat.m[1][j] +
					m[i][2] * mat.m[2][j] +
					m[i][3] * mat.m[3][j];
			}
		}
		return result;
	}
	Vec4 operator*(const Vec4 vec) const
	{
		Vec4 result;
		result.v[0] = m[0][0] * vec.v[0] + m[0][1] * vec.v[1] + m[0][2] * vec.v[2] + m[0][3] * vec.v[3];
		result.v[1] = m[1][0] * vec.v[0] + m[1][1] * vec.v[1] + m[1][2] * vec.v[2] + m[1][3] * vec.v[3];
		result.v[2] = m[2][0] * vec.v[0] + m[2][1] * vec.v[1] + m[2][2] * vec.v[2] + m[2][3] * vec.v[3];
		result.v[3] = m[3][0] * vec.v[0] + m[3][1] * vec.v[1] + m[3][2] * vec.v[2] + m[3][3] * vec.v[3];
		return result;
	}
	Mat4& operator*=(const Mat4& mat)
	{
		*this = *this * mat;
		return *this;
	}
	Mat4 operator+(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][j] + mat.m[i][j];
			}
		}
		return result;
	}
	Mat4& operator+=(const Mat4& mat)
	{
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m[i][j] += mat.m[i][j];
			}
		}
		return *this;
	}
	Mat4 operator-(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][j] - mat.m[i][j];
			}
		}
		return result;
	}
	Mat4& operator-=(const Mat4& mat)
	{
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m[i][j] -= mat.m[i][j];
			}
		}
		return *this;
	}

	Mat4 Dot(const Mat4& mat) const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][0] * mat.m[0][j] +
					m[i][1] * mat.m[1][j] +
					m[i][2] * mat.m[2][j] +
					m[i][3] * mat.m[3][j];
			}
		}
		return result;
	}

	Mat4 Transpose() const
	{
		Mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[j][i];
			}
		}
		return result;
	}

	Mat4 RotateX(float angle)
	{
		Mat4 mat = _Identity();
		float rad = angle * (float)M_PI / 180.0f;
		float c = cosf(rad);
		float s = sinf(rad);
		mat.m[1][1] = c;  mat.m[1][2] = -s;
		mat.m[2][1] = s;  mat.m[2][2] = c;
		return mat;
	}
	Mat4 RotateY(float angle)
	{
		Mat4 mat = _Identity();
		float rad = angle * (float)M_PI / 180.0f;
		float c = cosf(rad);
		float s = sinf(rad);
		mat.m[0][0] = c;  mat.m[0][2] = s;
		mat.m[2][0] = -s;  mat.m[2][2] = c;
		return mat;
	}
	Mat4 RotateZ(float angle)
	{
		Mat4 mat = _Identity();
		float rad = angle * (float)M_PI / 180.0f;
		float c = cosf(rad);
		float s = sinf(rad);
		mat.m[0][0] = c;  mat.m[0][1] = -s;
		mat.m[1][0] = s;  mat.m[1][1] = c;
		return mat;
	}
	Mat4 Translate(float x, float y, float z)
	{
		Mat4 mat = _Identity();
		mat.m[0][3] = x;
		mat.m[1][3] = y;
		mat.m[2][3] = z;
		return mat;
	}
	Mat4 Scale(float x, float y, float z)
	{
		Mat4 mat = _Identity();
		mat.m[0][0] = x;
		mat.m[1][1] = y;
		mat.m[2][2] = z;
		return mat;
	}

	Mat4 invert()
	{
		Mat4 inv;
		inv[0] = inv[5] * inv[10] * inv[15] - inv[5] * inv[11] * inv[14] - inv[9] * inv[6] * inv[15] + inv[9] * inv[7] * inv[14] + inv[13] * inv[6] * inv[11] - inv[13] * inv[7] * inv[10];
		inv[4] = -inv[4] * inv[10] * inv[15] + inv[4] * inv[11] * inv[14] + inv[8] * inv[6] * inv[15] - inv[8] * inv[7] * inv[14] - inv[12] * inv[6] * inv[11] + inv[12] * inv[7] * inv[10];
		inv[8] = inv[4] * inv[9] * inv[15] - inv[4] * inv[11] * inv[13] - inv[8] * inv[5] * inv[15] + inv[8] * inv[7] * inv[13] + inv[12] * inv[5] * inv[11] - inv[12] * inv[7] * inv[9];
		inv[12] = -inv[4] * inv[9] * inv[14] + inv[4] * inv[10] * inv[13] + inv[8] * inv[5] * inv[14] - inv[8] * inv[6] * inv[13] - inv[12] * inv[5] * inv[10] + inv[12] * inv[6] * inv[9];
		inv[1] = -inv[1] * inv[10] * inv[15] + inv[1] * inv[11] * inv[14] + inv[9] * inv[2] * inv[15] - inv[9] * inv[3] * inv[14] - inv[13] * inv[2] * inv[11] + inv[13] * inv[3] * inv[10];
		inv[5] = inv[0] * inv[10] * inv[15] - inv[0] * inv[11] * inv[14] - inv[8] * inv[2] * inv[15] + inv[8] * inv[3] * inv[14] + inv[12] * inv[2] * inv[11] - inv[12] * inv[3] * inv[10];
		inv[9] = -inv[0] * inv[9] * inv[15] + inv[0] * inv[11] * inv[13] + inv[8] * inv[1] * inv[15] - inv[8] * inv[3] * inv[13] - inv[12] * inv[1] * inv[11] + inv[12] * inv[3] * inv[9];
		inv[13] = inv[0] * inv[9] * inv[14] - inv[0] * inv[10] * inv[13] - inv[8] * inv[1] * inv[14] + inv[8] * inv[2] * inv[13] + inv[12] * inv[1] * inv[10] - inv[12] * inv[2] * inv[9];
		inv[2] = inv[1] * inv[6] * inv[15] - inv[1] * inv[7] * inv[14] - inv[5] * inv[2] * inv[15] + inv[5] * inv[3] * inv[14] + inv[13] * inv[2] * inv[7] - inv[13] * inv[3] * inv[6];
		inv[6] = -inv[0] * inv[6] * inv[15] + inv[0] * inv[7] * inv[14] + inv[4] * inv[2] * inv[15] - inv[4] * inv[3] * inv[14] - inv[12] * inv[2] * inv[7] + inv[12] * inv[3] * inv[6];
		inv[10] = inv[0] * inv[5] * inv[15] - inv[0] * inv[7] * inv[13] - inv[4] * inv[1] * inv[15] + inv[4] * inv[3] * inv[13] + inv[12] * inv[1] * inv[7] - inv[12] * inv[3] * inv[5];
		inv[14] = -inv[0] * inv[5] * inv[14] + inv[0] * inv[6] * inv[13] + inv[4] * inv[1] * inv[14] - inv[4] * inv[2] * inv[13] - inv[12] * inv[1] * inv[6] + inv[12] * inv[2] * inv[5];
		inv[3] = -inv[1] * inv[6] * inv[11] + inv[1] * inv[7] * inv[10] + inv[5] * inv[2] * inv[11] - inv[5] * inv[3] * inv[10] - inv[9] * inv[2] * inv[7] + inv[9] * inv[3] * inv[6];
		inv[7] = inv[0] * inv[6] * inv[11] - inv[0] * inv[7] * inv[10] - inv[4] * inv[2] * inv[11] + inv[4] * inv[3] * inv[10] + inv[8] * inv[2] * inv[7] - inv[8] * inv[3] * inv[6];
		inv[11] = -inv[0] * inv[5] * inv[11] + inv[0] * inv[7] * inv[9] + inv[4] * inv[1] * inv[11] - inv[4] * inv[3] * inv[9] - inv[8] * inv[1] * inv[7] + inv[8] * inv[3] * inv[5];
		inv[15] = inv[0] * inv[5] * inv[10] - inv[0] * inv[6] * inv[9] - inv[4] * inv[1] * inv[10] + inv[4] * inv[2] * inv[9] + inv[8] * inv[1] * inv[6] - inv[8] * inv[2] * inv[5];
		float det = inv[0] * inv[0] + inv[1] * inv[4] + inv[2] * inv[8] + inv[3] * inv[12];
		if (det == 0) {
			// Handle this case 
		}
		det = 1.0 / det;
		for (int i = 0; i < 16; i++) {
			inv[i] = inv[i] * det;
		}
		return inv;
	}

};


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
	Vec3 operator*(const Vec3& pVec) const
	{
		return Vec3(v[0] * pVec.v[0], v[1] * pVec.v[1], v[2] * pVec.v[2]);
	}
	Vec3 operator*(const float val) const
	{
		return Vec3(v[0] * val, v[1] * val, v[2] * val);
	}
	Vec3 operator-() const
	{
		return Vec3(-v[0], -v[1], -v[2]);
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

	Vec3 Cross(const Vec3& v1)
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
	Vec4 operator*(const Vec4& pVec) const
	{
		return Vec4(v[0] * pVec.v[0], v[1] * pVec.v[1], v[2] * pVec.v[2], v[3] * pVec.v[3]);
	}
	Vec4 operator*(const float val) const
	{
		return Vec4(v[0] * val, v[1] * val, v[2] * val, v[3] * val);
	}
	Vec4 operator-() const
	{
		return Vec4(-v[0], -v[1], -v[2], -v[3]);
	}

	Vec4 normalize(void) {
		float len = 1.0f / sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
		return Vec4(v[0] * len, v[1] * len, v[2] * len, v[3] * len);
	}

	float Dot(const Vec4& pVec) const
	{
		return v[0] * pVec.v[0] + v[1] * pVec.v[1] + v[2] * pVec.v[2] + v[3] * pVec.v[3];
	}

	Vec4 cross(const Vec4& v1)
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



class Triangle {
	public:
	Vec4 v0;
	Vec4 v1;
	Vec4 v2;
	Vec3 color0;
	Vec3 color1;
	Vec3 color2;
	Triangle() : v0(), v1(), v2() {
		color0 = Vec3(255, 255, 255);
		color1 = Vec3(255, 255, 255);
		color2 = Vec3(255, 255, 255);
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2) : v0(pV0), v1(pV1), v2(pV2) {
		color0 = Vec3(255, 255, 255);
		color1 = Vec3(255, 255, 255);
		color2 = Vec3(255, 255, 255);
	}
	Triangle(const Vec4& pV0, const Vec4 pV1, const Vec4& pV2, const Vec3& pColor0, const Vec3& pColor1, const Vec3& pColor2)
		: v0(pV0), v1(pV1), v2(pV2), color0(pColor0), color1(pColor1), color2(pColor2) {}

	Vec4 checkPointInside(const Vec4& point)
	{
		//Calculate area of the triangle
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		Vec4 n = v0v1.cross(v0v2);
		float area = sqrtf(n.v[0] * n.v[0] + n.v[1] * n.v[1] + n.v[2] * n.v[2]) / 2.0f;
		//Calculate area of sub-triangles
		Vec4 v0p = point - v0;
		Vec4 v1p = point - v1;
		Vec4 v2p = point - v2;
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
		Vec3 color = color0 * alpha + color1 * beta + color2 * gamma;

		if (fabs(area - areaSum) <= 0.0f)
			return Vec4(color.v[0], color.v[1], color.v[2], 1.0f); // inside
		else
			return Vec4(color.v[0], color.v[1], color.v[2], 0.0f); // outside
	}

};


class Camera {
	Vec4 position;
	Vec4 target;
	float fov;
	double clipNear;
	double clipFar;
public:
	Camera() : position(0.0f, 0.0f, 0.0f, 1.0f), target(0.0f, 0.0f, -1.0f, 1.0f), fov(90.0f), clipNear(0.1), clipFar(1000.0) {}

	Vec4 getViewProjectionVector(Vec4 projection)
	{
		Mat4 viewMatrix;
		viewMatrix._Identity();
		viewMatrix = viewMatrix.Translate(-position.v[0], -position.v[1], -position.v[2]);
		viewMatrix = viewMatrix.RotateY(-atan2f(target.v[0] - position.v[0], target.v[2] - position.v[2]) * 180.0f / (float)M_PI);
		viewMatrix = viewMatrix.RotateX(atan2f(target.v[1] - position.v[1], target.v[2] - position.v[2]) * 180.0f / (float)M_PI);
		double distance = sqrt(pow(target.v[0] - position.v[0], 2) + pow(target.v[1] - position.v[1], 2) + pow(target.v[2] - position.v[2], 2));
		//TODO: Complete view-projection matrix calculation
	}
};



int main()
{
	//initialize a triangle
	Triangle tri(
		Vec4(200.0f, 200.0f, 0.0f, 1.0f),
		Vec4(150.0f, 300.0f, 0.0f, 1.0f),
		Vec4(250.0f, 300.0f, 0.0f, 1.0f),
		Vec3(255.0f, 0.0f, 0.0f),
		Vec3(0.0f, 255.0f, 0.0f),
		Vec3(0.0f, 0.0f, 255.0f)
	);

	//initialize a window
	GamesEngineeringBase::Window canvas;
	canvas.create(800, 600, "window");
	while (true) {
		canvas.clear();
		canvas.checkInput();
		if(canvas.keyPressed(VK_ESCAPE))
			break;

		//check points inside the triangle and draw them
		for (int y = 0; y < 600; y++)
		{
			for (int x = 0; x < 800; x++)
			{
				Vec4 point((float)x, (float)y, 0.0f, 1.0f);
				Vec4 color = tri.checkPointInside(point);
				if (color.v[3])
				{
					canvas.draw(x, y, color.v[0], color.v[1], color.v[2]); //white color for points inside the triangle
				}
			}
		}


		canvas.present();
	}


	return 0;
}