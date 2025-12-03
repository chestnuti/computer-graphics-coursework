#pragma once
#include "Vector.h"
#include "Window.h"

#define ScreenWidth 1920.0f
#define ScreenHeight 1080.0f


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
		float aspectRatio = ScreenWidth / ScreenHeight; //window size
		float fovRad = 1.0f / tanf((fov * 0.5f) * (float)M_PI / 180.0f);
		Mat4 projectionMatrix = Mat4()._Identity();
		projectionMatrix.m[0][0] = fovRad / aspectRatio;
		projectionMatrix.m[1][1] = fovRad;
		projectionMatrix.m[2][2] = clipFar / (clipFar - clipNear);
		projectionMatrix.m[2][3] = (-clipFar * clipNear) / (clipFar - clipNear);
		projectionMatrix.m[3][2] = 1.0f;
		projectionMatrix.m[3][3] = 0.0f;
		return projectionMatrix * getLookatMatrix();
	}

	Vec3 getForwardVector()
	{
		return (target - position).normalize();
	}

	Vec3 getRightVector()
	{
		Vec3 outcoming = getForwardVector();
		return up.cross(outcoming).normalize();
	}

	void control(Window* win, float dt) {
		static float speed = 10.0f;
		if (win->keys['W']) position += getForwardVector() * dt * speed;
		if (win->keys['S']) position -= getForwardVector() * dt * speed;
		if (win->keys['A']) position -= getRightVector() * dt * speed;
		if (win->keys['D']) position += getRightVector() * dt * speed;
		if (win->keys['Q']) position -= up * dt * speed;
		if (win->keys['E']) position += up * dt * speed;
		// mouse look
		POINT mousePos;
		GetCursorPos(&mousePos);
		ScreenToClient(win->hwnd, &mousePos);
		static bool firstMouse = true;
		if (win->mouseButtons[0])
		{
			firstMouse = false;
		}
		else
		{
			firstMouse = true;
		}
		static int lastX = win->width / 2;
		static int lastY = win->height / 2;
		if (firstMouse)
		{
			lastX = mousePos.x;
			lastY = mousePos.y;
			firstMouse = false;
		}
		int xoffset = mousePos.x - lastX;
		int yoffset = lastY - mousePos.y;
		lastX = mousePos.x;
		lastY = mousePos.y;
		float sensitivity = 0.1f;
		xoffset = static_cast<int>(xoffset * sensitivity);
		yoffset = static_cast<int>(yoffset * sensitivity);
		static float yaw = -90.0f;
		static float pitch = 0.0f;
		yaw += static_cast<float>(xoffset);
		pitch += static_cast<float>(yoffset);
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
		Vec3 front;
		front.v[0] = cosf(yaw * (float)M_PI / 180.0f) * cosf(pitch * (float)M_PI / 180.0f);
		front.v[1] = sinf(pitch * (float)M_PI / 180.0f);
		front.v[2] = sinf(yaw * (float)M_PI / 180.0f) * cosf(pitch * (float)M_PI / 180.0f);
		target = position + front.normalize();
	}

};