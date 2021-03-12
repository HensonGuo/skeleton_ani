#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>


using namespace std;
using namespace glm;

// ����
struct Bone {
	int id = 0;
	string name = "";
	mat4 offset = glm::mat4(1.0f);
	vector<Bone> children = {};
};

// �����任
struct BoneTransformTrack {
	vector<float> positionTimestamps = {};
	vector<float> rotationTimestamps = {};
	vector<float> scaleTimestamps = {};

	vector<glm::vec3> positions = {};
	vector<glm::quat> rotations = {};
	vector<glm::vec3> scales = {};
};