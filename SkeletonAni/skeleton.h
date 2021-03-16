#pragma once

#include "bone.h"
#include "utils.h"
#include "shader.h"

#include <map>
#include <GLFW/glfw3.h>
#include <iostream>


class Skeleton {
public:
	Bone* rootBone;
	std::vector<Bone*> bones;
	std::vector<glm::mat4> boneTransforms;
	std::map<std::string, unsigned int> boneName2Index;

	glm::mat4 globalInverseTransform;

	float startTime = -1.0f;
	float durationInTicks;
	float ticksPerSecond;
	bool animationActive = false;

	Skeleton();
	void readBones(aiMesh* mesh, aiNode* node);
	void readAnimation(aiAnimation* animation);
	void draw(Shader& shader);
	void runAnimation();
private:
	Bone* createBoneHierarchy(aiNode* node);
	void setFinalJointTransforms();
	void generateGlobalAnimationMatrices(Bone* bone);
};