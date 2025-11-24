#include "./includes/Window.h"
#include "./includes/Core.h"
#include "./includes/Mesh.h"



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	Window win;
	win.create(1024, 1024, "My Window");
	Core core;
	core.init(win.hwnd, win.width, win.height);
	while (true) {
		core.resetCommandList();
		core.beginFrame();
		win.processMessages();
		if (win.keys[VK_ESCAPE] == 1)
		{
			break;
		}
		core.finishFrame();
	}
	core.flushGraphicsQueue();

}




/*int main()
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
}*/