#include "skeleton.h"


Skeleton::Skeleton()
{
	
}

Skeleton::~Skeleton()
{
	for (auto it = bones.begin(); it != bones.end(); it++)
	{
		delete *it;
		if (*it == rootBone)
			rootBone = NULL;
		*it = NULL;
	}
	bones.clear();
}

void Skeleton::readBones(aiMesh* mesh)
{
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		aiBone* boneData = mesh->mBones[i];
		std::string name = boneData->mName.C_Str();
		if (boneName2Index.find(name) != boneName2Index.end())
			continue;
		Bone* bone = new Bone(name, boneCount);
		boneName2Index.insert(std::pair <std::string, unsigned int>(name, boneCount));
		bone->offset = assimpToGlmMatrix(boneData->mOffsetMatrix);
		bones.push_back(bone);
		boneCount++;
	}
	bone2MeshTransforms.resize(bones.size());
	boneTransforms.resize(bones.size());
}

void Skeleton::setRootInfo(aiNode* rootNode, const map<string, mat4>&nodeName2LocalTransform)
{
	this->nodeName2LocalTransform = nodeName2LocalTransform;
	globalTransform = assimpToGlmMatrix(rootNode->mTransformation);
	rootBone = createBoneHierarchy(rootNode);
	//connectBones(rootBone);
}

void Skeleton::setAnimation(aiAnimation* animation)
{
	if (!animation)
		return;
	for (unsigned int i = 0; i < bones.size(); i++)
	{
		Bone* bone = bones[i];
		aiNodeAnim* aniNode = findNodeAnim(animation, bone->name);
		Animation* ani = new Animation(aniNode);
		bone->clear();
		bone->setAnimation(ani);
	}
}

void Skeleton::draw(Shader& shader)
{
	transformPoseLineDrawer.clear();
	updateTransformDrawer(rootBone, mat4(1.0f));
	transformPoseLineDrawer.setLineWidth(5);
 	transformPoseLineDrawer.draw(shader);

// 	bindPoseLineDrawer.setLineWidth(6);
// 	bindPoseLineDrawer.draw(shader);
}

void Skeleton::changePose(Shader& shader, DrawType drawType, float ticksElapsed)
{
	cout << ticksElapsed;
	calculateBoneTransform(rootBone, globalTransform, ticksElapsed);
	applyPose(shader, drawType);
}

void Skeleton::keepPose(Shader& shader, DrawType drawType)
{
	applyPose(shader, drawType);
}

void Skeleton::reCalculateTransform(float elapsed)
{
	if (rootBone)
		calculateBoneTransform(rootBone, globalTransform, elapsed);
}

bool Skeleton::hasBones()
{
	return boneCount > 0;
}

void Skeleton::applyPose(Shader& shader, DrawType drawType)
{
	if (drawType == DRAW_ENTITY)
	{
		glm::mat4* ptr = bone2MeshTransforms.data();
		shader.setMat4("bone_transforms", bones.size(), bone2MeshTransforms[0]);
	}
	else if (drawType == DRAW_SKELETON)
	{
		glm::mat4* ptr = boneTransforms.data();
		shader.setMat4("bone_transforms", bones.size(), boneTransforms[0]);
	}
}

Bone* Skeleton::createBoneHierarchy(aiNode* node)
{
	std::string nodeName = node->mName.C_Str();

	if (boneName2Index.count(nodeName) > 0)
	{
		unsigned int id = boneName2Index.at(nodeName);
		Bone* bone = bones[id];
		bone->localTransform = assimpToGlmMatrix(node->mTransformation);

		mat4 currentTransform = nodeName2LocalTransform.at(node->mName.C_Str());

		bone->position = currentTransform * vec4(0, 0, 0, 1);

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
	glm::mat4 nodeTransform = bone->getTransform(delta);
	glm::mat4 finalTransformation = parentTransform * nodeTransform;
	//�����ռ�
	boneTransforms[bone->id] = finalTransformation;
	//����offset�����任������ռ�
	bone2MeshTransforms[bone->id] = finalTransformation * bone->offset;

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
			vec3 start = currentTransform * boneTransforms[bone->id] * vec4(0, 0, 0, 1);
			vec3 end = currentTransform * boneTransforms[parent->id] * vec4(0, 0, 0, 1);
			vec4 color = BoneColor;
			transformPoseLineDrawer.addBoneLine(start, bone->id, end, parent->id, color);
		}
	}
}

void Skeleton::connectBones(Bone* bone)
{
	if (bone->parent) {
		const Bone* parent = bone->parent;
		uint startBoneId = boneName2Index.at(bone->name);
		uint endBoneId = boneName2Index.at(parent->name);

		bindPoseLineDrawer.addBoneLine(bone->position, bone->id, parent->position, parent->id, vec4(0, 0, 1, 1));
	}

	for (auto it = bone->children.begin(); it != bone->children.end(); ++it) {

		connectBones(*it);
	}
}
