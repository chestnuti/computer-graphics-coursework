#include "./includes/Window.h"
#include "./includes/Core.h"
#include "./includes/Mesh.h"
#include "./includes/Buffer.h"
#include "./includes/Shader.h"
#include "./includes/Matrix.h"
#include "./includes/Camera.h"
#include "./includes/Image.h"
#include "./includes/Actor.h"
#include "./includes/GamesEngineeringBase.h"
#include "./includes/GEMLoader.h"



int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Create window and initialize core
	Window win;
	win.create(ScreenWidth, ScreenHeight, "My Window");
	// Initialize core
	Core core;
	core.init(win.hwnd, win.width, win.height);
	GamesEngineeringBase::Timer timer;

	// Create camera
	Camera camera;

	ShaderManager shaderManager;
	shaderManager.createShader(&core, "animatedShader", "./hlsl/AnimatedVS.hlsl", "./hlsl/BasicPS.hlsl");
	shaderManager.createShader(&core, "basicShader", "./hlsl/BasicVS.hlsl", "./hlsl/BasicPS.hlsl");
	shaderManager.createShader(&core, "skyboxShader", "./hlsl/Skybox.hlsl", "./hlsl/Skybox.hlsl");
	shaderManager.createShader(&core, "instancedShader", "./hlsl/InstancedVS.hlsl", "./hlsl/BasicPS.hlsl");
	
	// Create PSO manager
	PSOManager psos;
	psos.createPSO(&core, "animatedPSO", shaderManager.shaders["animatedShader"], LayoutCache::getAnimatedLayout());
	psos.createPSO(&core, "basicPSO", shaderManager.shaders["basicShader"], LayoutCache::getStaticLayout());
	psos.createPSO(&core, "skyboxPSO", shaderManager.shaders["skyboxShader"], LayoutCache::getStaticLayout());
	psos.createPSO(&core, "instancedPSO", shaderManager.shaders["instancedShader"], LayoutCache::getInstancedLayout());

	// Load images
	ImageLoader imageLoader(&core);
	imageLoader.loadImage("Blank", "Models/Textures/Textures1_NH.png");
	//imageLoader.loadImage("Trex", "Models/Trex/Textures/T-rex_Base_Color_ALB.png");
	//imageLoader.loadImage("TrexNormal", "Models/Trex/Textures/T-rex_Base_Color_NH.png");
	imageLoader.loadImage("Sky", "Models/Textures/sky.png");
	imageLoader.loadImage("Ground", "Models/Textures/aerial_rocks_04_diff_4k.png");
	imageLoader.loadImage("Ground_Normal", "Models/Textures/aerial_rocks_04_nor_dx_4k.png");
	imageLoader.loadImage("ColorMap", "Models/LowPolyMilitary/Textures/Textures1_ALB.png");
	imageLoader.loadImage("AnimalsColorMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_alb.png");
	imageLoader.loadImage("AnimalsNormalMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_nh.png");

	// Load mesh
	/*Object trex(&psos);
	trex.loadGEM(&core, "Models/Trex/TRex.gem", "animatedPSO");
	trex.setDiffuseTexture(imageLoader.getImage("Trex"));
	trex.setNormalTexture(imageLoader.getImage("TrexNormal"));*/

	Object player(&psos);
	player.loadGEM(&core, "Models/AnimatedLowPolyAnimals/Farmer-male.gem", "animatedPSO");
	player.setDiffuseTexture(imageLoader.getImage("AnimalsColorMap"));
	//player.setNormalTexture(imageLoader.getImage("AnimalsNormalMap"));
	player.rotateBy(90.0f, X_AXIS);
	player.rotateBy(180.0f, Z_AXIS);
	player.scale = Vec3(0.05f, 0.05f, 0.05f);
	Player playerActor(&win);
	playerActor.init(&player);
	playerActor.bindCamera(&camera);

	// Load grass
	Object grass(&psos);
	grass.loadGEM(&core, "Models/LowPolyMilitary/grass_003.gem", "instancedPSO");
	grass.setDiffuseTexture(imageLoader.getImage("ColorMap"));

	// skybox and ground
	Sphere sphere;
	sphere.init(&core, 20, 20, 1.0f);
	sphere.psoNames = "skyboxPSO";
	sphere.setDiffuseTexture(imageLoader.getImage("Sky"));

	Plane plane;
	plane.init(&core, 30.0f);
	plane.psoNames = "basicPSO";
	plane.setDiffuseTexture(imageLoader.getImage("Ground"));
	plane.setNormalTexture(imageLoader.getImage("Ground_Normal"));

	Object otherObjects(&psos);
	otherObjects.meshes.push_back(&plane);
	otherObjects.meshes.push_back(&sphere);


	// Create cube instances
	Cube cube;
	cube.init(&core, 1.0f);

	std::vector<InstanceData> instanceDatas;
	for (int i = 0; i < 3000; i++) {
		InstanceData inst;
		float randX = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 60.0f;
		float randZ = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 60.0f;
		float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.01f + 0.005f;
		inst.World = Mat4().Translate(randX, 0, randZ) * Mat4().Scale(randScale, randScale, randScale);
		inst.World = inst.World.Transpose();
		inst.Color = Vec4(1, 1, 1, 1);
		instanceDatas.push_back(inst);
	}

	InstancedObject instancedObject(&psos);
	instancedObject.init(&core, &grass, instanceDatas);


	// Create secquencer
	/*Sequencer sequencer;
	sequencer.addItem(&trex.animation, "Run", 0.0f, 0.0f, 1.0f);
	sequencer.addItem(&trex.animation, "walk", 0.0f, 0.0f, 1.0f);
	sequencer.addItem(&trex.animation, "Idle", 0.0f, 0.0f, 1.0f);
	sequencer.addItem(&trex.animation, "attack", 0.0f, 0.0f, 1.0f);*/
	
	// params
	float time = 0.0f;
	Mat4 VP = camera.getViewProjectionMatrix();
	Mat4 W2 = Mat4()._Identity();
	Mat4 skyboxBuffer_W;
	float animationTransition = 0.0f;
	Vec4 lightDirection = Vec4(0.0f, -1.0f, -1.0f, 0.0f).normalize();

	// set constant buffer pointers
	shaderManager.setConstantBufferValuePointer("animatedShader", "animatedMeshBuffer", "bones", playerActor.getBoneMatrices(), VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("animatedShader", "animatedMeshBuffer", "W", playerActor.getWorldMatrix(), VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("animatedShader", "animatedMeshBuffer", "VP", &VP, VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("basicShader", "staticMeshBuffer", "W", &W2, VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("basicShader", "staticMeshBuffer", "VP", &VP, VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("skyboxShader", "skyboxBuffer", "W", &skyboxBuffer_W, VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("skyboxShader", "skyboxBuffer", "VP", &VP, VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "W", &W2, VERTEX_SHADER);
	shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "VP", &VP, VERTEX_SHADER);

	shaderManager.setConstantBufferValuePointer("animatedShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
	shaderManager.setConstantBufferValuePointer("basicShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
	shaderManager.setConstantBufferValuePointer("instancedShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);

	while (true) {
		core.beginFrame();
		
		win.processMessages();
		if (win.keys[VK_ESCAPE] == 1) break;

		// make lights orbit around center of screen
		float dt = timer.dt();
		time += dt;

		// update parameters
		//player.position.v[2] += 0.05f * remap_clamp(10.0f - time, 0.0f, 10.0f, 0.0f, 1.0f);
		//player.updateWorldMatrix();
		playerActor.update(dt);
		VP = camera.getViewProjectionMatrix();
		skyboxBuffer_W = Mat4().Translate(camera.position.v[0], camera.position.v[1], camera.position.v[2]) * Mat4().Scale(camera.clipFar - 1, camera.clipFar - 1, camera.clipFar - 1);


		// update shader constant buffers
		shaderManager.updateAllConstantBuffers();

		// update instance buffer
		instancedObject.updateInstances(instanceDatas);

		core.beginRenderPass();

		imageLoader.applySampler();

		// draw models
		player.draw(&core);

		// draw instances
		instancedObject.drawInstanced(&core);

		bool useTex = false;
		shaderManager.shaders["basicShader"]->updateConstantBuffer("basicPSBuffer", "useNormalMap", &useTex, PIXEL_SHADER);

		// draw other objects
		otherObjects.draw(&core);
		
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