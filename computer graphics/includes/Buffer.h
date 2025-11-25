#pragma once
#include "Mesh.h"
#include "Operators.h"


class Camera {
public:
	Vec4 position;
	Vec4 outcoming;
	Vec4 up;
	double fov;
	double clipNear;
	double clipFar;

	Camera() : position(0.0, 0.0, 50.0, 1.0), outcoming(0.0, 1.0, -1.0, 1.0), up(0.0, 0.0, 1.0, 1.0), fov(90.0), clipNear(0.1), clipFar(1000.0) {
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

		/*Vec4 distanceVec = projection - position;
		double distance = distanceVec.getLength();
		if (outcoming.normalize().Dot(distanceVec.normalize()) <= 0 || distance < clipNear || distance > clipFar)
			projected.v[3] = -1.0; //not visible
		else
			projected.v[3] = remap(distance, clipNear, clipFar, 0.0, 255.0); //visible with depth info*/

		return projected;
	}

};



class Buffer {
	Vec3* colorBuffer;
	double* depthBuffer;
	int width;
	int height;
public:
	Buffer(int pWidth, int pHeight) : width(pWidth), height(pHeight) {
		colorBuffer = new Vec3[width * height];
		depthBuffer = new double[width * height];
	}
	Vec3* getColorBuffer() { return colorBuffer; }
	double* getDepthBuffer() { return depthBuffer; }
	void clearColorBuffer(const Vec3& clearColor) {
		for (int i = 0; i < width * height; i++) {
			colorBuffer[i] = clearColor;
		}
	}
	void clearDepthBuffer() {
		for (int i = 0; i < width * height; i++) {
			depthBuffer[i] = 255;
		}
	}
	void clear(){
		clearColorBuffer(Vec3(0.0, 0.0, 0.0));
		clearDepthBuffer();
	}
	~Buffer() {
		delete[] colorBuffer;
		delete[] depthBuffer;
	}

	void drawIntoBuffer(const Triangle& triangle, Camera& camera) {
		Triangle tri_transformed;
		tri_transformed.v0 = camera.getViewProjectionVector(triangle.v0);
		tri_transformed.v1 = camera.getViewProjectionVector(triangle.v1);
		tri_transformed.v2 = camera.getViewProjectionVector(triangle.v2);
		tri_transformed.color0 = triangle.color0;
		tri_transformed.color1 = triangle.color1;
		tri_transformed.color2 = triangle.color2;
		//Convert to screen coordinates
		tri_transformed.v0.v[0] = (tri_transformed.v0.v[0] + 1.0) * 0.5 * ScreenWidth;
		tri_transformed.v0.v[1] = (1.0 - (tri_transformed.v0.v[1] + 1.0) * 0.5) * ScreenHeight;
		tri_transformed.v0.v[2] = 0;
		tri_transformed.v1.v[0] = (tri_transformed.v1.v[0] + 1.0) * 0.5 * ScreenWidth;
		tri_transformed.v1.v[1] = (1.0 - (tri_transformed.v1.v[1] + 1.0) * 0.5) * ScreenHeight;
		tri_transformed.v1.v[2] = 0;
		tri_transformed.v2.v[0] = (tri_transformed.v2.v[0] + 1.0) * 0.5 * ScreenWidth;
		tri_transformed.v2.v[1] = (1.0 - (tri_transformed.v2.v[1] + 1.0) * 0.5) * ScreenHeight;
		tri_transformed.v2.v[2] = 0;
		//Calculate distance
		double distance0 = remap((triangle.v0 - camera.position).getLength(), camera.clipNear, camera.clipFar, 0.0, 255.0)
			* (camera.outcoming.normalize().Dot((triangle.v0 - camera.position).normalize()) >= 0 ? 1 : -1);
		double distance1 = remap((triangle.v1 - camera.position).getLength(), camera.clipNear, camera.clipFar, 0.0, 255.0)
			* (camera.outcoming.normalize().Dot((triangle.v1 - camera.position).normalize()) >= 0 ? 1 : -1);
		double distance2 = remap((triangle.v2 - camera.position).getLength(), camera.clipNear, camera.clipFar, 0.0, 255.0)
			* (camera.outcoming.normalize().Dot((triangle.v2 - camera.position).normalize()) >= 0 ? 1 : -1);
		//Rasterization
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Vec4 point(static_cast<double>(x), static_cast<double>(y), 0.0, 1.0);
				Vec4 checkInfo = tri_transformed.checkPointInside(point);
				if (checkInfo.v[3] > 0) { //inside triangle (alpha, beta, gamma, inside)
					//correct depth value
					double depth = perspectiveCorrectInterpolateAttribute(distance0, distance1, distance2,
						tri_transformed.v0.v[3], tri_transformed.v1.v[3], tri_transformed.v2.v[3],
						checkInfo.v[0], checkInfo.v[1], checkInfo.v[2]);
					//Interpolate color
					int index = y * width + x;
					if (depth < depthBuffer[index] && depth >= 0) {
						depthBuffer[index] = depth;
						Vec3 colorDepth = perspectiveCorrectInterpolateAttribute(triangle.color0, triangle.color1, triangle.color2,
							tri_transformed.v0.v[3], tri_transformed.v1.v[3], tri_transformed.v2.v[3],
							checkInfo.v[0], checkInfo.v[1], checkInfo.v[2]);
						colorBuffer[index] = Vec3(colorDepth.v[0], colorDepth.v[1], colorDepth.v[2]);
					}
				}
			}
		}
	}
};