#pragma once
#include "Vector.h"
#include "Actor.h"
#include "EventBus.h"
#include "Operators.h"


class Hitbox {
public:
	Vec3 local_position;

	Actor* parent;

	Vec3 position;
	Vec3 size;

	Hitbox(Actor* pParent, const Vec3& pLocalPosition, const Vec3& pSize)
		: parent(pParent), local_position(pLocalPosition), size(pSize) {
		update();
	}

	HitboxCollisionEvent checkCollision(const Hitbox& other) {
		HitboxCollisionEvent info;
		info.collided = false;
		info.contactPoint = Vec3(0.0f, 0.0f, 0.0f);
		info.contactNormal = Vec3(0.0f, 0.0f, 0.0f);
		info.actorA = nullptr;
		info.actorB = nullptr;
		if (position.v[0] + size.v[0] < other.position.v[0] - other.size.v[0] ||
			position.v[0] - size.v[0] > other.position.v[0] + other.size.v[0]) {
			return info;
		}
		if (position.v[1] + size.v[1] < other.position.v[1] - other.size.v[1] ||
			position.v[1] - size.v[1] > other.position.v[1] + other.size.v[1]) {
			return info;
		}
		if (position.v[2] + size.v[2] < other.position.v[2] - other.size.v[2] ||
			position.v[2] - size.v[2] > other.position.v[2] + other.size.v[2]) {
			return info;
		}
		info.collided = true;
		info.contactPoint = Vec3(
			(position.v[0] * other.size.v[0] + other.position.v[0] * size.v[0]) / (size.v[0] + other.size.v[0]),
			(position.v[1] * other.size.v[1] + other.position.v[1] * size.v[1]) / (size.v[1] + other.size.v[1]),
			(position.v[2] * other.size.v[2] + other.position.v[2] * size.v[2]) / (size.v[2] + other.size.v[2])
		);
		info.contactNormal = (other.position - position).normalize();
		info.actorA = parent;
		info.actorB = other.parent;
		return info;
	}

	void update() {
		if (parent != nullptr) {
			position = parent->position + local_position;
		}
		else {
			position = local_position;
		}
	}

	void queueEvent(EventBus* eventBus, HitboxCollisionEvent info) {
		if (eventBus != nullptr) {
			eventBus->queue<HitboxCollisionEvent>(info);
		}
	}
};



class HitboxManager {
public:
	std::vector<Hitbox*> hitboxes;

	EventBus* eventBus;

	HitboxManager(EventBus* pEventBus) : eventBus(pEventBus) {}

	void addHitbox(Actor* pParent, const Vec3& pLocalPosition, const Vec3& pSize) {
		Hitbox* hitbox = new Hitbox(pParent, pLocalPosition, pSize);
		hitboxes.push_back(hitbox);
	}

	void update() {
		for (auto& hitbox : hitboxes) {
			hitbox->update();
		}
		// Check for collisions
		for (size_t i = 0; i < hitboxes.size(); i++) {
			for (size_t j = i + 1; j < hitboxes.size(); j++) {
				HitboxCollisionEvent info = hitboxes[i]->checkCollision(*hitboxes[j]);
				if (info.collided) {
					if (info.actorA != nullptr || info.actorB != nullptr) {
						/*DebugPrint("Collision between Actor A and Actor B, contant point: (" +
							std::to_string(info.contactPoint.v[0]) + ", " +
							std::to_string(info.contactPoint.v[1]) + ", " +
							std::to_string(info.contactPoint.v[2]) + ")");*/
						hitboxes[i]->queueEvent(eventBus, info);
					}
				}
			}
		}
	}
};
