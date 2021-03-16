#include "bone.h"

Keyframe::Keyframe()
{
}

Keyframe::Keyframe(vec3 translationV, vec3 scalingV, aiQuaternion quatV, float timeInTicks)
{
	this->translationV = translationV;
	this->scalingV = scalingV;
	this->quatV = quatV;
	this->timeInTicks = timeInTicks;
}

Keyframe& Keyframe::operator=(const Keyframe& k)
{
	translationV = k.translationV;
	scalingV = k.scalingV;
	quatV = k.quatV;
	timeInTicks = k.timeInTicks;
	return *this;
}

glm::mat4 Bone::getCurrentTransform(float ticksElapsed)
{
	unsigned int index = 0;

	for (index = 0; index < keyframes.size() - 1; index++)
	{
		if (ticksElapsed <= keyframes[index + 1]->timeInTicks)
		{
			break;
		}
	}

	Keyframe* currFrame, * nextFrame;
	currFrame = keyframes[index];
	nextFrame = keyframes[fmod(index + 1, keyframes.size())];

	float deltaTime = abs(nextFrame->timeInTicks - currFrame->timeInTicks);

	float factor = (ticksElapsed - currFrame->timeInTicks) / deltaTime;		//between 0.0 and 1.0

	if (factor < 0.0f || factor > 1.0f)
		std::cout << "Error! Factor should be in [0.0, 1.0]\n";

	//translation
	glm::vec3 deltaT = nextFrame->translationV - currFrame->translationV;
	glm::vec3 newTranslationV = currFrame->translationV + factor * deltaT;
	glm::mat4 translationM = glm::translate(glm::mat4(1.0f), newTranslationV);

	//rotation
	aiQuaternion newQuat;
	const aiQuaternion& currentQuat = currFrame->quatV;
	const aiQuaternion& nextQuat = nextFrame->quatV;
	aiQuaternion::Interpolate(newQuat, currentQuat, nextQuat, factor);

	glm::quat newGLMQuat = assimpToGlmQuat(newQuat);
	glm::mat4 rotationM = glm::toMat4(newGLMQuat);

	//scaling
	glm::vec3 deltaS = nextFrame->scalingV - currFrame->scalingV;
	glm::vec3 newScalingV = currFrame->scalingV + factor * deltaS;
	glm::mat4 scalingM = glm::scale(glm::mat4(1.0f), newScalingV);


	return translationM * rotationM * scalingM;
}
