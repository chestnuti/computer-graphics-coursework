#include "./includes/Window.h"
#include "./includes/Core.h"
#include "./includes/Mesh.h"
#include "./includes/Buffer.h"
#include "./includes/GamesEngineeringBase.h"

#include "./includes/GEMLoader.h"



/*int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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

}*/


class Colour
{
	public:
	float r;
	float g;
	float b;
	Colour() : r(0), g(0), b(0) {}
	Colour(float red, float green, float blue) : r(red), g(green), b(blue) {}
	Colour operator*(const Colour& col) const
	{
		return Colour(r * col.r, g * col.g, b * col.b);
	}
	Colour operator*(const float val) const
	{
		return Colour(r * val, g * val, b * val);
	}
	Colour operator/(const float val) const
	{
		return Colour(r / val, g / val, b / val);
	}
};





int main()
{
	/*//initialize a triangle
	Triangle tri(
		Vec4(200.0f, 200.0f, 0.0f, 1.0f),
		Vec4(150.0f, 300.0f, 0.0f, 1.0f),
		Vec4(250.0f, 300.0f, 0.0f, 1.0f),
		Vec3(255.0f, 0.0f, 0.0f),
		Vec3(0.0f, 255.0f, 0.0f),
		Vec3(0.0f, 0.0f, 255.0f)
	);

	Triangle axisX(
		Vec4(0.0f, 0.0f, 0.0f, 1.0f),
		Vec4(100.0f, 0.0f, 0.0f, 1.0f),
		Vec4(100.0f, 10.0f, 0.0f, 1.0f),
		Vec3(255.0f, 0.0f, 0.0f),
		Vec3(255.0f, 0.0f, 0.0f),
		Vec3(255.0f, 0.0f, 0.0f)
	);
	Triangle axisY(
		Vec4(0.0f, 0.0f, 0.0f, 1.0f),
		Vec4(0.0f, 100.0f, 0.0f, 1.0f),
		Vec4(10.0f, 100.0f, 0.0f, 1.0f),
		Vec3(0.0f, 255.0f, 0.0f),
		Vec3(0.0f, 255.0f, 0.0f),
		Vec3(0.0f, 255.0f, 0.0f)
	);
	Triangle axisZ(
		Vec4(0.0f, 0.0f, 0.0f, 1.0f),
		Vec4(0.0f, 0.0f, 100.0f, 1.0f),
		Vec4(0.0f, 10.0f, 100.0f, 1.0f),
		Vec3(0.0f, 0.0f, 255.0f),
		Vec3(0.0f, 0.0f, 255.0f),
		Vec3(0.0f, 0.0f, 255.0f)
	);

	Triangle* Triangles[4];
	Triangles[0] = &tri;
	Triangles[1] = &axisX;
	Triangles[2] = &axisY;
	Triangles[3] = &axisZ;*/

	//GEM model loading test
	std::vector<GEMLoader::GEMMesh> gemmeshes;
	GEMLoader::GEMModelLoader loader;
	loader.load("./Resources Lecture 2/cube.gem", gemmeshes);
	std::vector<Vec3> vertexList;
	std::vector<Colour> colorList;
	for (int i = 0; i < gemmeshes.size(); i++) {
		for (int j = 0; j < gemmeshes[i].indices.size(); j++) {
			GEMLoader::GEMVec3 vec;
			int index = gemmeshes[i].indices[j];
			vec = gemmeshes[i].verticesStatic[index].position;
			vertexList.push_back(Vec3(vec.x, vec.y, vec.z));
			vec = gemmeshes[i].verticesStatic[index].normal;
			colorList.push_back(Colour(fabs(vec.x), fabs(vec.y), fabs(vec.z)));
		}
	}

	int triangleCount = vertexList.size() / 3;
	Triangle* Triangles = new Triangle[triangleCount];
	for (int i = 0; i < vertexList.size() / 3; i++) {
		Triangles[i] = Triangle(
			Vec4(vertexList[i * 3 + 0].v[0], vertexList[i * 3 + 0].v[1], vertexList[i * 3 + 0].v[2], 1.0f),
			Vec4(vertexList[i * 3 + 1].v[0], vertexList[i * 3 + 1].v[1], vertexList[i * 3 + 1].v[2], 1.0f),
			Vec4(vertexList[i * 3 + 2].v[0], vertexList[i * 3 + 2].v[1], vertexList[i * 3 + 2].v[2], 1.0f),
			Vec3(colorList[i * 3 + 0].r * 255.0f, colorList[i * 3 + 0].g * 255.0f, colorList[i * 3 + 0].b * 255.0f),
			Vec3(colorList[i * 3 + 1].r * 255.0f, colorList[i * 3 + 1].g * 255.0f, colorList[i * 3 + 1].b * 255.0f),
			Vec3(colorList[i * 3 + 2].r * 255.0f, colorList[i * 3 + 2].g * 255.0f, colorList[i * 3 + 2].b * 255.0f)
		);
	}


	Camera camera;
	Buffer buffer(ScreenWidth, ScreenHeight);

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
				float yawAngle = deltaX * 0.1f; //sensitivity
				Mat4 yawRotation = Mat4().RotateZ(yawAngle);
				camera.outcoming = camera.outcoming.transform(yawRotation).normalize();
				//Pitch rotation
				float pitchAngle = deltaY * 0.1f; //sensitivity
				Mat4 pitchRotation = Mat4().RotateX(pitchAngle);
				camera.outcoming = (camera.outcoming + Vec4(0.0f, 1.0f, 0.0f, 1.0f).transform(pitchRotation) - Vec4(0.0f, 1.0f, 0.0f, 1.0f)).normalize();
			}
		}

		//move camera
		float dealtaTime = timer.dt();
		float cameraSpeed = 30.0f * dealtaTime;
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

		//cout << "Camera Position: (" << camera.position.v[0] << ", " << camera.position.v[1] << ", " << camera.position.v[2] << ")     ";
		//cout << "Camera Outcoming: (" << camera.outcoming.v[0] << ", " << camera.outcoming.v[1] << ", " << camera.outcoming.v[2] << ")     ";
		cout << "FPS: " << 1.0f / dealtaTime << "\r";

		buffer.clear();
		//transform triangles to screen space
		for (int i = 0; i < triangleCount; i++)
		{
			buffer.drawIntoBuffer(Triangles[i], camera);
		}

		//check points inside the triangle and draw them
		for (int y = 0; y < ScreenHeight; y++)
		{
			for (int x = 0; x < ScreenWidth; x++)
			{
				//canvas.draw(x, y, static_cast<int>(buffer.getDepthBuffer()[y * ScreenWidth + x]), static_cast<int>(buffer.getDepthBuffer()[y * ScreenWidth + x]), buffer.getDepthBuffer()[y * ScreenWidth + x]);
				canvas.draw(x, y, buffer.getColorBuffer()[y * ScreenWidth + x].v[0], buffer.getColorBuffer()[y * ScreenWidth + x].v[1], buffer.getColorBuffer()[y * ScreenWidth + x].v[2]);
			}
		}


		canvas.present();
	}


	return 0;
}