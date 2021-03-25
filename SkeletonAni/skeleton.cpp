#include "skeleton.h"


Skeleton::Skeleton()
{
}

void Skeleton::readBones(aiMesh* mesh, aiNode* node, aiAnimation* animation)
{
	if (animation)
	{
		ticksPerSecond = (float)animation->mTicksPerSecond;
		durationInTicks = (float)animation->mDuration;
	}

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		aiBone* boneData = mesh->mBones[i];
		std::string name = boneData->mName.C_Str();
		auto channel = animation->mChannels[i];
		Bone* bone = new Bone(name, i, channel);
		boneName2Index.insert(std::pair <std::string, unsigned int>(name, i));
		bone->offset = assimpToGlmMatrix(boneData->mOffsetMatrix);
		bones.push_back(bone);
	}
	rootBone = createBoneHierarchy(node);
	boneTransforms.resize(bones.size());
// 	createBonesVertices(rootBone, aiMatrix4x4());
// 	skeletonLine.setUp();
}

void Skeleton::draw(Shader& shader)
{
	skeletonLine.draw(shader);
}

void Skeleton::changePose(Shader& shader)
{
	if (this->startTime < 0.0f)
	{
		std::cout << "start time set!!!\n";
		startTime = (float)glfwGetTime();
	}

	float timeElapsed = (float)glfwGetTime() - startTime;
	float ticksElapsed = fmod((timeElapsed * ticksPerSecond), durationInTicks);
	calculateBoneTransform(rootBone, glm::mat4(1.0f), ticksElapsed);

	glm::mat4* ptr = boneTransforms.data();
	shader.setMat4("bone_transforms", bones.size(), boneTransforms[0]);
}

Bone* Skeleton::createBoneHierarchy(aiNode* node)
{
	std::string nodeName = node->mName.C_Str();

	if (boneName2Index.count(nodeName) > 0)
	{
		unsigned int id = boneName2Index.at(nodeName);
		Bone* bone = bones[id];
		bone->mTransform = node->mTransformation;
		bone->transformation = assimpToGlmMatrix(node->mTransformation);

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

void Skeleton::calculateBoneTransform(Bone* bone, glm::mat4 parentTransform, float delta)
{
	std::string nodeName = bone->name;
	glm::mat4 nodeTransform = bone->transformation;
	bone->update(delta);
	nodeTransform = bone->localTransform;

	glm::mat4 globalTransformation = parentTransform * nodeTransform;
	boneTransforms[bone->id] = globalTransformation * bone->offset;

	for (int i = 0; i < bone->children.size(); i++)
		calculateBoneTransform(bone->children[i], globalTransformation, delta);
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

		uint startBoneId = boneName2Index.at(bone->name);
		uint endBoneId = boneName2Index.at(parent->name);

		skeletonLine.addBoneLine(start, startBoneId, end, endBoneId, vec4(0, 1, 0, 1));
	}

	for (auto it = bone->children.begin(); it != bone->children.end(); ++it) {

		createBonesVertices(*it, currentTransform);
	}
}
