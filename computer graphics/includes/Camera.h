#pragma once
#include "Vector.h"

#define ScreenWidth 1024
#define ScreenHeight 1024


class Camera {
public:
	Vec3 position;
	Vec3 target;
	Vec3 up;
	float fov;
	float clipNear;
	float clipFar;

	Camera() : position(0.0f, 3.0f, 3.0f), target(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f), fov(90.0f), clipNear(0.01f), clipFar(100.0f) {
	}

	Mat4 getLookatMatrix()
	{
		//Calculate camera tangent vector
		Vec3 outcoming = (target - position).normalize();
		Vec3 tangent = up.cross(outcoming).normalize();
		//Recalculate up vector
		Vec3 _up = outcoming.cross(tangent).normalize();
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

	Mat4 getViewProjectionMatrix()
	{
		float aspectRatio = static_cast<float>(ScreenWidth / ScreenHeight); //window size
		float fovRad = 1.0f / tanf((fov * 0.5f) * (float)M_PI / 180.0f);
		Mat4 projectionMatrix = Mat4()._Identity();
		projectionMatrix.m[0][0] = aspectRatio * fovRad;
		projectionMatrix.m[1][1] = fovRad;
		projectionMatrix.m[2][2] = clipFar / (clipFar - clipNear);
		projectionMatrix.m[2][3] = (-clipFar * clipNear) / (clipFar - clipNear);
		projectionMatrix.m[3][2] = 1.0f;
		return projectionMatrix * getLookatMatrix();
	}

	Vec4 getViewProjectionVector(Vec4 projection)
	{
		Vec4 transformed = projection.transform(getLookatMatrix());
		//Perspective projection
		float aspectRatio = static_cast<float>(ScreenWidth / ScreenHeight); //window size
		float fovRad = 1.0f / tanf((fov * 0.5f) * (float)M_PI / 180.0f);
		Mat4 projectionMatrix = Mat4()._Identity();
		projectionMatrix.m[0][0] = aspectRatio * fovRad;
		projectionMatrix.m[1][1] = fovRad;
		projectionMatrix.m[2][2] = clipFar / (clipFar - clipNear);
		projectionMatrix.m[2][3] = (-clipFar * clipNear) / (clipFar - clipNear);
		projectionMatrix.m[3][2] = 1.0f;
		Vec4 projected = transformed.transform(projectionMatrix);
		//Normalize to camera space
		projected.v[0] /= projected.v[3];
		projected.v[1] /= projected.v[3];
		projected.v[2] /= projected.v[3];

		/*Vec4 distanceVec = projection - position;
		float distance = distanceVec.getLength();
		if (outcoming.normalize().Dot(distanceVec.normalize()) <= 0 || distance < clipNear || distance > clipFar)
			projected.v[3] = -1.0f; //not visible
		else
			projected.v[3] = remap(distance, clipNear, clipFar, 0.0f, 255.0f); //visible with depth info*/

		return projected;
	}

};