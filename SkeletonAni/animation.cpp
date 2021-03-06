#include "animation.h"

Animation::Animation(const aiNodeAnim* channel):numPostions(0), numRotations(0), numScales(0)
{
	if (!channel)
		return;
	numPostions = channel->mNumPositionKeys;
	numRotations = channel->mNumRotationKeys;
	numScales = channel->mNumScalingKeys;
	for (int positionIndex = 0; positionIndex < numPostions; ++positionIndex)
	{
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		PositionKeyframe data;
		data.position = assimpToGlmVec3(aiPosition);
		data.timeStamp = timeStamp;
		positions.push_back(data);
	}

	for (int rotationIndex = 0; rotationIndex < numRotations; ++rotationIndex)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		RotationKeyframe data;
		data.orientation = assimpToGlmQuat(aiOrientation);
		data.timeStamp = timeStamp;
		rotations.push_back(data);
	}

	for (int keyIndex = 0; keyIndex < numScales; ++keyIndex)
	{
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		ScaleKeyframe data;
		data.scale = assimpToGlmVec3(scale);
		data.timeStamp = timeStamp;
		scales.push_back(data);
	}
}

void Animation::update(float delta)
{
	glm::mat4 translation = interpolatePosition(delta);
	glm::mat4 rotation = interpolateRotation(delta);
	glm::mat4 scale = interpolateScaling(delta);
	currentTransform = translation * rotation * scale;
}

mat4 Animation::getCurrentTransform()
{
	return currentTransform;
}

bool Animation::hasAnimaiton()
{
	return numPostions > 0 || numRotations > 0 || numScales > 0;
}

int Animation::getPositionFrameIndex(float delta)
{
	for (int index = 0; index < numPostions - 1; ++index)
	{
		if (delta < positions[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

int Animation::getRotationFrameIndex(float delta)
{
	for (int index = 0; index < numRotations - 1; ++index)
	{
		if (delta < rotations[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

int Animation::getScaleFrameIndex(float delta)
{
	for (int index = 0; index < numScales - 1; ++index)
	{
		if (delta < scales[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

float Animation::getFactor(float lastFrameStamp, float nextFrameStamp, float delta)
{
	float factor = 0.0f;
	float midWayLength = delta - lastFrameStamp;
	float framesDiff = nextFrameStamp - lastFrameStamp;
	factor = midWayLength / framesDiff;
	return factor;
}

glm::mat4 Animation::interpolatePosition(float delta)
{
	if (numPostions == 1)
		return glm::translate(glm::mat4(1.0f), positions[0].position);

	int frameIndex = getPositionFrameIndex(delta);
	float factor = getFactor(positions[frameIndex].timeStamp,
		positions[frameIndex + 1].timeStamp, delta);

	int p0Index = frameIndex;
	int p1Index = p0Index + 1;
	glm::vec3 finalPosition = glm::mix(positions[p0Index].position,
		positions[p1Index].position
		, factor);
	return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Animation::interpolateRotation(float delta)
{
	if (numRotations == 1)
	{
		auto rotation = glm::normalize(rotations[0].orientation);
		return glm::toMat4(rotation);
	}
	int frameIndex = getRotationFrameIndex(delta);
	float factor = getFactor(rotations[frameIndex].timeStamp,
		rotations[frameIndex + 1].timeStamp, delta);

	int p0Index = frameIndex;
	int p1Index = p0Index + 1;
	glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation,
		rotations[p1Index].orientation, factor);
	finalRotation = glm::normalize(finalRotation);
	return glm::toMat4(finalRotation);
}

glm::mat4 Animation::interpolateScaling(float delta)
{
	if (numScales == 1)
		return glm::scale(glm::mat4(1.0f), scales[0].scale);

	int frameIndex = getScaleFrameIndex(delta);
	float factor = getFactor(scales[frameIndex].timeStamp,
		scales[frameIndex + 1].timeStamp, delta);

	int p0Index = frameIndex;
	int p1Index = p0Index + 1;
	glm::vec3 finalScale = glm::mix(scales[p0Index].scale,
		scales[p1Index].scale, factor);
	return glm::scale(glm::mat4(1.0f), finalScale);
}