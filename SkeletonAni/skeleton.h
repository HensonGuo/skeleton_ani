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
	std::vector<glm::mat4> boneTransforms;
	std::vector<glm::mat4> modelTransforms;
	std::map<std::string, unsigned int> boneName2Index;
	glm::mat4 globalTransform;

	float startTime = -1.0f;
	float durationInTicks;
	float ticksPerSecond;
	float ticksElapsed;
	bool animationActive = false;

	Skeleton();
	void readBones(aiMesh* mesh);
	void setRootInfo(aiNode* rootNode);
	void setAnimation(aiAnimation* animation);
	void draw(Shader& shader);
	void changePose(Shader& shader, DrawType drawType);
	void keepPose(Shader& shader, DrawType drawType);
	void reCalculateTransform(float elapsed);
private:
	void applyPose(Shader& shader, DrawType drawType);
	Bone* createBoneHierarchy(aiNode* node, aiMatrix4x4 currentTransform);
	void calculateBoneTransform(Bone* bone, glm::mat4 parentTransform, float delta);
	aiNodeAnim* findNodeAnim(const aiAnimation* ani, const std::string& nodeName);

	LineDrawer transformLineDrawer;
	void updateTransformDrawer(Bone* bone, mat4 currentTransform);

	LineDrawer skeletonLineDrawer;
	void connectBones(Bone* bone);
};