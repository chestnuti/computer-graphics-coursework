#pragma once
#include "Core.h"
#include "Mesh.h"
#include "Buffer.h"
#include "Shader.h"
#include "Matrix.h"
#include "Camera.h"
#include "Image.h"
#include "Actor.h"
#include "EventBus.h"
#include "Hitbox.h"
#include "GamesEngineeringBase.h"
#include "GEMLoader.h"


class GameContext {
	// subsystems
	Core* core;
	Window* win;

	// parts
	GamesEngineeringBase::Timer timer = GamesEngineeringBase::Timer();
	EventBus eventBus = EventBus();
	Camera camera = Camera();
	ShaderManager shaderManager = ShaderManager();
	PSOManager psos = PSOManager(&shaderManager);
	ImageLoader imageLoader = ImageLoader(core);
	HitboxManager hitboxManager = HitboxManager(&eventBus);

public:
	// actors and objects
	Player* player;
	ActorList* actors;
	Object* skybox;
	std::vector<Object> worldObjects;
	std::unordered_map<std::string, InstancedObject> instancedObjects;

	// params
	Mat4 VP = camera.getViewProjectionMatrix();
	Mat4 W = Mat4()._Identity();
	Vec3 playerPos = Vec3(0.0f, 0.0f, 0.0f);
	Mat4 skyboxBuffer_W;
	float animationTransition = 0.0f;
	Vec4 lightDirection = Vec4(-1.0f, -1.0f, 0.0f, 0.0f).normalize();
	std::unordered_map<std::string, std::vector<InstanceData>> instanceDataMap;

	GameContext(Core* _core, Window* _win) : core(_core), win(_win) {}

	void init() {
		shaderManager.createShader(core, "animatedShader", "./hlsl/AnimatedVS.hlsl", "./hlsl/BasicPS.hlsl");
		shaderManager.createShader(core, "basicShader", "./hlsl/BasicVS.hlsl", "./hlsl/BasicPS.hlsl");
		shaderManager.createShader(core, "skyboxShader", "./hlsl/Skybox.hlsl", "./hlsl/Skybox.hlsl");
		shaderManager.createShader(core, "instancedShader", "./hlsl/InstancedVS.hlsl", "./hlsl/BasicPS.hlsl");

		// Create PSO manager
		psos.createPSO(core, "animatedPSO", "animatedShader", LayoutCache::getAnimatedLayout());
		psos.createPSO(core, "basicPSO", "basicShader", LayoutCache::getStaticLayout());
		psos.createPSO(core, "skyboxPSO", "skyboxShader", LayoutCache::getStaticLayout());
		psos.createPSO(core, "instancedPSO", "instancedShader", LayoutCache::getInstancedLayout());

		// Load images
		imageLoader.loadImage("Blank", "Models/Textures/Textures1_NH.png");
		imageLoader.loadImage("Sky", "Models/Textures/sky.png");
		imageLoader.loadImage("Ground", "Models/Textures/moss_groud_01_Base_Color_4k.png");
		imageLoader.loadImage("Ground_Normal", "Models/Textures/moss_groud_01_Normal_dx_4k.png");
		imageLoader.loadImage("ColorMap", "Models/LowPolyMilitary/Textures/Textures1_ALB.png");
		imageLoader.loadImage("AnimalsColorMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_alb.png");
		imageLoader.loadImage("AnimalsNormalMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_nh.png");
		
		// add boundary
		hitboxManager.addHitbox(nullptr, Vec3(60.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 60.0f));
		hitboxManager.addHitbox(nullptr, Vec3(-60.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 60.0f));
		hitboxManager.addHitbox(nullptr, Vec3(0.0f, 0.0f, 60.0f), Vec3(60.0f, 1.0f, 1.0f));
		hitboxManager.addHitbox(nullptr, Vec3(0.0f, 0.0f, -60.0f), Vec3(60.0f, 1.0f, 1.0f));

		// Load Player
		Object* player = new Object(&psos);
		player->loadGEM(core, "Models/AnimatedLowPolyAnimals/Farmer-male.gem", "animatedPSO");
		player->setDiffuseTexture(imageLoader.getImage("AnimalsColorMap"));
		player->setNormalTexture(imageLoader.getImage("AnimalsNormalMap"));
		player->rotateBy(90.0f, X_AXIS);
		player->rotateBy(180.0f, Z_AXIS);
		player->scale = Vec3(0.05f, 0.05f, 0.05f);
		this->player = new Player(win);
		this->player->init(player);
		this->player->bindCamera(&camera);
		hitboxManager.addHitbox(this->player, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.7f, 1.0f, 0.7f));
		this->player->subscribeEvent(&eventBus);

		// NPCs
		this->actors = new ActorList();
		for (int i = 0; i < 20; i++) {
			Object* hen_brown = new Object(&psos);
			hen_brown->loadGEM(core, "Models/AnimatedLowPolyAnimals/Hen-brown.gem", "animatedPSO");
			hen_brown->setDiffuseTexture(imageLoader.getImage("AnimalsColorMap"));
			hen_brown->setNormalTexture(imageLoader.getImage("AnimalsNormalMap"));
			hen_brown->rotateBy(180.0f, Y_AXIS);
			hen_brown->scale = Vec3(0.05f, 0.05f, 0.05f);
			Hen* henActor = new Hen();
			henActor->init(hen_brown);
			henActor->setPlayer(this->player);
			henActor->position = Vec3(((float)(rand() % 1000) / 1000.0f - 0.5) * 50.0f, 0.0f, ((float)(rand() % 1000) / 1000.0f - 0.5) * 50.0f);
			// subscribe to events
			henActor->subscribeEvent(&eventBus);
			// add hitbox
			hitboxManager.addHitbox(henActor, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.5f, 0.5f, 0.5f));
			this->actors->addActor(henActor);
		}

		// skybox and ground
		this->skybox = new Object(&psos);
		Sphere* sphere = new Sphere();
		sphere->init(core, 20, 20, 1.0f);
		sphere->psoNames = "skyboxPSO";
		sphere->setDiffuseTexture(imageLoader.getImage("Sky"));
		this->skybox->meshes.push_back(sphere);

		// ground
		Plane* plane = new Plane();
		plane->init(core, 20.0f);
		plane->psoNames = "instancedPSO";
		plane->setDiffuseTexture(imageLoader.getImage("Ground"));
		plane->setNormalTexture(imageLoader.getImage("Ground_Normal"));
		Object* ground = new Object(&psos);
		ground->meshes.push_back(plane);
		// create instance data of ground
		std::vector<InstanceData> groundInstanceData ;
		for (int i = -5; i < 5; i++) {
			for (int j = -5; j < 5; j++) {
				InstanceData inst;
				inst.World = Mat4().Translate(i * 40.0f, 0.0f, j * 40.0f);
				inst.World = inst.World.Transpose();
				groundInstanceData.push_back(inst);
			}
		}
		// create instanced object
		InstancedObject* instanced_ground = new InstancedObject(&psos);
		instanced_ground->init(core, ground, groundInstanceData);
		// add to world objects
		instanceDataMap.insert({ "ground", groundInstanceData });
		instancedObjects.insert({ "ground", *instanced_ground });

		// Load grass
		Object* grass003 = new Object(&psos);
		grass003->loadGEM(core, "Models/LowPolyMilitary/grass_003.gem", "instancedPSO");
		grass003->setDiffuseTexture(imageLoader.getImage("ColorMap"));
		// create instance data
		std::vector<InstanceData> instanceDatas;
		for (int i = 0; i < 3000; i++) {
			InstanceData inst;
			float randX = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 120.0f;
			float randZ = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 120.0f;
			float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
			float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.01f + 0.005f;
			inst.World = Mat4().Translate(randX, 0, randZ) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
			inst.World = inst.World.Transpose();
			instanceDatas.push_back(inst);
		}
		// create instanced object
		InstancedObject* instanced_grass003 = new InstancedObject(&psos);
		instanced_grass003->init(core, grass003, instanceDatas);
		// add to world objects
		instanceDataMap.insert({ "grass003", instanceDatas });
		instancedObjects.insert({ "grass003", *instanced_grass003 });

		//Load grass
		Object* grass007 = new Object(&psos);
		grass007->loadGEM(core, "Models/LowPolyMilitary/grass_007.gem", "instancedPSO");
		grass007->setDiffuseTexture(imageLoader.getImage("ColorMap"));
		// create instance data
		std::vector<InstanceData> instanceDatas2;
		for (int i = 0; i < 2000; i++) {
			InstanceData inst;
			float randX = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 120.0f;
			float randZ = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 120.0f;
			float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
			float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.01f + 0.005f;
			inst.World = Mat4().Translate(randX, 0, randZ) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
			inst.World = inst.World.Transpose();
			instanceDatas2.push_back(inst);
		}
		// create instanced object
		InstancedObject* instanced_grass007 = new InstancedObject(&psos);
		instanced_grass007->init(core, grass007, instanceDatas2);
		// add to world objects
		instanceDataMap.insert({ "grass007", instanceDatas2 });
		instancedObjects.insert({ "grass007", *instanced_grass007 });

		//Load grass
		Object* grass008 = new Object(&psos);
		grass008->loadGEM(core, "Models/LowPolyMilitary/grass_008.gem", "instancedPSO");
		grass008->setDiffuseTexture(imageLoader.getImage("ColorMap"));
		// create instance data
		std::vector<InstanceData> instanceDatas3;
		for (int i = 0; i < 1000; i++) {
			InstanceData inst;
			float randX = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 120.0f;
			float randZ = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 120.0f;
			float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
			float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.01f + 0.005f;
			inst.World = Mat4().Translate(randX, 0, randZ) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
			inst.World = inst.World.Transpose();
			instanceDatas3.push_back(inst);
		}
		// create instanced object
		InstancedObject* instanced_grass008 = new InstancedObject(&psos);
		instanced_grass008->init(core, grass008, instanceDatas3);
		// add to world objects
		instanceDataMap.insert({ "grass008", instanceDatas3 });
		instancedObjects.insert({ "grass008", *instanced_grass008 });

		// set constant buffer pointers
		shaderManager.setConstantBufferValuePointer("animatedShader", "animatedMeshBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("basicShader", "staticMeshBuffer", "W", &W, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("basicShader", "staticMeshBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("skyboxShader", "skyboxBuffer", "W", &skyboxBuffer_W, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("skyboxShader", "skyboxBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "W", &W, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "playerPosition", &playerPos, VERTEX_SHADER);

		shaderManager.setConstantBufferValuePointer("animatedShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
		shaderManager.setConstantBufferValuePointer("basicShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
	}

	void update() {
		static bool running = true;
		while (running) {
			if (!running) return;

			core->beginFrame();

			win->processMessages();
			if (win->keys[VK_ESCAPE]) running = false;

			// make lights orbit around center of screen
			float dt = timer.dt();

			// update parameters
			player->update(dt);
			actors->update(dt);
			VP = camera.getViewProjectionMatrix();
			skyboxBuffer_W = Mat4().Translate(camera.position.v[0], camera.position.v[1], camera.position.v[2]) * Mat4().Scale(camera.clipFar - 1, camera.clipFar - 1, camera.clipFar - 1);
			playerPos = player->position;
			std::vector<InstanceData> grass003_update = RotateInstanceObjectByPlayer("grass003");
			std::vector<InstanceData> grass007_update = RotateInstanceObjectByPlayer("grass007");
			std::vector<InstanceData> grass008_update = RotateInstanceObjectByPlayer("grass008");
			std::unordered_map<std::string, std::vector<InstanceData>> updatedInstanceDataMap = 
			{
				{"ground", instanceDataMap["ground"] },
				{"grass003", grass003_update },
				{"grass007", grass007_update },
				{"grass008", grass008_update }
			};

			// update hitboxes
			hitboxManager.update();

			// dispatch events
			eventBus.dispatch();


			core->beginRenderPass();


			// update instance buffer
			for (auto& [name, instancedObject] : instancedObjects) {
				instancedObject.updateInstances(updatedInstanceDataMap[name]);
			}

			// set samplers
			imageLoader.applySampler();

			// draw NPCs
			actors->draw(core);

			// draw player
			player->draw(core);

			// draw instances
			for (auto& [name, instancedObject] : instancedObjects) {
				instancedObject.drawInstanced(core);
			}

			// draw other objects
			for (int i = 0; i < worldObjects.size(); i++) {
				worldObjects[i].draw(core);
			}

			// draw skybox
			skybox->draw(core);

			core->finishFrame();
		}
	}

	std::vector<InstanceData> RotateInstanceObjectByPlayer(const std::string& objectName) {
		std::vector<InstanceData> updatedInstanceData;
		Vec3 playerPos = this->player->position;
		auto it = instanceDataMap.find(objectName);
		if (it != instanceDataMap.end()) {
			for (size_t i = 0; i < it->second.size(); i++) {
				Mat4 instanceWorld = it->second[i].World.Transpose();
				Vec3 instancePos = Vec3(instanceWorld.m[0][3], instanceWorld.m[1][3], instanceWorld.m[2][3]);
				Vec3 dir = playerPos - instancePos;
				float length = dir.getLength() + 0.3;
				length *= length;
				Vec3 axis = Vec3(0.0f, 1.0f, 0.0f).cross(dir).normalize();
				Mat4 rotationMat = Mat4().Rotate(20.0f * expf(-length / 5.0f), axis);
				Mat4 newWorld = Mat4().Translate(instancePos.v[0], instancePos.v[1], instancePos.v[2]) * rotationMat * Mat4().Scale(instanceWorld.m[0][0], instanceWorld.m[1][1], instanceWorld.m[2][2]);
				InstanceData newInst;
				newInst.World = newWorld.Transpose();
				updatedInstanceData.push_back(newInst);
			}
		}
		return updatedInstanceData;
	}

};