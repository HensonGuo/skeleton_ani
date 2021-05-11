#pragma once

#include "mesh.h"
#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "material.h"
#include "skeleton.h"
#include "constants.h"

#include <iostream>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"

using namespace std;
using namespace glm;

struct NodeInfo {
	string name;
	mat4 localTransform;
	Animation* ani;
};


class Model
{
public:
	Model();
	Model(const string& path);
	void clear();
	void loadModel(const string& path);
	void loadAnimation(const string& path);
	void draw(Shader &shader, DrawType drawType);

	void playAnimation(bool active);
	bool isPlayingAnimation();
	float getAniDuration();
	float getAniElapsed();

	void changePoseStopAtTime(float delta);

	uint getBonesCount();
	uint getVertexCount();
private:
	void processNode(aiNode* node, const aiScene* scene, aiMatrix4x4 currentTransform);
	void setAnimation(aiAnimation* animation);
	void showNodeName(aiNode* node);
	void setNodeAnimation(string nodeName, Animation *ani);
	void applyNodeTransform(Shader& shader);
	
	string directory;

	uint vertexCount;
	vector<Mesh*> meshes;
	Skeleton* skeleton;

	float startTime = -1.0f;
	float durationInTicks;
	float ticksPerSecond;
	float ticksElapsed;

	std::vector<glm::mat4> nodeTransforms;
	map<uint, NodeInfo*> nodeIndex2Info;
	uint nodeCount = 0;
};