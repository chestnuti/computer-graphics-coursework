#include "./includes/Window.h"
#include "./includes/Core.h"
#include "./includes/Mesh.h"
#include "./includes/Buffer.h"
#include "./includes/Shader.h"
#include "./includes/Matrix.h"
#include "./includes/Camera.h"
#include "./includes/Image.h"
#include "./includes/Actor.h"
#include "./includes/EventBus.h"
#include "./includes/Hitbox.h"
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

	// Create event bus
	EventBus eventBus;

	// Create camera
	Camera camera;

	ShaderManager shaderManager;
	shaderManager.createShader(&core, "animatedShader", "./hlsl/AnimatedVS.hlsl", "./hlsl/BasicPS.hlsl");
	shaderManager.createShader(&core, "basicShader", "./hlsl/BasicVS.hlsl", "./hlsl/BasicPS.hlsl");
	shaderManager.createShader(&core, "skyboxShader", "./hlsl/Skybox.hlsl", "./hlsl/Skybox.hlsl");
	shaderManager.createShader(&core, "instancedShader", "./hlsl/InstancedVS.hlsl", "./hlsl/BasicPS.hlsl");
	
	// Create PSO manager
	PSOManager psos(&shaderManager);
	psos.createPSO(&core, "animatedPSO", "animatedShader", LayoutCache::getAnimatedLayout());
	psos.createPSO(&core, "basicPSO", "basicShader", LayoutCache::getStaticLayout());
	psos.createPSO(&core, "skyboxPSO", "skyboxShader", LayoutCache::getStaticLayout());
	psos.createPSO(&core, "instancedPSO", "instancedShader", LayoutCache::getInstancedLayout());

	// Load images
	ImageLoader imageLoader(&core);
	imageLoader.loadImage("Blank", "Models/Textures/Textures1_NH.png");
	imageLoader.loadImage("Sky", "Models/Textures/sky.png");
	imageLoader.loadImage("Ground", "Models/Textures/moss_groud_01_Base_Color_4k.png");
	imageLoader.loadImage("Ground_Normal", "Models/Textures/moss_groud_01_Normal_dx_4k.png");
	imageLoader.loadImage("ColorMap", "Models/LowPolyMilitary/Textures/Textures1_ALB.png");
	imageLoader.loadImage("AnimalsColorMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_alb.png");
	imageLoader.loadImage("AnimalsNormalMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_nh.png");

	// Create hitbox
	HitboxManager hitboxManager(&eventBus);

	// Load mesh
	Object player(&psos);
	player.loadGEM(&core, "Models/AnimatedLowPolyAnimals/Farmer-male.gem", "animatedPSO");
	player.setDiffuseTexture(imageLoader.getImage("AnimalsColorMap"));
	player.setNormalTexture(imageLoader.getImage("AnimalsNormalMap"));
	player.rotateBy(90.0f, X_AXIS);
	player.rotateBy(180.0f, Z_AXIS);
	player.scale = Vec3(0.05f, 0.05f, 0.05f);
	Player playerActor(&win);
	playerActor.init(&player);
	playerActor.bindCamera(&camera);
	hitboxManager.addHitbox(&playerActor, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.7f, 1.0f, 0.7f));
	playerActor.subscribeEvent(&eventBus);

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
	plane.init(&core, 15.0f);
	plane.psoNames = "basicPSO";
	plane.setDiffuseTexture(imageLoader.getImage("Ground"));
	plane.setNormalTexture(imageLoader.getImage("Ground_Normal"));

	Object otherObjects(&psos);
	otherObjects.meshes.push_back(&plane);
	otherObjects.meshes.push_back(&sphere);

	// NPCs
	ActorList henList;
	for (int i = 0; i < 4; i++) {
		Object* hen_brown = new Object(&psos);
		hen_brown->loadGEM(&core, "Models/AnimatedLowPolyAnimals/Hen-brown.gem", "animatedPSO");
		hen_brown->setDiffuseTexture(imageLoader.getImage("AnimalsColorMap"));
		hen_brown->setNormalTexture(imageLoader.getImage("AnimalsNormalMap"));
		hen_brown->rotateBy(180.0f, Y_AXIS);
		hen_brown->scale = Vec3(0.05f, 0.05f, 0.05f);
		Hen* henActor = new Hen();
		henActor->init(hen_brown);
		henActor->setPlayer(&playerActor);
		henActor->position = Vec3((float)(rand() % 1000) / 1000.0f * 10.0f, 0.0f, (float)(rand() % 1000) / 1000.0f * 10.0f);
		// subscribe to events
		henActor->subscribeEvent(&eventBus);
		// add hitbox
		hitboxManager.addHitbox(henActor, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.5f, 0.5f, 0.5f));
		henList.addActor(henActor);
	}

	
	std::vector<InstanceData> instanceDatas;
	for (int i = 0; i < 3000; i++) {
		InstanceData inst;
		float randX = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 30.0f;
		float randZ = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 30.0f;
		float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
		float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.01f + 0.005f;
		inst.World = Mat4().Translate(randX, 0, randZ) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
		inst.World = inst.World.Transpose();
		inst.Color = Vec4(1, 1, 1, 1);
		instanceDatas.push_back(inst);
	}

	InstancedObject instancedObject(&psos);
	instancedObject.init(&core, &grass, instanceDatas);
	
	// params
	float time = 0.0f;
	Mat4 VP = camera.getViewProjectionMatrix();
	Mat4 W2 = Mat4()._Identity();
	Mat4 skyboxBuffer_W;
	float animationTransition = 0.0f;
	Vec4 lightDirection = Vec4(-1.0f, -1.0f, 0.0f, 0.0f).normalize();

	// set constant buffer pointers
	//shaderManager.setConstantBufferValuePointer("animatedShader", "animatedMeshBuffer", "bones", playerActor.getBoneMatrices(), VERTEX_SHADER);
	//shaderManager.setConstantBufferValuePointer("animatedShader", "animatedMeshBuffer", "W", playerActor.getWorldMatrix(), VERTEX_SHADER);
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
		playerActor.update(dt);
		henList.update(dt);
		VP = camera.getViewProjectionMatrix();
		skyboxBuffer_W = Mat4().Translate(camera.position.v[0], camera.position.v[1], camera.position.v[2]) * Mat4().Scale(camera.clipFar - 1, camera.clipFar - 1, camera.clipFar - 1);

		// update hitboxes
		hitboxManager.update();

		// dispatch events
		eventBus.dispatch();


		core.beginRenderPass();


		// update instance buffer
		instancedObject.updateInstances(instanceDatas);

		// set samplers
		imageLoader.applySampler();

		// draw NPCs
		henList.draw(&core);

		// draw player
		playerActor.draw(&core);

		// draw instances
		instancedObject.drawInstanced(&core);

		// draw other objects
		otherObjects.draw(&core);
		
		core.finishFrame();


	}
	core.flushGraphicsQueue();
	return 0;
}
