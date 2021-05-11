#pragma once

#include "mesh.h"
#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "material.h"
#include "skeleton.h"
#include "constants.h"
#include "node.h"

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
	NodeInfo* processNode(aiNode* node, const aiScene* scene, aiMatrix4x4 currentTransform);
	void setAnimation(aiAnimation* animation);
	void showNodeName(aiNode* node);
	void calculateNodeTransform(NodeInfo* node, glm::mat4 parentTransform, float delta);
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
	map<string, NodeInfo*> nodeName2Info;
	NodeInfo* rootNode;
	uint nodeCount = 0;
};