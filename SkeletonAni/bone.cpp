#include "bone.h"

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
	:name(name),
	id(ID),
	localTransform(1.0f)
{
	if (!channel)
		return;
	keyframeSize = channel->mNumPositionKeys;
	for (int positionIndex = 0; positionIndex < keyframeSize; ++positionIndex)
	{
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		PositionKeyframe data;
		data.position = assimpToGlmVec3(aiPosition);
		data.timeStamp = timeStamp;
		positions.push_back(data);
	}

	for (int rotationIndex = 0; rotationIndex < keyframeSize; ++rotationIndex)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		RotationKeyframe data;
		data.orientation = assimpToGlmQuat(aiOrientation);
		data.timeStamp = timeStamp;
		rotations.push_back(data);
	}

	for (int keyIndex = 0; keyIndex < keyframeSize; ++keyIndex)
	{
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		ScaleKeyframe data;
		data.scale = assimpToGlmVec3(scale);
		data.timeStamp = timeStamp;
		scales.push_back(data);
	}
}

void Bone::update(float delta)
{
	int frameIndex = getKeyFrameIndex(delta);
	float factor = getFactor(rotations[frameIndex].timeStamp,
		rotations[frameIndex + 1].timeStamp, delta);

	glm::mat4 translation = interpolatePosition(frameIndex, factor);
	glm::mat4 rotation = interpolateRotation(frameIndex, factor);
	glm::mat4 scale = interpolateScaling(frameIndex, factor);
	localTransform = translation * rotation * scale;
}

int Bone::getKeyFrameIndex(float delta)
{
	for (int index = 0; index < keyframeSize - 1; ++index)
	{
		if (delta < positions[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

float Bone::getFactor(float lastFrameStamp, float nextFrameStamp, float delta)
{
	float factor = 0.0f;
	float midWayLength = delta - lastFrameStamp;
	float framesDiff = nextFrameStamp - lastFrameStamp;
	factor = midWayLength / framesDiff;
	return factor;
}

glm::mat4 Bone::interpolatePosition(int frameIndex, float factor)
{
	if (keyframeSize == 1)
		return glm::translate(glm::mat4(1.0f), positions[0].position);

	int p0Index = frameIndex;
	int p1Index = p0Index + 1;
	glm::vec3 finalPosition = glm::mix(positions[p0Index].position,
		positions[p1Index].position
		, factor);
	return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(int frameIndex, float factor)
{
	if (keyframeSize == 1)
	{
		auto rotation = glm::normalize(rotations[0].orientation);
		return glm::toMat4(rotation);
	}
	int p0Index = frameIndex;
	int p1Index = p0Index + 1;
	glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation,
		rotations[p1Index].orientation, factor);
	finalRotation = glm::normalize(finalRotation);
	return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScaling(int frameIndex, float factor)
{
	if (keyframeSize == 1)
		return glm::scale(glm::mat4(1.0f), scales[0].scale);

	int p0Index = frameIndex;
	int p1Index = p0Index + 1;
	glm::vec3 finalScale = glm::mix(scales[p0Index].scale,
		scales[p1Index].scale, factor);
	return glm::scale(glm::mat4(1.0f), finalScale);
}
