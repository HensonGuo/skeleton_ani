#pragma once

#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "line_drawer.h"
#include "constants.h"
#include "animation.h"

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
	bool animationActive = false;


	Skeleton();
	~Skeleton();
	void readBones(aiMesh* mesh);
	void setRootInfo(aiNode* rootNode);
	void setAnimation(aiAnimation* animation);
	void draw(Shader& shader);
	void changePose(Shader& shader, DrawType drawType, float ticksElapsed);
	void keepPose(Shader& shader, DrawType drawType);
	void reCalculateTransform(float elapsed);
	bool hasBones();
private:
	void applyPose(Shader& shader, DrawType drawType);
	Bone* createBoneHierarchy(aiNode* node);
	void calculateBoneTransform(Bone* bone, glm::mat4 parentTransform, float delta);
	aiNodeAnim* findNodeAnim(const aiAnimation* ani, const std::string& nodeName);

	LineDrawer transformPoseLineDrawer;
	void updateTransformDrawer(Bone* bone, mat4 currentTransform);

	LineDrawer bindPoseLineDrawer;
	void connectBones(Bone* bone);
};