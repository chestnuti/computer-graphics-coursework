#pragma once

#include "Vector.h"
#include "Operators.h"

#define ScreenWidth 960
#define ScreenHeight 540


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
		: v0(pV0), v1(pV1), v2(pV2), color0(pColor0), color1(pColor1), color2(pColor2) {
	}

	Vec4 checkPointInside(const Vec4& point)
	{
		//Calculate area of the triangle
		Vec4 v0v1 = v1 - v0;
		Vec4 v0v2 = v2 - v0;
		v0v1.v[2] = 0.0;
		v0v2.v[2] = 0.0;
		Vec4 n = v0v1.cross(v0v2);
		double area = sqrtf(n.v[0] * n.v[0] + n.v[1] * n.v[1] + n.v[2] * n.v[2]) / 2.0;
		//Calculate area of sub-triangles
		Vec4 v0p = point - v0;
		Vec4 v1p = point - v1;
		Vec4 v2p = point - v2;
		v0p.v[2] = 0.0;
		v1p.v[2] = 0.0;
		v2p.v[2] = 0.0;
		Vec4 n0 = v0p.cross(v1p);
		Vec4 n1 = v0p.cross(v2p);
		Vec4 n2 = v1p.cross(v2p);
		double area0 = sqrtf(n0.v[0] * n0.v[0] + n0.v[1] * n0.v[1] + n0.v[2] * n0.v[2]) / 2.0;
		double area1 = sqrtf(n1.v[0] * n1.v[0] + n1.v[1] * n1.v[1] + n1.v[2] * n1.v[2]) / 2.0;
		double area2 = sqrtf(n2.v[0] * n2.v[0] + n2.v[1] * n2.v[1] + n2.v[2] * n2.v[2]) / 2.0;
		double areaSum = area0 + area1 + area2;
		//Barycentric coordinates for color interpolation
		double alpha = area1 / area;
		double beta = area2 / area;
		double gamma = area0 / area;
		//Vec3 color = color0 * alpha + color1 * beta + color2 * gamma;
		//debug: z depth as color
		Vec3 color0 = Vec3(v0.v[3], v0.v[3], v0.v[3]) * gamma;
		Vec3 color1 = Vec3(v1.v[3], v1.v[3], v1.v[3]) * alpha;
		Vec3 color2 = Vec3(v2.v[3], v2.v[3], v2.v[3]) * beta;
		Vec3 color = color0 + color1 + color2;
		if (fabs(area - areaSum) <= 0.1)
			return Vec4(color.v[0], color.v[1], color.v[2], 1.0); // inside
		else
			return Vec4(color.v[0], color.v[1], color.v[2], 0.0); // outside
	}

};


class Camera {
public:
	Vec4 position;
	Vec4 outcoming;
	Vec4 up;
	double fov;
	double clipNear;
	double clipFar;

	Camera() : position(0.0, 0.0, 50.0, 1.0), outcoming(0.0, 1.0, -1.0, 1.0), up(0.0, 0.0, 1.0, 1.0), fov(90.0), clipNear(0.01), clipFar(1000.0) {
		Vec4 tangent = up.cross(outcoming).normalize();
	}

	Mat4 getLookatMatrix()
	{
		//Calculate camera tangent vector
		Vec4 tangent = up.cross(outcoming).normalize();
		//Recalculate up vector
		Vec4 _up = outcoming.cross(tangent).normalize();
		//Create rotation matrix
		Mat4 rotation = Mat4()._Identity();
		rotation.m[0][0] = tangent.v[0];
		rotation.m[0][1] = tangent.v[1];
		rotation.m[0][2] = tangent.v[2];
		rotation.m[1][0] = _up.v[0];
		rotation.m[1][1] = _up.v[1];
		rotation.m[1][2] = _up.v[2];
		rotation.m[2][0] = outcoming.v[0];
		rotation.m[2][1] = outcoming.v[1];
		rotation.m[2][2] = outcoming.v[2];
		//Create translation matrix
		Mat4 translation = Mat4()._Identity();
		translation.m[0][3] = -position.v[0];
		translation.m[1][3] = -position.v[1];
		translation.m[2][3] = -position.v[2];
		//Combine rotation and translation
		Mat4 transform = rotation * translation;
		return transform;
	}

	Vec4 getViewProjectionVector(Vec4 projection)
	{
		Vec4 transformed = projection.transform(getLookatMatrix());
		//Perspective projection
		double aspectRatio = static_cast<double>(ScreenWidth / ScreenHeight); //window size
		double fovRad = 1.0 / tanf((fov * 0.5) * (double)M_PI / 180.0);
		Mat4 projectionMatrix = Mat4()._Identity();
		projectionMatrix.m[0][0] = aspectRatio * fovRad;
		projectionMatrix.m[1][1] = fovRad;
		projectionMatrix.m[2][2] = clipFar / (clipFar - clipNear);
		projectionMatrix.m[2][3] = (-clipFar * clipNear) / (clipFar - clipNear);
		projectionMatrix.m[3][2] = 1.0;
		Vec4 projected = transformed.transform(projectionMatrix);
		//Normalize to screen space
		projected.v[0] /= projected.v[3];
		projected.v[1] /= projected.v[3];
		projected.v[2] /= projected.v[3];
		//Convert to screen coordinates
		projected.v[0] = (projected.v[0] + 1.0) * 0.5 * ScreenWidth; //window width
		projected.v[1] = (1.0 - (projected.v[1] + 1.0) * 0.5) * ScreenHeight; //window height

		Vec4 distanceVec = projection - position;
		double distance = distanceVec.getLength();
		if (outcoming.normalize().Dot(distanceVec.normalize()) <= 0 || distance < clipNear || distance > clipFar)
			projected.v[3] = 0.0; //not visible
		else
			projected.v[3] = remap(distance, clipNear, clipFar, 0.0, 255.0); //visible with depth info


		return projected;
	}


};