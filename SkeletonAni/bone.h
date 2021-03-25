#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/scene.h>

#include "utils.h"


using namespace std;
using namespace glm;


struct PositionKeyframe
{
	glm::vec3 position;
	float timeStamp;
};

struct RotationKeyframe
{
	glm::quat orientation;
	float timeStamp;
};

struct ScaleKeyframe
{
	glm::vec3 scale;
	float timeStamp;
};



// 骨骼
class Bone {
public:
	int id = 0;
	string name = "";
	mat4 localTransform;
	/*偏移矩阵将顶点从模型空间转换为骨骼空间*/
	mat4 offset;
	mat4 transformation;

	Bone* parent;
	vector<Bone*> children = {};

	aiMatrix4x4 mTransform;
	aiMatrix4x4 mTempTransform;

	Bone(const string& name, int ID, const aiNodeAnim* channel);
	void update(float delta);

private:
	int keyframeSize;
	vector<PositionKeyframe> positions;
	vector<RotationKeyframe> rotations;
	vector<ScaleKeyframe> scales;

	int getKeyFrameIndex(float delta);
	float getFactor(float lastFrameStamp, float nextFrameStamp, float delta);
	//插值计算位移、旋转、缩放
	mat4 interpolatePosition(int frameIndex, float factor);
	mat4 interpolateRotation(int frameIndex, float factor);
	mat4 interpolateScaling(int frameIndex, float factor);
};