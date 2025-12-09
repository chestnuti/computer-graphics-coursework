#pragma once
#include "Vector.h"
#include "Actor.h"


struct HitboxCollisionInfo {
	Hitbox* hitboxA;
	Hitbox* hitboxB;
};


class Hitbox {
public:
	Vec3 local_position;

	Actor* parent;

	Vec3 position;
	Vec3 size;

	Hitbox() : position(0.0f, 0.0f, 0.0f), size(1.0f, 1.0f, 1.0f) {}

	HitboxCollisionInfo checkCollision(const Hitbox& other) {
		HitboxCollisionInfo info;
		info.hitboxA = nullptr;
		info.hitboxB = nullptr;
		if (position.v[0] + size.v[0] < other.position.v[0] ||
			position.v[0] > other.position.v[0] + other.size.v[0]) {
			return info;
		}
		if (position.v[1] + size.v[1] < other.position.v[1] ||
			position.v[1] > other.position.v[1] + other.size.v[1]) {
			return info;
		}
		if (position.v[2] + size.v[2] < other.position.v[2] ||
			position.v[2] > other.position.v[2] + other.size.v[2]) {
			return info;
		}
		info.hitboxA = this;
		info.hitboxB = (Hitbox*)&other;
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
};



class HitboxManager {
public:
	std::vector<Hitbox*> hitboxes;

	void addHitbox(Hitbox* hitbox) {
		hitboxes.push_back(hitbox);
	}

	void update() {
		for (auto& hitbox : hitboxes) {
			hitbox->update();
		}
		
	}
};



class CollisionEvent {
public:

	void onCollision(HitboxCollisionInfo info) {
		// Handle collision event
	}
};