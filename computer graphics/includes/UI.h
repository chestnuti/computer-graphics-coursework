#pragma once
#include "Core.h"
#include "Window.h"
#include "Mesh.h"
#include "Camera.h"
#include "Image.h"
#include <string>



class UIPlane : public Mesh {
public:
	std::vector<UI_VERTEX> vertices;
	std::vector<unsigned int> indices;

	bool canDraw = true;

	float uiOffset[2] = { 0.0f, 0.0f };
	float uiScale[2] = { 1.0f, 1.0f };

	void init(Core* core, float positionX, float positionY, float sizeX, float sizeY) {
		vertices.push_back({ {positionX, positionY + sizeY}, {0.0f, 0.0f} }); // Top-left
		vertices.push_back({ {positionX + sizeX, positionY + sizeY}, {1.0f, 0.0f} }); // Top-right
		vertices.push_back({ {positionX, positionY}, {0.0f, 1.0f} }); // Bottom-left
		vertices.push_back({ {positionX + sizeX, positionY}, {1.0f, 1.0f} }); // Bottom-right

		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);
		Mesh::init(core, vertices, indices);
	}

	void setPosition(float positionX, float positionY) {
		uiOffset[0] = positionX;
		uiOffset[1] = positionY;
	}
	void setScale(float scaleX, float scaleY) {
		uiScale[0] = scaleX;
		uiScale[1] = scaleY;
	}

	void draw(Core* core, Shader* shader) override{
		if (!canDraw) return;
		Mesh::draw(core, shader);
	}
};



class UIManager : public Object {
public:
	std::unordered_map<std::string, UIPlane*> meshes;

	void addUIPlane(Core* core, float positionX, float positionY, float sizeX, float sizeY, Image* image, std::string name) {
		UIPlane* uiPlane = new UIPlane();
		uiPlane->init(core, positionX, positionY, sizeX, sizeY);
		uiPlane->setDiffuseTexture(image);
		uiPlane->psoNames = "uiPSO";
		meshes.insert({ name, uiPlane });
	}

	void draw(Core* core) {
		for (auto& [name, uiPlane] : meshes) {
			if (!uiPlane->canDraw) continue;
			psoManager->getShader(uiPlane->psoNames)->updateConstantBuffer("UIBuffer", "uioffset", &uiPlane->uiOffset, VERTEX_SHADER);
			psoManager->getShader(uiPlane->psoNames)->updateConstantBuffer("UIBuffer", "uiscale", &uiPlane->uiScale, VERTEX_SHADER);
			psoManager->getShader(uiPlane->psoNames)->updateAllConstantBuffers();
			psoManager->set(core, uiPlane->psoNames);
			uiPlane->draw(core, psoManager->getShader(uiPlane->psoNames));
			psoManager->advance(uiPlane->psoNames);
		}
	}

	void setTexture(std::string name, Image* image) {
		if (meshes.find(name) != meshes.end()) {
			meshes[name]->setDiffuseTexture(image);
		}
	}
};