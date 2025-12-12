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

	void init(Core* core, float positionX, float positionY, float sizeX, float sizeY) {
		vertices.push_back({ {positionX, positionY + sizeY}, {0.0f, 0.0f} }); // Top-left
		vertices.push_back({ {positionX + sizeX, positionY + sizeY}, {1.0f, 0.0f} }); // Top-right
		vertices.push_back({ {positionX, positionY}, {0.0f, 1.0f} }); // Bottom-left
		vertices.push_back({ {positionX + sizeX, positionY}, {1.0f, 1.0f} }); // Bottom-right

		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);
		Mesh::init(core, vertices, indices);
	}
};



class UIManager : public Object {
public:

	void addUIPlane(Core* core, float positionX, float positionY, float sizeX, float sizeY, Image* image) {
		UIPlane* uiPlane = new UIPlane();
		uiPlane->init(core, positionX, positionY, sizeX, sizeY);
		uiPlane->setDiffuseTexture(image);
		uiPlane->psoNames = "uiPSO";
		meshes.push_back(uiPlane);
	}

	void draw(Core* core) {
		for (int i = 0; i < meshes.size(); i++) {
			psoManager->getShader(meshes[i]->psoNames)->updateAllConstantBuffers();
			psoManager->set(core, meshes[i]->psoNames);
			meshes[i]->draw(core, psoManager->getShader(meshes[i]->psoNames));
			psoManager->advance(meshes[i]->psoNames);
		}
	}
};