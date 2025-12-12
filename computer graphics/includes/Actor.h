#pragma once

#include "Vector.h"
#include "Mesh.h"
#include "Window.h"
#include "Animation.h"
#include "EventBus.h"
#include <memory>


class Actor {
protected:
	Sequencer sequencer;
	StateMachine stateMachine;
	EventBus* eventBus;
	Object* object;

public:
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
		stateMachine = StateMachine(&sequencer);
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
		updateWorldMatrix();
		sequencer.update(dt);
	}

	virtual void subscribeEvent(EventBus* eventBus) {
		this->eventBus = eventBus;
		// Subscribe to hitbox collision events
		eventBus->subscribe<HitboxCollisionEvent>(
			[this](const HitboxCollisionEvent& event) {
				// Check if this actor's object is involved in the collision
				if (event.actorA == this || event.actorB == this) {
					// push back the actor slightly along the collision normal
					Vec3 pushDir = event.contactPoint - this->position;
					pushDir.v[1] = 0.0f; // keep on ground
					pushDir = pushDir.normalize();
					if (pushDir.Dot(forward) > 0.0f) {
						speed = 0.0f;
					}
				}
			}
		);
	}

	Mat4* getBoneMatrices() {
		return sequencer.getBoneMatrices();
	}

	virtual void draw(Core* core) {
		object->psoManager->getShader("animatedPSO")->updateConstantBuffer("animatedMeshBuffer", "bones", getBoneMatrices(), VERTEX_SHADER);
		object->psoManager->getShader("animatedPSO")->updateConstantBuffer("animatedMeshBuffer", "W", getWorldMatrix(), VERTEX_SHADER);
		object->draw(core);
	}
};




class Player : public Actor {
private:
	Window* win;
	Camera* camera;

	bool isCatching = false;

public:
	Player(Window* window) : win(window) {
		Sequencer sequencer;
	}

	void init(Object* obj) override {
		Actor::init(obj);
		//set initial state
		stateMachine.setCurrentState("idle basic 01");
	}

	void bindCamera(Camera* cam) {
		camera = cam;
	}

	void update(float dt) override {
		// control player movement
		position += forward * dt * speed;
		// handle control
		control(dt);

		// change state based on speed
		if (speed > 5.0f) {
			stateMachine.transitionTo("run", 0.3f);
		}
		else if (speed > 0.1f) {
			if (isCatching)
				stateMachine.transitionTo("walk carry", 0.2f);
			else
				stateMachine.transitionTo("walk", 0.2f);
		}
		else {
			if (isCatching)
				stateMachine.transitionTo("idle basic 02", 0.2f);
			else
				stateMachine.transitionTo("idle basic 01", 0.2f);
		}
		
		//update animation
		stateMachine.update(dt);

		updateWorldMatrix();
	}

	void control(float dt) {
		// update camera
		camera->control(win, dt);
		camera->bindTragetAt(position + Vec3(0.0f, 4.0f, 0.0f));
		// when any movement key is pressed, update forward and right vector
		if (win->keys['W'] || win->keys['A'] || win->keys['S'] || win->keys['D']) {
			if (win->keys[VK_SHIFT] && !isCatching) {
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

		// catch chickens
		static bool eKeyPressed = false;
		if (win->keys['E']) {
			if (!isCatching && !eKeyPressed) {
				// send catch event
				PlayerCatchEvent catchEvent;
				catchEvent.player = this;
				catchEvent.catchPosition = position;
				catchEvent.playerForward = forward;
				eventBus->queue<PlayerCatchEvent>(catchEvent);
				stateMachine.transitionTo("grab low", 0.1f);
				stateMachine.sequencer->resetTime();
				eKeyPressed = true;	
			}
		}
		else {
			eKeyPressed = false;
		}
		// release chickens
		static bool qKeyPressed = false;
		if (win->keys['Q'] && !qKeyPressed) {
			if (isCatching) {
				isCatching = false;
				PlayerReleaseEvent releaseEvent;
				releaseEvent.player = this;
				releaseEvent.releasePosition = position;
				releaseEvent.playerForward = forward;
				eventBus->queue<PlayerReleaseEvent>(releaseEvent);
				stateMachine.transitionTo("grab low", 0.1f);
				stateMachine.sequencer->resetTime();
				qKeyPressed = true;
			}
		}
		else {
			qKeyPressed = false;
		}
	}

	void subscribeEvent(EventBus* eventBus) override {
		Actor::subscribeEvent(eventBus);
		// hen catched event
		eventBus->subscribe<HenCatchedEvent>(
			[this](const HenCatchedEvent& event) {
				isCatching = true;
				DebugPrint("is catching: " + std::to_string(isCatching));
			}
		);
	}
};



class Hen : public Actor {
private:
	Player* player;
	bool isScared = false;
	bool isCatched = false;

public:

	void init(Object* obj) override {
		Actor::init(obj);
		//set initial state
		stateMachine.setCurrentState("idle");
	}

	void setPlayer(Player* p) {
		player = p;
	}

	void update(float dt) override {
		// check distance to player
		float dist = (player->position - position).getLength();
		static bool catchEventSent = false;
		if (!isCatched) {
			// control player movement
			position += forward * dt * speed;
			if (dist < 10.0f && !isScared) {
				isScared = true;	
			}
			else if (dist >= 20.0f && isScared) {
				isScared = false;
			}
			// if scared, move away from player
			if (isScared) {
				speed += 5.0f * dt;
				if (speed > 10.0f) speed = 10.0f;
				Vec3 dir = position - player->position;
				dir.v[1] = 0.0f; // keep on ground
				dir = dir.normalize();
				if (dir.Dot(forward) < -0.5f)
					forward = dir;
				Vec4 rot = quatFromTo(Vec3(0.0f, 0.0f, 1.0f), forward);
				rotation = slerp(rotation, rot, dt * 10.0f);
			}
			else{
				speed -= 10.0f * dt;
				if (speed < 0.0f) speed = 0.0f;
			}
			// change state based on speed
			if (speed > 0.0f) {
				stateMachine.transitionTo("run forward", 0.1f);
			}
			else {
				stateMachine.transitionTo("idle", 1.0f);
			}
			catchEventSent = false;
		}
		else{
			// if catched, stick to player
			speed = 0.0f;
			rotation = player->rotation;
			position = player->position + Vec3(0.0f, 3.0f, 0.0f);
			if (!catchEventSent) {
				// Send hen catched event
				HenCatchedEvent catchedEvent;
				catchedEvent.hen = this;
				catchedEvent.player = player;
				this->eventBus->queue<HenCatchedEvent>(catchedEvent);
				catchEventSent = true;
			}
		}
		
		//update animation
		stateMachine.update(dt);
		updateWorldMatrix();
	}

	void subscribeEvent(EventBus* eventBus) override {
		Actor::subscribeEvent(eventBus);

		// Subscribe to hitbox collision events
		eventBus->subscribe<HitboxCollisionEvent>(
			[this](const HitboxCollisionEvent& event) {
				// Check if this actor's object is involved in the collision
				if (event.actorA == this || event.actorB == this) {
					// push back the actor slightly along the collision normal
					Vec3 pushDir = event.contactPoint - this->position;
					pushDir.v[1] = 0.0f; // keep on ground
					pushDir = pushDir.normalize();
					if (pushDir.Dot(forward) > 0.0f) {
						speed = 0.0f;
						forward = Vec3(rand() % 100 - 50.0f, 0.0f, rand() % 100 - 50.0f).normalize();
					}
				}
			}
		);

		// Subscribe to catch events
		eventBus->subscribe<PlayerCatchEvent>(
			[this](const PlayerCatchEvent& event) {
				// Check if player is close enough and facing the hen
				float dist = (event.catchPosition - this->position).getLength();
				if (dist < 3.0f) {
					Vec3 toHen = (this->position - event.catchPosition).normalize();
					float dot = toHen.Dot(event.playerForward);
					if (dot > 0.5f) {
						// Hen is caught
						isCatched = true;
						speed = 0.0f;
						stateMachine.transitionTo("swim eating", 0.1f);
						object->position += Vec3(0.0f, 0.0f, 1.0f); // raise hen above player
					}
				}
			}
		);

		// Subscribe to release events
		eventBus->subscribe<PlayerReleaseEvent>(
			[this](const PlayerReleaseEvent& event) {
				// Release the hen
				if (isCatched) {
					position = event.releasePosition + event.playerForward * 2.0f;
					position.v[1] = 0.0f; // keep on ground
					isCatched = false;
					isScared = true;
					stateMachine.transitionTo("run forward", 0.1f);
					object->position -= Vec3(0.0f, 0.0f, 1.0f); // raise hen above player
				}
			}
		);
	}
};



class ActorList {
private:
	struct ActorNode {
		Actor* actor;
		ActorNode* next;
	};

public:
	ActorNode* head;

	ActorList() : head(nullptr) {}

	void addActor(Actor* actor) {
		ActorNode* newNode = new ActorNode();
		newNode->actor = actor;
		newNode->next = head;
		head = newNode;
	}

	void update(float dt) {
		ActorNode* current = head;
		while (current != nullptr) {
			current->actor->update(dt);
			current = current->next;
		}
	}

	void draw(Core* core) {
		ActorNode* current = head;
		while (current != nullptr) {
			current->actor->draw(core);
			current = current->next;
		}
	}
};