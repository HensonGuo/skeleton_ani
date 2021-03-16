#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/scene.h>

#include "utils.h"


using namespace std;
using namespace glm;

struct Keyframe {
	vec3 translationV;
	vec3 scalingV;
	aiQuaternion quatV;
	float timeInTicks;

	Keyframe();
	Keyframe(vec3 translationV, vec3 scalingV, aiQuaternion quatV, float timeInTicks);
	Keyframe& operator=(const Keyframe& k);
};


// นว๗ภ
class Bone {
public:
	int id = 0;
	string name = "";
	Bone* parent;
	vector<Bone*> children = {};
	glm::mat4 invBindPoseM;
	glm::mat4 localAnimationM;
	glm::mat4 globalAnimationM;

	vector<Keyframe*> keyframes;
	glm::mat4 getCurrentTransform(float ticksElapsed);
};