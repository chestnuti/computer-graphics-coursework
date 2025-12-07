#pragma once
#include <string>
#include <vector>
#include <map>
#include "Matrix.h"
#include "Vector.h"
#include "Operators.h"


struct Bone
{
	std::string name;
	Mat4 offset;
	int parentIndex;
};

struct Skeleton
{
	std::vector<Bone> bones;
	Mat4 globalInverse;
};

struct AnimationFrame
{
	std::vector<Vec3> positions;
	std::vector<Vec4> rotations;
	std::vector<Vec3> scales;
};



class AnimationSequence {
public:
	std::vector<AnimationFrame> frames;
	float ticksPerSecond;

	Vec3 interpolate(Vec3 p1, Vec3 p2, float t) {
		return ((p1 * (1.0f - t)) + (p2 * t));
	}

	Vec4 interpolate(Vec4 q1, Vec4 q2, float t) {
		return slerp(q1, q2, t);
	}

	float duration() {
		return ((float)frames.size() / ticksPerSecond);
	}

	void calcFrame(float t, int& frame, float& interpolationFact)
	{
		interpolationFact = t * ticksPerSecond;
		frame = (int)floorf(interpolationFact);
		interpolationFact = interpolationFact - (float)frame;
		frame = min(frame, frames.size() - 1);
	}

	int nextFrame(int frame)
	{
		return min(frame + 1, frames.size() - 1);

	}

	Mat4 interpolateBoneToGlobal(Mat4* matrices, int baseFrame, float interpolationFact, Skeleton* skeleton, int boneIndex)
	{
		// scale
		Vec3 scaleFactor = interpolate(frames[baseFrame].scales[boneIndex], frames[nextFrame(baseFrame)].scales[boneIndex], interpolationFact);
		Mat4 scale = Mat4().Scale(scaleFactor.v[0], scaleFactor.v[1], scaleFactor.v[2]);
		// rotation
		Vec4 rotationFactor = interpolate(frames[baseFrame].rotations[boneIndex], frames[nextFrame(baseFrame)].rotations[boneIndex], interpolationFact);
		float x = rotationFactor.v[0];
		float y = rotationFactor.v[1];
		float z = rotationFactor.v[2];
		float w = rotationFactor.v[3];
		Mat4 rotation;
		rotation.m[0][0] = 1 - 2 * y * y - 2 * z * z;
		rotation.m[0][1] = 2 * x * y - 2 * z * w;
		rotation.m[0][2] = 2 * x * z + 2 * y * w;
		rotation.m[1][0] = 2 * x * y + 2 * z * w;
		rotation.m[1][1] = 1 - 2 * x * x - 2 * z * z;
		rotation.m[1][2] = 2 * y * z - 2 * x * w;
		rotation.m[2][0] = 2 * x * z - 2 * y * w;
		rotation.m[2][1] = 2 * y * z + 2 * x * w;
		rotation.m[2][2] = 1 - 2 * x * x - 2 * y * y;
		// translation
		Vec3 position = interpolate(frames[baseFrame].positions[boneIndex], frames[nextFrame(baseFrame)].positions[boneIndex], interpolationFact);
		Mat4 translation = Mat4().Translate(position.v[0], position.v[1], position.v[2]);
		Mat4 local = translation * rotation * scale;
		if (skeleton->bones[boneIndex].parentIndex > -1)
		{
			Mat4 global = matrices[skeleton->bones[boneIndex].parentIndex] * local;
			return global;
		}
		return local;
	}
};



class Animation {
public:
	std::map<std::string, AnimationSequence> animations;
	Skeleton skeleton;

	void calcFrame(std::string name, float t, int& frame, float& interpolationFact)
	{
		animations[name].calcFrame(t, frame, interpolationFact);
	}

	Mat4 interpolateBoneToGlobal(std::string name, Mat4* matrices, int baseFrame, float interpolationFact, int boneIndex) 
	{
		return animations[name].interpolateBoneToGlobal(matrices, baseFrame, interpolationFact, &skeleton, boneIndex);
	}

	int bonesSize()
	{
		return (int)skeleton.bones.size();
	}

	void calcFinalTransforms(Mat4* matrices)
	{
		for (int i = 0; i < bonesSize(); i++)
		{
			matrices[i] = matrices[i] * skeleton.bones[i].offset * skeleton.globalInverse;
		}
	}

};



// Holds an instance of animation data, allows multiple characters to use same animation data
class AnimationInstance {
public:
	Animation* animation;
	std::string currentAnimation;
	float t;
	Mat4 matrices[256];

	void resetAnimationTime()
	{
		t = 0;
	}

	bool animationFinished()
	{
		if (t > animation->animations[currentAnimation].duration())
		{
			return true;
		}
		return false;
	}

	void update(std::string name, float dt) {
		if (name == currentAnimation) {
			t += dt;
		}
		else {
			currentAnimation = name;  t = 0;
		}
		if (animationFinished() == true) { resetAnimationTime(); }
		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(name, t, frame, interpolationFact);
		for (int i = 0; i < animation->bonesSize(); i++)
		{
			matrices[i] = animation->interpolateBoneToGlobal(name, matrices, frame, interpolationFact, i);
		}
		animation->calcFinalTransforms(matrices);
	}

	Mat4* getBoneMatrices() {
		return matrices;
	}

};



class Sequencer {
public:
	struct Item {
		AnimationInstance* animationInstance;
		std::string animationName;
		float weight;
		float startTime;
		float speed;
	};
	float totalWeight;
	std::vector<Item> items;
	Mat4 matrices[256];

	Sequencer() : totalWeight(0.0f) {}

	void addItem(Animation* animation, std::string name, float weight, float startTime, float speed) {
		Item item;
		item.animationInstance = new AnimationInstance();
		item.animationInstance->animation = animation;
		item.animationName = name;
		item.weight = weight;
		item.startTime = startTime;
		item.speed = speed;
		items.push_back(item);
		totalWeight += weight;
	}

	void update(float globalTime, float dt) {
		for (int i = 0; i < items.size(); i++) {
			if (globalTime >= items[i].startTime) {
				items[i].animationInstance->update(items[i].animationName, dt * items[i].speed);
			}
		}
		// blend matrices
		for (int b = 0; b < 256; b++) {
			matrices[b] = Mat4()._Identity().Scale(0.0f, 0.0f, 0.0f); // reset to zero matrix
			for (int i = 0; i < items.size(); i++) {
				if (globalTime >= items[i].startTime) {
					Mat4* boneMatrices = items[i].animationInstance->getBoneMatrices();
					if (totalWeight > 0.0f) {
						boneMatrices[b] = boneMatrices[b] * (items[i].weight / totalWeight);
						matrices[b] += boneMatrices[b];
					}
				}
			}
			
		}
	}

	Mat4* getBoneMatrices() {
		return matrices;
	}

	void setWeights(std::vector<float> weights) {
		if (weights.size() != items.size()) return;
		totalWeight = 0.0f;
		for (int i = 0; i < items.size(); i++) {
			items[i].weight = weights[i];
			totalWeight += weights[i];
		}
	}

	void setWeight(int index, float weight) {
		if (index < 0 || index >= items.size()) return;
		totalWeight = totalWeight - items[index].weight + weight;
		items[index].weight = weight;
	}

	void clearWeights() {
		for (int i = 0; i < items.size(); i++) {
			items[i].weight = 0.0f;
		}
		totalWeight = 0.0f;
	}

	~Sequencer() {
		items.clear();
	}
};



class StateMachine {
public:
	Sequencer* sequencer;
	std::string currentState;
	std::vector<std::string> stateList;
	std::vector<float> transitionTimes;
	float transTime;

	StateMachine(Sequencer* seq) : sequencer(seq), currentState(""), transTime(0.0f) {}

	int getStateIndex(std::string state) {
		for (int i = 0; i < sequencer->items.size(); i++) {
			if (sequencer->items[i].animationName == state) {
				return i;
			}
		}
		return -1;
	}

	void transitionTo(std::string state, float time) {
		stateList.push_back(state);
		transitionTimes.push_back(time);
	}

	void setCurrentState(std::string state) {
		currentState = state;
		stateList.clear();
		int currentIndex = getStateIndex(currentState);
		if (currentIndex != -1) {
			sequencer->clearWeights();
			sequencer->setWeight(currentIndex, 1.0f);
		}
	}

	void update(float globalTime, float dt) {
		// No states to process
		if (stateList.size() != 0) {
			// First time setup
			if (currentState == "") {
				currentState = stateList[0];
				stateList.erase(stateList.begin());
				transitionTimes.erase(transitionTimes.begin());
				int currentIndex = getStateIndex(currentState);
				if (currentIndex != -1) {
					sequencer->clearWeights();
					sequencer->setWeight(currentIndex, 1.0f);
				}
			}
			else {
				// Process transition
				std::string nextState = stateList[0];
				// Initialize transition time
				if (transTime <= 0.0f)
					transTime = transitionTimes[0];
				// Update transition time
				transTime -= dt;
				// Update weights
				int currentIndex = getStateIndex(currentState);
				int nextIndex = getStateIndex(nextState);
				if (currentIndex != -1 && nextIndex != -1) {
					float factor = remap(transTime, 0.0f, transitionTimes[0], 1.0f, 0.0f);
					sequencer->setWeight(currentIndex, 1.0f - factor);
					sequencer->setWeight(nextIndex, factor);
				}
				// Check if transition complete
				if (transTime <= 0.0f) {
					currentState = nextState;
					// Remove first state from list
					stateList.erase(stateList.begin());
					transitionTimes.erase(transitionTimes.begin());
					if (stateList.size() > 0)
						transTime = transitionTimes[0];
				}
			}
		}
		// Update sequencer
		sequencer->update(globalTime, dt);
	}
};