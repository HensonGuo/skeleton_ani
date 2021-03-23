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
	createBonesVertices(rootBone, aiMatrix4x4());
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
	skeletonLine.draw(shader);
}

void Skeleton::changePose(Shader& shader)
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
		unsigned int id = boneName2Index.at(nodeName);
		Bone* bone = bones[id];
		bone->mTransform = node->mTransformation;

		//set parent
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

void Skeleton::createBonesVertices(Bone* bone, aiMatrix4x4 currentTransform)
{
	aiMatrix4x4 m = bone->mTransform;
	currentTransform = currentTransform * m;
	bone->mTempTransform = currentTransform;

	if (bone->parent) {
		float node_x = currentTransform.a4;
		float node_y = currentTransform.b4;
		float node_z = currentTransform.c4;

		vec3 start = vec3(node_x, node_y, node_z);

		const Bone* parent = bone->parent;
		aiMatrix4x4 parentTransform = parent->mTempTransform;

		float parent_x = parentTransform.a4;
		float parent_y = parentTransform.b4;
		float parent_z = parentTransform.c4;

		vec3 end = vec3(parent_x, parent_y, parent_z);

		skeletonLine.addLine(start, end, vec4(0, 1, 0, 1));
	}

	for (auto it = bone->children.begin(); it != bone->children.end(); ++it) {

		createBonesVertices(*it, currentTransform);
	}
	skeletonLine.setUp();
}
