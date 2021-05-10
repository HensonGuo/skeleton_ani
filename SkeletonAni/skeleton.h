#pragma once

#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "line_drawer.h"
#include "constants.h"

#include <map>
#include <GLFW/glfw3.h>
#include <iostream>


class Skeleton {
public:
	Bone* rootBone;
	std::vector<Bone*> bones;
	std::vector<glm::mat4> bone2MeshTransforms;
	std::vector<glm::mat4> boneTransforms;
	std::map<std::string, unsigned int> boneName2Index;
	glm::mat4 globalTransform;
	int boneCount = 0;

	float startTime = -1.0f;
	float durationInTicks;
	float ticksPerSecond;
	float ticksElapsed;
	bool animationActive = false;


	Skeleton();
	~Skeleton();
	void readBones(aiMesh* mesh);
	void setRootInfo(aiNode* rootNode, const map<string, mat4>&nodeName2LocalTransform);
	void setAnimation(aiAnimation* animation);
	void draw(Shader& shader);
	void changePose(Shader& shader, DrawType drawType);
	void keepPose(Shader& shader, DrawType drawType);
	void reCalculateTransform(float elapsed);
	bool hasBones();
private:
	map<string, mat4> nodeName2LocalTransform;
	void applyPose(Shader& shader, DrawType drawType);
	Bone* createBoneHierarchy(aiNode* node);
	void calculateBoneTransform(Bone* bone, glm::mat4 parentTransform, float delta);
	aiNodeAnim* findNodeAnim(const aiAnimation* ani, const std::string& nodeName);

	LineDrawer transformPoseLineDrawer;
	void updateTransformDrawer(Bone* bone, mat4 currentTransform);

	LineDrawer bindPoseLineDrawer;
	void connectBones(Bone* bone);
};