#pragma once

#include "mesh.h"
#include "bone.h"
#include "utils.h"
#include "stb_image.h"
#include "shader.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"

using namespace std;
using namespace glm;

// ����
struct Animation {
	float duration = 0.0f;
	float ticksPerSecond = 1.0f;
	unordered_map<string, BoneTransformTrack> boneTransforms = {};
};

class Model
{
public:
	Model(const string& path);
	void loadModel(const string& path);
	void draw(Shader &shader);
	void getPose(std::vector<glm::mat4>& output, Bone& skeletion, float dt, mat4& parentTransform, mat4& globalInverseTransform);

private:
	bool readSkeleton(Bone& boneOutput, aiNode* node, unordered_map<string, pair<int, mat4>>& boneInfoTable);
	void readTexture(const string& path);
	void readAnimation(const aiScene* scene);
	std::pair<uint, float> getTimeFraction(std::vector<float>& times, float& dt);
	
	Mesh mesh;
	GLuint textureId = 0;
	vector<Vertex> vertices = {};
	vector<uint> indices = {};
	uint boneCount = 0;
	Bone skeleton;
	Animation animation;
	//�����Ʊ�����currentPose�����У�����Ϊ���������ϴ���gpu
	std::vector<glm::mat4> currentPose = {};
	mat4 identity;
	mat4 globalInverseTransform;//ȫ�ֱ任
};