#pragma once
#include <vector>
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

class Animation {
public:
	int numPostions;
	int numRotations;
	int numScales;
	vector<PositionKeyframe> positions;
	vector<RotationKeyframe> rotations;
	vector<ScaleKeyframe> scales;

	Animation(const aiNodeAnim* channel);
	void update(float delta);
	mat4 getCurrentTransform();
	bool hasAnimaiton();

private:
	mat4 currentTransform;

	int getPositionFrameIndex(float delta);
	int getRotationFrameIndex(float delta);
	int getScaleFrameIndex(float delta);
	float getFactor(float lastFrameStamp, float nextFrameStamp, float delta);
	//插值计算位移、旋转、缩放
	mat4 interpolatePosition(float delta);
	mat4 interpolateRotation(float delta);
	mat4 interpolateScaling(float delta);
};