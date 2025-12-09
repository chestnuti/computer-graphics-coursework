#pragma once

#include "Vector.h"
#include "Mesh.h"
#include "Window.h"
#include "Animation.h"


class Actor {
public:
	Object* object;

	Vec3 position;
	Vec4 rotation;
	Vec3 scale;
	Mat4 worldMatrix;

	float speed = 10.0f;
	Vec3 forward = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 right = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
	
	virtual void init(Object* obj) {
		object = obj;
		position = Vec3(0.0f, 0.0f, 0.0f);
		rotation = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		scale = Vec3(1.0f, 1.0f, 1.0f);
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
	}
};




class Player : public Actor {
private:
	Sequencer sequencer;
public:
	Window* win;
	Camera* camera;
	StateMachine* stateMachine;

	Player(Window* window) : win(window) {
		Sequencer sequencer;
	}

	void init(Object* obj) override {
		Actor::init(obj);
		sequencer = Sequencer();
		sequencer.addItem(&obj->animation, "idle basic 01", 0.0f, 0.0f, 1.0f);
		sequencer.addItem(&obj->animation, "idle basic 02", 0.0f, 0.0f, 1.0f);
		sequencer.addItem(&obj->animation, "walk", 0.0f, 0.0f, 1.0f);
		sequencer.addItem(&obj->animation, "run", 0.0f, 0.0f, 1.0f);
		sequencer.addItem(&obj->animation, "jump", 0.0f, 0.0f, 1.0f);
		sequencer.addItem(&obj->animation, "land", 0.0f, 0.0f, 1.0f);
		sequencer.addItem(&obj->animation, "fall loop", 0.0f, 0.0f, 1.0f);
		//Create State Machine
		stateMachine = new StateMachine(&sequencer);
		stateMachine->setCurrentState("idle basic 01");
	}

	void bindCamera(Camera* cam) {
		camera = cam;
	}

	void update(float dt) override {
		// update camera
		camera->control(win, dt);
		camera->bindTragetAt(position + Vec3(0.0f, 4.0f, 0.0f));
		// when any movement key is pressed, update forward and right vector
		if (win->keys['W'] || win->keys['A'] || win->keys['S'] || win->keys['D'] || win->keys['Q'] || win->keys['E']) {
			speed += 50.0f * dt;
			if (speed > 10.0f) speed = 10.0f;
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
		// control player movement
		position += forward * dt * speed;

		// change state based on speed
		if (speed > 6.0f) {
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

		// Call base class update to update the object
		Actor::update(dt);
	}

	Mat4* getBoneMatrices() {
		return sequencer.getBoneMatrices();
	}
};
