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
#include "UI.h"
#include "GamesEngineeringBase.h"
#include "GEMLoader.h"
#include <direct.h>

#define HEN_BROWN "Models/AnimatedLowPolyAnimals/Hen-brown.gem"
#define HEN_WHITE "Models/AnimatedLowPolyAnimals/Hen-white.gem"
#define ROOSTER_DARK "Models/AnimatedLowPolyAnimals/Rooster-dark.gem"
#define ROOSTER_BROWN "Models/AnimatedLowPolyAnimals/Rooster-brown.gem"

#define SAVE_DIR "Levels/"



class LevelManager {
public:
	LevelManager() {}

	static bool WriteToFile(const std::string& filename, int scoreToWin, const std::vector<Actor>& actors)
	{
		if (actors.size() != scoreToWin) {
			throw "Actor count does not match scoreToWin\n";
			return false;
		}

		std::string savePath = SAVE_DIR + filename;

		createFileIfNotExists(filename, scoreToWin, actors);

		std::ofstream file(savePath);
		if (!file.is_open()) {
			throw "Failed to open file for writing\n";
			return false;
		}

		// write scoreToWin
		file << scoreToWin << "\n";

		// write actor data
		for (const auto& actor : actors) {
			file << actor.type << "\n";

			file << actor.position.v[0] << " "
				<< actor.position.v[1] << " "
				<< actor.position.v[2] << "\n";

			file << actor.rotation.v[0] << " "
				<< actor.rotation.v[1] << " "
				<< actor.rotation.v[2] << " "
				<< actor.rotation.v[3] << "\n";

			file << actor.scale.v[0] << " "
				<< actor.scale.v[1] << " "
				<< actor.scale.v[2] << "\n";
		}

		file.close();
		return true;
	}


	template<typename AT>
	static std::vector<AT*> ReadFromFile(const std::string& filename, int& scoreToWin)
	{
		std::vector<AT*> actors;

		std::string loadPath = SAVE_DIR + filename;

		std::ifstream file(loadPath);
		if (!file.is_open()) {
			throw "Failed to open file for reading\n";
			return actors;
		}

		// read scoreToWin
		file >> scoreToWin;

		actors.clear();
		actors.reserve(scoreToWin);

		for (int i = 0; i < scoreToWin; ++i) {
			AT* actor = new AT();

			file >> actor->type;

			file >> actor->position.v[0]
				>> actor->position.v[1]
				>> actor->position.v[2];

			file >> actor->rotation.v[0]
				>> actor->rotation.v[1]
				>> actor->rotation.v[2]
				>> actor->rotation.v[3];

			file >> actor->scale.v[0]
				>> actor->scale.v[1]
				>> actor->scale.v[2];

			actors.push_back(actor);
		}

		file.close();
		return actors;
	}

	static bool checkFileExists(const std::string& filename) {
		std::string filepath = SAVE_DIR + filename;
		std::ifstream file(filepath);
		return file.good();
	}

	static bool createFileIfNotExists(const std::string& filename, int scoreToWin, const std::vector<Actor>& defaultActors) {
		std::string filepath = SAVE_DIR + filename;
		if (!checkFileExists(filepath)) {
			if (_mkdir(filepath.c_str()) == -1 && errno != EEXIST) {
				throw "Failed to create directory\n";
				return false;
			}
			return true;
		}
		return true;
	}
};




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
	UIManager uiManager = UIManager(&psos);
	LevelManager levelManager = LevelManager();

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
	float time = 0.0f;

	// game context methods
	int gameState = 0; // 0 - playing, 1 - win, 2 - lose
	int score = 0;
	int scoreToWin = 20;
	int timeLimit = 300; // seconds

	GameContext(Core* _core, Window* _win) : core(_core), win(_win) {}

	void init() {
		// Subscribe event handlers
		subscribeEventHandlers();

		// Create shaders
		shaderManager.createShader(core, "animatedShader", "./hlsl/AnimatedVS.hlsl", "./hlsl/BasicPS.hlsl");
		shaderManager.createShader(core, "basicShader", "./hlsl/BasicVS.hlsl", "./hlsl/BasicPS.hlsl");
		shaderManager.createShader(core, "skyboxShader", "./hlsl/Skybox.hlsl", "./hlsl/Skybox.hlsl");
		shaderManager.createShader(core, "instancedShader", "./hlsl/InstancedVS.hlsl", "./hlsl/BasicPS.hlsl");
		shaderManager.createShader(core, "instancedStaticShader", "./hlsl/InstancedStaticVS.hlsl", "./hlsl/BasicPS.hlsl");
		shaderManager.createShader(core, "uiShader", "./hlsl/UI.hlsl", "./hlsl/UI.hlsl");

		// Create PSO manager
		psos.createPSO(core, "animatedPSO", "animatedShader", LayoutCache::getAnimatedLayout());
		psos.createPSO(core, "basicPSO", "basicShader", LayoutCache::getStaticLayout());
		psos.createPSO(core, "skyboxPSO", "skyboxShader", LayoutCache::getStaticLayout());
		psos.createPSO(core, "instancedPSO", "instancedShader", LayoutCache::getInstancedLayout());
		psos.createPSO(core, "instancedStaticPSO", "instancedStaticShader", LayoutCache::getInstancedLayout());
		psos.createPSO(core, "uiPSO", "uiShader", LayoutCache::getUILayout());

		// Load images
		imageLoader.loadImage("Blank", "Models/Textures/Textures1_NH.png");
		imageLoader.loadImage("Sky", "Models/Textures/sky.png");
		imageLoader.loadImage("Ground", "Models/Textures/moss_groud_01_Base_Color_4k.png");
		imageLoader.loadImage("Ground_Normal", "Models/Textures/moss_groud_01_Normal_dx_4k.png");
		imageLoader.loadImage("ColorMap", "Models/LowPolyMilitary/Textures/Textures1_ALB.png");
		imageLoader.loadImage("AnimalsColorMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_alb.png");
		imageLoader.loadImage("AnimalsNormalMap", "Models/AnimatedLowPolyAnimals/Textures/T_Animalstextures_nh.png");
		imageLoader.loadImage("Bamboo", "Models/TreeModels/Textures/bamboo branch_ALB.png");
		imageLoader.loadImage("Bamboo_Normal", "Models/TreeModels/Textures/bamboo branch_NH.png");
		imageLoader.loadImage("Bamboo_branch", "Models/TreeModels/Textures/plant02_ALB.png");
		imageLoader.loadImage("Bamboo_branch_Normal", "Models/TreeModels/Textures/plant02_NH.png");
		// UI Images
		imageLoader.loadImage("UI_Time", "UI/Time.png");
		imageLoader.loadImage("UI_Score", "UI/Score.png");
		imageLoader.loadImage("Number_0", "UI/Numbers/0.png");
		imageLoader.loadImage("Number_1", "UI/Numbers/1.png");
		imageLoader.loadImage("Number_2", "UI/Numbers/2.png");
		imageLoader.loadImage("Number_3", "UI/Numbers/3.png");
		imageLoader.loadImage("Number_4", "UI/Numbers/4.png");
		imageLoader.loadImage("Number_5", "UI/Numbers/5.png");
		imageLoader.loadImage("Number_6", "UI/Numbers/6.png");
		imageLoader.loadImage("Number_7", "UI/Numbers/7.png");
		imageLoader.loadImage("Number_8", "UI/Numbers/8.png");
		imageLoader.loadImage("Number_9", "UI/Numbers/9.png");
		imageLoader.loadImage("UI_bar", "UI/Bar.png");


		// add boundary
		hitboxManager.addHitbox(nullptr, Vec3(60.0f, 0.0f, 0.0f), Vec3(2.0f, 5.0f, 60.0f));
		hitboxManager.addHitbox(nullptr, Vec3(-60.0f, 0.0f, 0.0f), Vec3(2.0f, 5.0f, 60.0f));
		hitboxManager.addHitbox(nullptr, Vec3(0.0f, 0.0f, 60.0f), Vec3(60.0f, 5.0f, 2.0f));
		hitboxManager.addHitbox(nullptr, Vec3(0.0f, 0.0f, -60.0f), Vec3(60.0f, 5.0f, 2.0f));
		hitboxManager.addHitbox(nullptr, Vec3(60.0f, 0.0f, 60.0f), Vec3(2.0f, 5.0f, 2.0f));
		hitboxManager.addHitbox(nullptr, Vec3(-60.0f, 0.0f, 60.0f), Vec3(2.0f, 5.0f, 2.0f));
		hitboxManager.addHitbox(nullptr, Vec3(60.0f, 0.0f, -60.0f), Vec3(2.0f, 5.0f, 2.0f));
		hitboxManager.addHitbox(nullptr, Vec3(-60.0f, 0.0f, -60.0f), Vec3(2.0f, 5.0f, 2.0f));

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

		// Hens
		this->actors = new ActorList();
		for (int i = 0; i < scoreToWin; i++) {
			Object* hen = new Object(&psos);
			// randomly select hen model
			int r = rand() % 4;
			switch (r) {
			case 0:
				hen->loadGEM(core, HEN_BROWN, "animatedPSO");
				break;
			case 1:
				hen->loadGEM(core, HEN_WHITE, "animatedPSO");
				break;
			case 2:
				hen->loadGEM(core, ROOSTER_DARK, "animatedPSO");
				break;
			case 3:
				hen->loadGEM(core, ROOSTER_BROWN, "animatedPSO");
				break;
			}
			hen->setDiffuseTexture(imageLoader.getImage("AnimalsColorMap"));
			hen->setNormalTexture(imageLoader.getImage("AnimalsNormalMap"));
			hen->rotateBy(180.0f, Y_AXIS);
			hen->scale = Vec3(0.05f, 0.05f, 0.05f);
			Hen* henActor = new Hen();
			henActor->init(hen);
			henActor->setPlayer(this->player);
			henActor->type = r;	// save hen type
			henActor->position = Vec3(((float)(rand() % 1000) / 1000.0f - 0.5) * 100.0f, 0.0f, ((float)(rand() % 1000) / 1000.0f - 0.5) * 100.0f);
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

		// buliding
		Object* building = new Object(&psos);
		building->loadGEM(core, "Models/LowPolyMilitary/building_001.gem", "basicPSO");
		building->setDiffuseTexture(imageLoader.getImage("ColorMap"));
		building->position = Vec3(0.0f, -2.1f, 0.0f);
		building->scale = Vec3(0.02f, 0.03f, 0.03f);
		worldObjects.push_back(*building);
		// bulidng hitbox
		hitboxManager.addHitbox(nullptr, Vec3(17.5f, 0.0f, -5.9f), Vec3(0.2f, 5.0f, 6.6f));
		hitboxManager.addHitbox(nullptr, Vec3(-17.5f, 0.0f, -5.9f), Vec3(0.2f, 5.0f, 6.6f));
		hitboxManager.addHitbox(nullptr, Vec3(0.0f, 0.0f, -12.5f), Vec3(17.5f, 5.0f, 0.2f));
		hitboxManager.addHitbox(nullptr, Vec3(0.0f, 0.0f, 7.7f), Vec3(12.3f, 5.0f, 0.2f));
		hitboxManager.addHitbox(nullptr, Vec3(-12.3f, 0.0f, 4.2f), Vec3(0.2f, 5.0f, 3.5f));
		hitboxManager.addHitbox(nullptr, Vec3(12.3f, 0.0f, 4.2f), Vec3(0.2f, 5.0f, 3.5f));
		hitboxManager.addHitbox(nullptr, Vec3(0.0f, 0.0f, -11.0f), Vec3(12.0f, 2.5f, 1.5f));

		// ground
		Plane* plane = new Plane();
		plane->init(core, 20.0f);
		plane->psoNames = "instancedStaticPSO";
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
		for (int i = 0; i < 6000; i++) {
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

		// Load Bamboo
		Object* bamboo = new Object(&psos);
		bamboo->loadGEM(core, "Models/TreeModels/bamboo.gem", "instancedPSO");
		bamboo->meshes[1]->setDiffuseTexture(imageLoader.getImage("Bamboo"));
		bamboo->meshes[1]->setNormalTexture(imageLoader.getImage("Bamboo_Normal"));
		bamboo->meshes[0]->setDiffuseTexture(imageLoader.getImage("Bamboo_branch"));
		bamboo->meshes[0]->setNormalTexture(imageLoader.getImage("Bamboo_branch_Normal"));
		// create instance data
		std::vector<InstanceData> bambooInstanceDatas;
		for (int i = 0; i < 40; i++) {
			InstanceData inst;
			float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
			float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.05f + 0.02f;
			inst.World = Mat4().Translate(61, 0, 3 * i - 60) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
			inst.World = inst.World.Transpose();
			bambooInstanceDatas.push_back(inst);
		}
		for (int i = 0; i < 40; i++) {
			InstanceData inst;
			float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
			float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.05f + 0.02f;
			inst.World = Mat4().Translate(-61, 0, 3 * i - 60) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
			inst.World = inst.World.Transpose();
			bambooInstanceDatas.push_back(inst);
		}
		for (int i = 0; i < 40; i++) {
			InstanceData inst;
			float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
			float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.05f + 0.02f;
			inst.World = Mat4().Translate(3 * i - 60, 0, 61) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
			inst.World = inst.World.Transpose();
			bambooInstanceDatas.push_back(inst);
		}
		for (int i = 0; i < 40; i++) {
			InstanceData inst;
			float randRotation = ((float)(rand() % 1000) / 1000.0f) * 360.0f;
			float randScale = ((float)(rand() % 1000) / 1000.0f) * 0.05f + 0.02f;
			inst.World = Mat4().Translate(3 * i - 60, 0, -61) * Mat4().RotateY(randRotation) * Mat4().Scale(randScale, randScale, randScale);
			inst.World = inst.World.Transpose();
			bambooInstanceDatas.push_back(inst);
		}
		// create instanced object
		InstancedObject* instanced_bamboo = new InstancedObject(&psos);
		instanced_bamboo->init(core, bamboo, bambooInstanceDatas);
		// add to world objects
		instanceDataMap.insert({ "bamboo", bambooInstanceDatas });
		instancedObjects.insert({ "bamboo", *instanced_bamboo });

		// Create UI elements
		uiManager.addUIPlane(core, -0.9f, 0.8f, 0.25f, 0.2f, imageLoader.getImage("UI_Score"), "UI_Score");
		uiManager.addUIPlane(core, -0.9f, 0.6f, 0.05f, 0.2f, imageLoader.getImage("Number_0"), "UI_Score_Tens");
		uiManager.addUIPlane(core, -0.85f, 0.6f, 0.05f, 0.2f, imageLoader.getImage("Number_0"), "UI_Score_Ones");
		uiManager.addUIPlane(core, -0.75f, 0.6f, 0.01f, 0.2f, imageLoader.getImage("UI_bar"), "UI_Score_Space");
		uiManager.addUIPlane(core, -0.7f, 0.6f, 0.05f, 0.2f, imageLoader.getImage("Number_0"), "UI_Score_To_Win_Tens");
		uiManager.addUIPlane(core, -0.65f, 0.6f, 0.05f, 0.2f, imageLoader.getImage("Number_0"), "UI_Score_To_Win_Ones");
		uiManager.addUIPlane(core, 0.7f, 0.8f, 0.2f, 0.2f, imageLoader.getImage("UI_Time"), "UI_Time");
		uiManager.addUIPlane(core, 0.7f, 0.6f, 0.05f, 0.2f, imageLoader.getImage("Number_0"), "UI_Time_Hundreds");
		uiManager.addUIPlane(core, 0.75f, 0.6f, 0.05f, 0.2f, imageLoader.getImage("Number_0"), "UI_Time_Tens");
		uiManager.addUIPlane(core, 0.8f, 0.6f, 0.05f, 0.2f, imageLoader.getImage("Number_0"), "UI_Time_Ones");
		

		// set constant buffer pointers
		shaderManager.setConstantBufferValuePointer("animatedShader", "animatedMeshBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("basicShader", "staticMeshBuffer", "W", &W, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("basicShader", "staticMeshBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("skyboxShader", "skyboxBuffer", "W", &skyboxBuffer_W, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("skyboxShader", "skyboxBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "W", &W, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "VP", &VP, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "playerPosition", &playerPos, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "staticMeshBuffer", "time", &time, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedStaticShader", "staticMeshBuffer", "W", &W, VERTEX_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedStaticShader", "staticMeshBuffer", "VP", &VP, VERTEX_SHADER);

		shaderManager.setConstantBufferValuePointer("animatedShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
		shaderManager.setConstantBufferValuePointer("basicShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);
		shaderManager.setConstantBufferValuePointer("instancedStaticShader", "basicPSBuffer", "lightDirection", &lightDirection, PIXEL_SHADER);

		// hitbox debug draw
		/*for (auto& hitbox : hitboxManager.hitboxes) {
			Object* debugBox = new Object(&psos);
			Cube* cube = new Cube();
			cube->init(core, 1.0f);
			cube->psoNames = "basicPSO";
			debugBox->meshes.push_back(cube);
			debugBox->position = hitbox->position;
			debugBox->scale = hitbox->size;
			worldObjects.push_back(*debugBox);
		}
		// hitpoint debug toggle
		Object* debugBox = new Object(&psos);
		Cube* cube = new Cube();
		cube->init(core, 1.0f);
		cube->psoNames = "basicPSO";
		debugBox->meshes.push_back(cube);
		debugBox->scale = Vec3(0.2f, 2.0f, 0.2f);
		worldObjects.push_back(*debugBox);
		eventBus.subscribe<HitboxCollisionEvent>(
			[this](const HitboxCollisionEvent& event) {
				worldObjects[worldObjects.size() - 1].position = event.contactPoint;
			});*/
	}

	void update() {
		static bool running = true;
		while (gameState == 0) {
			if (!running) return;

			win->processMessages();
			if (win->keys[VK_ESCAPE]) running = false;

			// updatte time
			float dt = timer.dt();
			time += dt;

			// update UI
			int Score_Tens = score / 10;
			int Score_Ones = score % 10;
			int ScoreToWin_Tens = scoreToWin / 10;
			int ScoreToWin_Ones = scoreToWin % 10;
			uiManager.setTexture("UI_Score_Tens", imageLoader.getImage("Number_" + std::to_string(Score_Tens)));
			uiManager.setTexture("UI_Score_Ones", imageLoader.getImage("Number_" + std::to_string(Score_Ones)));
			uiManager.setTexture("UI_Score_To_Win_Tens", imageLoader.getImage("Number_" + std::to_string(ScoreToWin_Tens)));
			uiManager.setTexture("UI_Score_To_Win_Ones", imageLoader.getImage("Number_" + std::to_string(ScoreToWin_Ones)));
			int timeLeft = timeLimit - (int)time;
			if (timeLeft < 0) timeLeft = 0;
			int Time_Hundreds = timeLeft / 100;
			int Time_Tens = (timeLeft / 10) % 10;
			int Time_Ones = timeLeft % 10;
			uiManager.setTexture("UI_Time_Hundreds", imageLoader.getImage("Number_" + std::to_string(Time_Hundreds)));
			uiManager.setTexture("UI_Time_Tens", imageLoader.getImage("Number_" + std::to_string(Time_Tens)));
			uiManager.setTexture("UI_Time_Ones", imageLoader.getImage("Number_" + std::to_string(Time_Ones)));

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
				{"grass008", grass008_update },
				{"bamboo", instanceDataMap["bamboo"] }
			};
			// update hitbox debug draw positions
			/*for (int i = 0; i < hitboxManager.hitboxes.size(); i++) {
				worldObjects[i + 1].position = hitboxManager.hitboxes[i]->position;
			}*/

			// update hitboxes
			hitboxManager.update();


			// begin frame
			core->beginFrame();

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

			// draw UI
			uiManager.draw(core);

			core->finishFrame();

			// manage score and win condition
			ScoreManager();

			// dispatch events
			eventBus.dispatch();
		}
	}

	std::vector<InstanceData> RotateInstanceObjectByPlayer(const std::string& objectName) {
		std::vector<InstanceData> updatedInstanceData;
		Vec3 playerPos = this->player->position;
		auto it = instanceDataMap.find(objectName);
		if (it != instanceDataMap.end()) {
			for (size_t i = 0; i < it->second.size(); i++) {
				// calculate rotation based on distance to player
				Mat4 instanceWorld = it->second[i].World.Transpose();	
				Vec3 instancePos = Vec3(instanceWorld.m[0][3], instanceWorld.m[1][3], instanceWorld.m[2][3]);
				Vec3 dir = playerPos - instancePos;
				float length = dir.getLength() + 0.3;
				length *= length;
				Vec3 axis = Vec3(0.0f, 1.0f, 0.0f).cross(dir).normalize();
				Mat4 rotationMat = Mat4().Rotate(30.0f * expf(-length / 5.0f), axis);
				Mat4 newWorld = Mat4().Translate(instancePos.v[0], instancePos.v[1], instancePos.v[2]) * rotationMat * Mat4().Scale(instanceWorld.m[0][0], instanceWorld.m[1][1], instanceWorld.m[2][2]);
				InstanceData newInst;
				newInst.World = newWorld.Transpose();
				updatedInstanceData.push_back(newInst);
			}
		}
		return updatedInstanceData;
	}

	void ScoreManager() {
		int _score = 0;
		// calculate score
		for (int i = 0; i < actors->numActors; i++) {
			Hen* hen = dynamic_cast<Hen*>(actors->getActorAt(i));
			if (hen->position.v[0] < 17.5f && hen->position.v[0] > -17.5f &&
				hen->position.v[2] < 7.7f && hen->position.v[2] > -12.5f) {
				_score++;
			}
		}
		score = _score;
		// check win condition
		if (score >= scoreToWin) {
			WinConditionEvent winEvent;
			winEvent.player = player;
			winEvent.score = score;
			eventBus.queue<WinConditionEvent>(winEvent);
		}
	}

	void subscribeEventHandlers() {
		eventBus.subscribe<WinConditionEvent>(
			[this](const WinConditionEvent& event) {
				gameState = 1; // win
				DebugPrint("You Win! Final Score: " + std::to_string(event.score));
			});
	}

	void initHensFromFile(const std::string& filename) {
		// read actors from file
		int scoreToWinFromFile = 0;
		std::vector<Hen*> henActors = LevelManager::ReadFromFile<Hen>(filename, scoreToWinFromFile);
		// clear existing actors
		actors->clear();
		// init hen objects
		for (int i = 0; i < scoreToWin; i++) {
			Object* hen = new Object(&psos);
			Hen* henActor = henActors[i];
			// load hen model based on type
			int r = henActor->type;
			switch (r) {
			case 0:
				hen->loadGEM(core, HEN_BROWN, "animatedPSO");
				break;
			case 1:
				hen->loadGEM(core, HEN_WHITE, "animatedPSO");
				break;
			case 2:
				hen->loadGEM(core, ROOSTER_DARK, "animatedPSO");
				break;
			case 3:
				hen->loadGEM(core, ROOSTER_BROWN, "animatedPSO");
				break;
			}
			hen->setDiffuseTexture(imageLoader.getImage("AnimalsColorMap"));
			hen->setNormalTexture(imageLoader.getImage("AnimalsNormalMap"));
			hen->rotateBy(180.0f, Y_AXIS);
			hen->scale = Vec3(0.05f, 0.05f, 0.05f);
			// init hen actor
			henActor->init(hen);
			henActor->setPlayer(this->player);
			henActor->type = r;	// save hen type
			henActor->position = henActors[i]->position;
			henActor->rotation = henActors[i]->rotation;
			henActor->scale = henActors[i]->scale;
			// subscribe to events
			henActor->subscribeEvent(&eventBus);
			// add hitbox
			hitboxManager.addHitbox(henActor, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.5f, 0.5f, 0.5f));
			this->actors->addActor(henActor);
		}
		// set score to win
		this->scoreToWin = scoreToWinFromFile;
	}
};

