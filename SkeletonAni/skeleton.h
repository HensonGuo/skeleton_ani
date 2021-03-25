#pragma once

#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "line_drawer.h"

#include <map>
#include <GLFW/glfw3.h>
#include <iostream>


class Skeleton {
public:
	Bone* rootBone;
	std::vector<Bone*> bones;
	std::vector<glm::mat4> boneTransforms;
	std::map<std::string, unsigned int> boneName2Index;

	float startTime = -1.0f;
	float durationInTicks;
	float ticksPerSecond;
	bool animationActive = true;

	Skeleton();
	void readBones(aiMesh* mesh, aiNode* node, aiAnimation* animation);
	void draw(Shader& shader);
	void changePose(Shader& shader);
private:
	Bone* createBoneHierarchy(aiNode* node, aiMatrix4x4 currentTransform);
	void calculateBoneTransform(Bone* bone, glm::mat4 parentTransform, float delta);

	LineDrawer transformLineDrawer;
	void updateTransformDrawer(Bone* bone, mat4 currentTransform);

	LineDrawer skeletonLineDrawer;
	void connectBones(Bone* bone);
};