#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include "./includes/GamesEngineeringBase.h"
#include "./includes/Vector.h"
#include "./includes/Matrix.h"
using namespace std;

#define ScreenWidth 960
#define ScreenHeight 540

double clamp(double val, double minVal, double maxVal)
{
	return max(minVal, min(maxVal, val));
}

double remap(double val, double inMin, double inMax, double outMin, double outMax)
{
	return outMin + (outMax - outMin) * ((val - inMin) / (inMax - inMin));
}



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
		double aspectRatio = ScreenWidth / ScreenHeight; //window size
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



int main()
{
	//initialize a triangle
	Triangle tri(
		Vec4(200.0, 200.0, 0.0, 1.0),
		Vec4(150.0, 300.0, 0.0, 1.0),
		Vec4(250.0, 300.0, 0.0, 1.0),
		Vec3(255.0, 0.0, 0.0),
		Vec3(0.0, 255.0, 0.0),
		Vec3(0.0, 0.0, 255.0)
	);

	Triangle axisX(
		Vec4(0.0, 0.0, 0.0, 1.0),
		Vec4(100.0, 0.0, 0.0, 1.0),
		Vec4(100.0, 10.0, 0.0, 1.0),
		Vec3(255.0, 0.0, 0.0),
		Vec3(255.0, 0.0, 0.0),
		Vec3(255.0, 0.0, 0.0)
	);
	Triangle axisY(
		Vec4(0.0, 0.0, 0.0, 1.0),
		Vec4(0.0, 100.0, 0.0, 1.0),
		Vec4(10.0, 100.0, 0.0, 1.0),
		Vec3(0.0, 255.0, 0.0),
		Vec3(0.0, 255.0, 0.0),
		Vec3(0.0, 255.0, 0.0)
	);
	Triangle axisZ(
		Vec4(0.0, 0.0, 0.0, 1.0),
		Vec4(0.0, 0.0, 100.0, 1.0),
		Vec4(0.0, 10.0, 100.0, 1.0),
		Vec3(0.0, 0.0, 255.0),
		Vec3(0.0, 0.0, 255.0),
		Vec3(0.0, 0.0, 255.0)
	);

	Triangle* Triangles[4];
	Triangles[0] = &tri;
	Triangles[1] = &axisX;
	Triangles[2] = &axisY;
	Triangles[3] = &axisZ;
	Triangle* tri_transformed[4];
	Camera camera;

	int mouseX_prev = 0;
	int mouseY_prev = 0;

	//initialize a window
	GamesEngineeringBase::Window canvas;
	canvas.create(ScreenWidth, ScreenHeight, "window");
	GamesEngineeringBase::Timer timer;

	//main loop
	while (true) {
		canvas.clear();
		canvas.checkInput();
		if(canvas.keyPressed(VK_ESCAPE))
			break;

		int mouseX = canvas.getMouseX();
		int mouseY = canvas.getMouseY();

		int deltaX = mouseX - mouseX_prev;
		int deltaY = mouseY - mouseY_prev;
		mouseX_prev = mouseX;
		mouseY_prev = mouseY;
		//update camera orientation based on mouse movement
		if (canvas.mouseButtonPressed(GamesEngineeringBase::MouseLeft))
		{
			if (deltaX != 0 || deltaY != 0)
			{
				//Yaw rotation
				double yawAngle = deltaX * 0.1; //sensitivity
				Mat4 yawRotation = Mat4().RotateZ(yawAngle);
				camera.outcoming = camera.outcoming.transform(yawRotation).normalize();
				//Pitch rotation
				double pitchAngle = deltaY * 0.1; //sensitivity
				Mat4 pitchRotation = Mat4().RotateX(pitchAngle);
				camera.outcoming = (camera.outcoming + Vec4(0.0, 1.0, 0.0, 1.0).transform(pitchRotation) - Vec4(0.0, 1.0, 0.0, 1.0)).normalize();
			}
		}

		//move camera
		float dealtaTime = timer.dt();
		double cameraSpeed = 500.0 * dealtaTime;
		Vec4 tangent = camera.up.cross(camera.outcoming).normalize();
		if (canvas.keyPressed('W'))
		{
			camera.position += camera.outcoming * cameraSpeed;
		}
		if (canvas.keyPressed('S'))
		{
			camera.position -= camera.outcoming * cameraSpeed;
		}
		if (canvas.keyPressed('A'))
		{
			camera.position -= tangent * cameraSpeed;
		}
		if (canvas.keyPressed('D'))
		{
			camera.position += tangent * cameraSpeed;
		}
		if (canvas.keyPressed('E'))
		{
			camera.position += camera.up * cameraSpeed;
		}
		if (canvas.keyPressed('Q'))
		{
			camera.position -= camera.up * cameraSpeed;
		}

		cout << "Camera Position: (" << camera.position.v[0] << ", " << camera.position.v[1] << ", " << camera.position.v[2] << ")     ";
		cout << "Camera Outcoming: (" << camera.outcoming.v[0] << ", " << camera.outcoming.v[1] << ", " << camera.outcoming.v[2] << ")     ";
		cout << "FPS: " << 1.0 / dealtaTime << "\r";

		//transform triangle vertices to camera space
		for (int i = 0; i < 4; i++)
		{
			tri_transformed[i] = new Triangle();
			tri_transformed[i]->v0 = camera.getViewProjectionVector(Triangles[i]->v0);
			tri_transformed[i]->v1 = camera.getViewProjectionVector(Triangles[i]->v1);
			tri_transformed[i]->v2 = camera.getViewProjectionVector(Triangles[i]->v2);
			tri_transformed[i]->color0 = Triangles[i]->color0;
			tri_transformed[i]->color1 = Triangles[i]->color1;
			tri_transformed[i]->color2 = Triangles[i]->color2;
		}

		//check points inside the triangle and draw them
		for (int y = 0; y < ScreenHeight; y++)
		{
			for (int x = 0; x < ScreenWidth; x++)
			{
				Vec4 point((double)x, (double)y, 0.0, 1.0);
				for (int i = 0; i < 4; i++)
				{
					if(tri_transformed[i]->v0.v[3] == 0.0 || tri_transformed[i]->v1.v[3] == 0.0 || tri_transformed[i]->v2.v[3] == 0.0)
						continue; //triangle not visible
					Vec4 color = tri_transformed[i]->checkPointInside(point);
					if (color.v[3]){
						canvas.draw(x, y, color.v[0], color.v[1], color.v[2]); //draw point with interpolated color
					}
				}
			}
		}


		canvas.present();
	}


	return 0;
}