#pragma once

#include "mesh.h"
#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "material.h"

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

// 动画
struct Animation {
	double duration = 0.0f;
	double ticksPerSecond = 1.0f;
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
	void readVertices(aiMesh* aimesh);
	void readIndices(aiMesh* aimesh);
	void readBones(aiMesh* aimesh);
	bool readSkeleton(Bone& boneOutput, aiNode* node, unordered_map<string, pair<int, mat4>>& boneInfoTable);
	void readMaterial(aiMaterial* material);
	void readAnimation(const aiScene* scene);
	void normalizeBonesWeight();
	std::pair<uint, float> getTimeFraction(std::vector<float>& times, float& dt);
	
	string directory;

	Mesh mesh;	
	vector<Material*> materials = {};
	vector<Vertex> vertices = {};
	vector<uint> indices = {};
	uint boneCount = 0;
	unordered_map<string, pair<int, mat4>> boneInfo = {};
	Bone skeleton;
	Animation animation;
	//将姿势保存在currentPose向量中，并作为矩阵数组上传到gpu
	std::vector<glm::mat4> currentPose = {};
	mat4 identity;
	mat4 globalInverseTransform;//全局变换
};