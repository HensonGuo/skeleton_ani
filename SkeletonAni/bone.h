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
	mat4 localTransform;
	/*ƫ�ƾ��󽫶����ģ�Ϳռ�ת��Ϊ�����ռ�*/
	mat4 offset;
	vec3 position;

	Bone* parent;
	vector<Bone*> children = {};

	Bone(const string& name, int ID, const aiNodeAnim* channel);
	void update(float delta);

private:
	int keyframeSize;
	vector<PositionKeyframe> positions;
	vector<RotationKeyframe> rotations;
	vector<ScaleKeyframe> scales;

	int getKeyFrameIndex(float delta);
	float getFactor(float lastFrameStamp, float nextFrameStamp, float delta);
	//��ֵ����λ�ơ���ת������
	mat4 interpolatePosition(int frameIndex, float factor);
	mat4 interpolateRotation(int frameIndex, float factor);
	mat4 interpolateScaling(int frameIndex, float factor);
};