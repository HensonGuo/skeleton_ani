#include "skeleton.h"


Skeleton::Skeleton()
{
}

void Skeleton::readBones(aiMesh* mesh)
{
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		aiBone* boneData = mesh->mBones[i];
		std::string name = boneData->mName.C_Str();
		Bone* bone = new Bone(name, boneCount);
		boneName2Index.insert(std::pair <std::string, unsigned int>(name, boneCount));
		bone->offset = assimpToGlmMatrix(boneData->mOffsetMatrix);
		bones.push_back(bone);
		boneCount++;
	}
	boneTransforms.resize(bones.size());
	modelTransforms.resize(bones.size());
}

void Skeleton::setRootInfo(aiNode* rootNode)
{
	globalTransform = assimpToGlmMatrix(rootNode->mTransformation);
	rootBone = createBoneHierarchy(rootNode, aiMatrix4x4());
}

void Skeleton::setAnimation(aiAnimation* animation)
{
	if (!animation)
		return;
	ticksPerSecond = (float)animation->mTicksPerSecond;
	durationInTicks = (float)animation->mDuration;
	for (unsigned int i = 0; i < bones.size(); i++)
	{
		Bone* bone = bones[i];
		aiNodeAnim* ani = findNodeAnim(animation, bone->name);
		bone->setAnimation(ani);
	}
}

void Skeleton::draw(Shader& shader)
{
	transformLineDrawer.clear();
	updateTransformDrawer(rootBone, mat4(1.0f));
	transformLineDrawer.setLineWidth(5);
 	transformLineDrawer.draw(shader);

// 	skeletonLineDrawer.setLineWidth(6);
// 	skeletonLineDrawer.draw(shader);
}

void Skeleton::changePose(Shader& shader, DrawType drawType)
{
	if (this->startTime < 0.0f)
	{
		std::cout << "start time set!!!\n";
		startTime = (float)glfwGetTime();
	}

	float timeElapsed = (float)glfwGetTime() - startTime;
	ticksElapsed = fmod((timeElapsed * ticksPerSecond), durationInTicks);
	calculateBoneTransform(rootBone, globalTransform, ticksElapsed);
	applyPose(shader, drawType);
}

void Skeleton::keepPose(Shader& shader, DrawType drawType)
{
	applyPose(shader, drawType);
}

void Skeleton::reCalculateTransform(float elapsed)
{
	ticksElapsed = elapsed;
	calculateBoneTransform(rootBone, globalTransform, elapsed);
}

void Skeleton::applyPose(Shader& shader, DrawType drawType)
{
	if (drawType == DRAW_ENTITY)
	{
		glm::mat4* ptr = boneTransforms.data();
		shader.setMat4("bone_transforms", bones.size(), boneTransforms[0]);
	}
	else if (drawType == DRAW_SKELETON)
	{
		glm::mat4* ptr = modelTransforms.data();
		shader.setMat4("bone_transforms", bones.size(), modelTransforms[0]);
	}
}

Bone* Skeleton::createBoneHierarchy(aiNode* node, aiMatrix4x4 currentTransform)
{
	std::string nodeName = node->mName.C_Str();

	if (boneName2Index.count(nodeName) > 0)
	{
		unsigned int id = boneName2Index.at(nodeName);
		Bone* bone = bones[id];
		bone->localTransform = assimpToGlmMatrix(node->mTransformation);

		currentTransform = currentTransform * node->mTransformation;
		bone->position = vec3(currentTransform.a4, currentTransform.b4, currentTransform.c4);

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
			Bone* child = createBoneHierarchy(node->mChildren[i], currentTransform);
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
			Bone* child = createBoneHierarchy(node->mChildren[i], currentTransform);
			if (child != nullptr)
				toReturn = child;
		}
		return toReturn;
	}
}

void Skeleton::calculateBoneTransform(Bone* bone, glm::mat4 parentTransform, float delta)
{
	std::string nodeName = bone->name;
	glm::mat4 nodeTransform = bone->localTransform;
	if (animationActive)
	{
		bone->update(delta);
		nodeTransform = bone->localTransform;
	}

	glm::mat4 finalTransformation = parentTransform * nodeTransform;
	boneTransforms[bone->id] = finalTransformation * bone->offset;

	//不计算offset即是模型空间，*offset即转换到骨骼空间
	modelTransforms[bone->id] = finalTransformation;

	for (int i = 0; i < bone->children.size(); i++)
		calculateBoneTransform(bone->children[i], finalTransformation, delta);
}

aiNodeAnim* Skeleton::findNodeAnim(const aiAnimation* ani, const std::string& nodeName)
{
	for (uint i = 0; i < ani->mNumChannels; ++i)
	{
		if (strcmp(ani->mChannels[i]->mNodeName.C_Str(), nodeName.c_str()) == 0)
		{
			return ani->mChannels[i];
		}
	}
	return NULL;
}

void Skeleton::updateTransformDrawer(Bone* bone, mat4 currentTransform)
{
	for (int boneIndex = 0; boneIndex < bones.size(); ++boneIndex)
	{
		const Bone* bone = bones[boneIndex];
		const Bone* parent = bone->parent;
		if (parent)
		{
			vec3 start = currentTransform * modelTransforms[bone->id] * vec4(0, 0, 0, 1);
			vec3 end = currentTransform * modelTransforms[parent->id] * vec4(0, 0, 0, 1);
			vec4 color = BoneColor;
			transformLineDrawer.addBoneLine(start, bone->id, end, parent->id, color);
		}
	}
}

void Skeleton::connectBones(Bone* bone)
{
	if (bone->parent) {
		const Bone* parent = bone->parent;
		uint startBoneId = boneName2Index.at(bone->name);
		uint endBoneId = boneName2Index.at(parent->name);

		skeletonLineDrawer.addBoneLine(bone->position, bone->id, parent->position, parent->id, vec4(0, 0, 1, 1));
	}

	for (auto it = bone->children.begin(); it != bone->children.end(); ++it) {

		connectBones(*it);
	}
}
