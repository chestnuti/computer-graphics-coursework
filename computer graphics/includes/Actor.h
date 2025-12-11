#pragma once

#include "Vector.h"
#include "Mesh.h"
#include "Window.h"
#include "Animation.h"
#include "EventBus.h"


class Actor {
protected:
	Sequencer sequencer;
	StateMachine* stateMachine;
public:
	Object* object;

	Vec3 position;
	Vec4 rotation;
	Vec3 scale;
	Mat4 worldMatrix;

	float speed = 0.0f;
	Vec3 forward = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 right = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
	
	virtual void init(Object* obj) {
		object = obj;
		position = Vec3(0.0f, 0.0f, 0.0f);
		rotation = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		scale = Vec3(1.0f, 1.0f, 1.0f);
		sequencer = Sequencer();
		sequencer.addAllAnimations(&object->animation, 0.0f, 0.0f, 1.0f);
		stateMachine = new StateMachine(&sequencer);
	}

	void updateWorldMatrix() {
		Mat4 t = Mat4().Translate(position.v[0], position.v[1], position.v[2]);
		Mat4 r = Mat4().rotationQuaternion(rotation.v[0], rotation.v[1], rotation.v[2], rotation.v[3]);
		Mat4 s = Mat4().Scale(scale.v[0], scale.v[1], scale.v[2]);
		object->updateWorldMatrix();
		worldMatrix = t * r * s * object->worldMatrix;
	}

	Mat4* getWorldMatrix() {
		return &worldMatrix;
	}

	virtual void update(float dt) {
		// Update object
		if (object != nullptr)
		{
			// update object's transform from actor's transform
			updateWorldMatrix();
		}
		sequencer.update(dt);
	}

	void subscribeCollisionEvent(EventBus* eventBus) {
		eventBus->subscribe<HitboxCollisionEvent>(
			[this](const HitboxCollisionEvent& event) {
				// Check if this actor's object is involved in the collision
				if (event.actorA == this || event.actorB == this) {
					// push back the actor slightly along the collision normal
					Vec3 pushDir = event.contactPoint - this->position;
					pushDir.v[1] = 0.0f; // keep on ground
					pushDir = pushDir.normalize();
					if (pushDir.Dot(forward) > 0.5f)
						speed = 0.0f;
				}
			}
		);
	}

	Mat4* getBoneMatrices() {
		return sequencer.getBoneMatrices();
	}

	virtual void draw(Core* core) {
		if (object != nullptr) {
			object->psoManager->getShader("animatedPSO")->updateConstantBuffer("animatedMeshBuffer", "bones", getBoneMatrices(), VERTEX_SHADER);
			object->psoManager->getShader("animatedPSO")->updateConstantBuffer("animatedMeshBuffer", "W", getWorldMatrix(), VERTEX_SHADER);
			object->draw(core);
		}
	}
};




class Player : public Actor {
public:
	Window* win;
	Camera* camera;

	Player(Window* window) : win(window) {
		Sequencer sequencer;
	}

	void init(Object* obj) override {
		Actor::init(obj);
		//set initial state
		stateMachine->setCurrentState("idle basic 01");
	}

	void bindCamera(Camera* cam) {
		camera = cam;
	}

	void update(float dt) override {
		// control player movement
		position += forward * dt * speed;
		// update camera
		camera->control(win, dt);
		camera->bindTragetAt(position + Vec3(0.0f, 4.0f, 0.0f));
		// when any movement key is pressed, update forward and right vector
		if (win->keys['W'] || win->keys['A'] || win->keys['S'] || win->keys['D']) {
			if (win->keys[VK_SHIFT]) {
				// running
				speed += 20.0f * dt;
				if (speed > 12.0f) speed = 12.0f;
			}
			else {
				// walking
				speed += 10.0f * dt;
				if (speed > 5.0f) speed = 5.0f;
			}
			if (win->keys['W']) forward += camera->getForwardVector();
			if (win->keys['S']) forward -= camera->getForwardVector();
			if (win->keys['A']) forward -= camera->getRightVector();
			if (win->keys['D']) forward += camera->getRightVector();
			forward.v[1] = 0.0f; // keep player on the ground
			if (forward.normalize_GetLength() == 0.0f) forward = Vec3(0.0f, 0.0f, 1.0f);
			forward = forward.normalize();
			Vec4 rot = quatFromTo(Vec3(0.0f, 0.0f, 1.0f), forward);
			rotation = slerp(rotation, rot, dt * 10.0f);
		}
		else {
			speed -= 50.0f * dt;
			if (speed < 0.0f) speed = 0.0f;
		}
		

		// change state based on speed
		if (speed > 5.0f) {
			stateMachine->transitionTo("run", 0.2f);
		}
		else if (speed > 0.1f) {
			stateMachine->transitionTo("walk", 0.2f);
		}
		else {
			stateMachine->transitionTo("idle basic 01", 0.2f);
		}
		
		//update animation
		stateMachine->update(dt);

		updateWorldMatrix();
	}
};



class Hen : public Actor {
public:

	void init(Object* obj) override {
		Actor::init(obj);
		//set initial state
		stateMachine->setCurrentState("idle");
	}
};