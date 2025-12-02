#include "./includes/Window.h"
#include "./includes/Core.h"
#include "./includes/Mesh.h"
#include "./includes/Buffer.h"
#include "./includes/Shader.h"
#include "./includes/Matrix.h"
#include "./includes/Camera.h"
#include "./includes/GamesEngineeringBase.h"

#include "./includes/GEMLoader.h"


struct alignas(16) StaticMeshBuffer {
	Mat4 W;
	Mat4 VP;
};

void DebugPrint(const std::string& message) {
	OutputDebugStringA((message + "\n").c_str());
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Create window and initialize core
	Window win;
	win.create(1920, 1080, "My Window");
	// Initialize core
	Core core;
	core.init(win.hwnd, win.width, win.height);
	GamesEngineeringBase::Timer timer;

	Cube sst;
	sst.init(&core);

	// Compile shaders
	Shader shader;
	shader.init("./hlsl/Shader4.hlsl", "./hlsl/Shader4.hlsl");
	
	// Create PSO manager
	PSOManager psos;
	psos.createPSO(&core, "sst", shader.vertexShader, shader.pixelShader, sst.vb.inputLayoutDesc);

	// Reflect shaders to get constant buffer info
	shader.reflect(&core, shader.vertexShader, shader.vsConstantBuffers);
	shader.reflect(&core, shader.pixelShader, shader.psConstantBuffers);
	
	// Create camera
	Camera camera;

	int width = win.width;
	int height = win.height;
	
	float time = 0.0f;
	Vec4 lights[4];
	StaticMeshBuffer meshBuffer;
	meshBuffer.W = Mat4()._Identity();
	meshBuffer.VP = camera.getViewProjectionMatrix().Transpose();
	

	while (true) {
		core.beginFrame();
		
		win.processMessages();
		if (win.keys[VK_ESCAPE] == 1) break;

		// make lights orbit around center of screen
		float dt = timer.dt();
		time += dt;
		/*for (int i = 0; i < 4; i++) {
			float angle = time + (i * M_PI / 2.0f);
			lights[i] = Vec4(width / 2.0f + (cosf(angle) * (width * 0.3f)),
				height / 2.0f + (sinf(angle) * (height * 0.3f)),
				0, 0);
		};
		// update constant buffer
		shader.psConstantBuffers[0].update("time", &time);
		shader.psConstantBuffers[0].update("lights", &lights);*/

		// camera controls
		static float speed = 10.0f;
		if (win.keys['W']) camera.position += camera.getForwardVector() * dt * speed;
		if (win.keys['S']) camera.position -= camera.getForwardVector() * dt * speed;
		if (win.keys['A']) camera.position -= camera.getRightVector() * dt * speed;
		if (win.keys['D']) camera.position += camera.getRightVector() * dt * speed;
		if (win.keys['Q']) camera.position -= camera.up * dt * speed;
		if (win.keys['E']) camera.position += camera.up * dt * speed;
		// mouse look
		POINT mousePos;
		GetCursorPos(&mousePos);
		ScreenToClient(win.hwnd, &mousePos);
		static bool firstMouse = true;
		if (win.mouseButtons[0])
		{
			firstMouse = false;
		}
		else
		{
			firstMouse = true;
		}
		static int lastX = win.width / 2;
		static int lastY = win.height / 2;
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
		camera.target = camera.position + front.normalize();

		
		// Debug output camera info
		/*std::stringstream mat;
		mat << "Cam Matrix:\n";
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				mat << meshBuffer.VP.m[i][j] << " ";
			}
			mat << "\n";
		}
		DebugPrint(mat.str());
		std::stringstream cam;
		cam << "Camera Position: " << camera.position.v[0] << ", " << camera.position.v[1] << ", " << camera.position.v[2] << "\n";
		cam << "Camera Forward: " << camera.getForwardVector().v[0] << ", " << camera.getForwardVector().v[1] << ", " << camera.getForwardVector().v[2] << "\n";
		cam << "Camera Right: " << camera.getRightVector().v[0] << ", " << camera.getRightVector().v[1] << ", " << camera.getRightVector().v[2] << "\n";
		cam << "Camera Up: " << camera.up.v[0] << ", " << camera.up.v[1] << ", " << camera.up.v[2] << "\n";
		DebugPrint(cam.str());*/

		// rotate cube over time
		meshBuffer.W = Mat4().RotateY(time * 50.0f).Transpose();
		meshBuffer.VP = camera.getViewProjectionMatrix().Transpose();

		// update constant buffer
		shader.vsConstantBuffers[0].update("W", &meshBuffer.W);
		shader.vsConstantBuffers[0].update("VP", &meshBuffer.VP);


		core.beginRenderPass();

		// apply shader
		shader.apply(&core);

		// bind pso
		psos.bind(&core, "sst");

		// draw triangle
		sst.vb.draw(&core);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
	return 0;
}



/*int main()
{
	/*
	//initialize a triangle
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
/*
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
}*/