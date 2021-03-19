#include "skeleton.h"

Skeleton::Skeleton()
{
}

void Skeleton::readBones(aiMesh* mesh, aiNode* node)
{
	glm::mat4 transform = assimpToGlmMatrix(node->mTransformation);
	globalInverseTransform = glm::inverse(transform);

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		Bone* bone = new Bone();
		aiBone* boneData = mesh->mBones[i];
		std::string name = boneData->mName.C_Str();

		boneName2Index.insert(std::pair <std::string, unsigned int>(name, i));

		bone->invBindPoseM = assimpToGlmMatrix(boneData->mOffsetMatrix);
		bone->id = i;
		bone->name = name;

		bones.push_back(bone);
	}
	rootBone = createBoneHierarchy(node);
}

void Skeleton::readAnimation(aiAnimation* animation)
{
	ticksPerSecond = (float)animation->mTicksPerSecond;
	durationInTicks = (float)animation->mDuration;

	for (unsigned int i = 0; i < animation->mNumChannels; i++)
	{
		aiNodeAnim* boneAnimation = animation->mChannels[i];
		std::string jointName = boneAnimation->mNodeName.C_Str();
		unsigned int boneId = boneName2Index.at(jointName);

		int keyframeCnt = boneAnimation->mNumPositionKeys;
		for (unsigned int j = 0; j < keyframeCnt; j++)
		{
			float timeInTicks = (float)boneAnimation->mPositionKeys[j].mTime;

			glm::vec3 translationVect = assimpToGlmVec3(boneAnimation->mPositionKeys[j].mValue);
			glm::vec3 scaleVect = assimpToGlmVec3(boneAnimation->mScalingKeys[j].mValue);
			aiQuaternion rotationQuat = boneAnimation->mRotationKeys[j].mValue;

			Keyframe* currKeyframe = new Keyframe(translationVect, scaleVect, rotationQuat, timeInTicks);
			bones[boneId]->keyframes.push_back(currKeyframe);
		}
	}
}

void Skeleton::draw(Shader& shader)
{
	boneTransforms.resize(bones.size());
	if (this->startTime < 0.0f)
	{
		std::cout << "start time set!!!\n";
		startTime = (float)glfwGetTime();
	}

	if (!animationActive)
	{
		for (unsigned int i = 0; i < bones.size(); i++)
			boneTransforms[i] = globalInverseTransform;
	}
	else
		runAnimation();

	glm::mat4* ptr = boneTransforms.data();
	shader.setMat4("bone_transforms", bones.size(), boneTransforms[0]);
}

void Skeleton::runAnimation()
{
	float timeElapsed = (float)glfwGetTime() - startTime;
	float ticksElapsed = fmod((timeElapsed * ticksPerSecond), durationInTicks);

	for (unsigned int i = 0; i < bones.size(); i++)
	{
		glm::mat4 modelM = bones[i]->getCurrentTransform(ticksElapsed);
		bones[i]->localAnimationM = modelM;
	}

	generateGlobalAnimationMatrices(rootBone);

	boneTransforms.resize(bones.size());
	setFinalBoneTransforms();
}

Bone* Skeleton::createBoneHierarchy(aiNode* node)
{
	std::string nodeName = node->mName.C_Str();

	if (boneName2Index.count(nodeName) > 0)
	{
		//add node to hierarchy
		unsigned int id = boneName2Index.at(nodeName);
		Bone* bone = bones[id];;

		//set the parent for the joint
		if (node->mParent == NULL)
			bone->parent = nullptr;
		else
		{
			std::string parentName = node->mParent->mName.C_Str();
			if (boneName2Index.count(parentName) > 0)
			{
				unsigned int parentId = boneName2Index.at(parentName);
				bone->parent = bones[parentId];
			}
			else
				bone->parent = nullptr;
		}

		//set children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			Bone* child = createBoneHierarchy(node->mChildren[i]);
			if (child != nullptr)
				bone->children.push_back(child);
		}

		return bone;

	}
	else
	{
		//do not add node to hierarchy
		Bone* toReturn = nullptr;

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			Bone* child = createBoneHierarchy(node->mChildren[i]);
			if (child != nullptr)
				toReturn = child;
		}
		return toReturn;
	}
}

void Skeleton::setFinalBoneTransforms()
{
	for (unsigned int i = 0; i < bones.size(); i++)
	{
		glm::mat4 invBindPoseM = bones[i]->invBindPoseM;
		glm::mat4 globalAnimationM = bones[i]->globalAnimationM;
		glm::mat4 finalM = globalInverseTransform * globalAnimationM * glm::transpose(invBindPoseM);

		boneTransforms[i] = finalM;
	}
}

void Skeleton::generateGlobalAnimationMatrices(Bone* bone)
{
	//根关节的全局动画矩阵是其局部动画矩阵
	if (bone->parent == nullptr)
		bone->globalAnimationM = bone->localAnimationM;
	else
	{
		glm::mat4 globalAnimationM = bone->parent->globalAnimationM * bone->localAnimationM;
		bone->globalAnimationM = globalAnimationM;

	}

	unsigned int childCnt = bone->children.size();
	for (unsigned int i = 0; i < childCnt; i++)
	{
		generateGlobalAnimationMatrices(bone->children[i]);
	}
}
