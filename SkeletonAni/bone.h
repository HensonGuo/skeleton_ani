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



// ����
class Bone {
public:
	int id = 0;
	string name = "";
	glm::mat4 localTransform;
	/*ƫ�ƾ��󽫶����ģ�Ϳռ�ת��Ϊ�����ռ�*/
	glm::mat4 offset;
	glm::mat4 transformation;

	Bone* parent;
	vector<Bone*> children = {};

	aiMatrix4x4 mTransform;
	aiMatrix4x4 mTempTransform;

	Bone(const std::string& name, int ID, const aiNodeAnim* channel);
	void update(float delta);

private:
	int keyframeSize;
	std::vector<PositionKeyframe> positions;
	std::vector<RotationKeyframe> rotations;
	std::vector<ScaleKeyframe> scales;

	int getKeyFrameIndex(float delta);
	float getFactor(float lastFrameStamp, float nextFrameStamp, float delta);
	//��ֵ����λ�ơ���ת������
	glm::mat4 interpolatePosition(int frameIndex, float factor);
	glm::mat4 interpolateRotation(int frameIndex, float factor);
	glm::mat4 interpolateScaling(int frameIndex, float factor);
};