#include "model.h"

Model::Model()
{
}

Model::Model(const string& path)
{
	this->loadModel(path);
}

void Model::clear()
{
	delete this->skeleton;
	this->skeleton = NULL;
	for (auto it = meshes.begin(); it != meshes.end(); it++)
	{
		delete* it;
		*it = NULL;
	}
	this->meshes.clear();
	nodeTransforms.clear();
	nodeIndex2Info.clear();
}

void Model::loadModel(const string& path)
{
	this->skeleton = new Skeleton();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	cout << " \n------------------------------- Details Info-------------------------------------------- \n " << endl;
	cout << "              Animations: " << scene->HasAnimations() << endl;
	cout << "                  Meshes: " << scene->mNumMeshes << endl;
	if (scene->HasAnimations())
	{
		cout << "              Bone Count: " << scene->mAnimations[0]->mNumChannels << endl;
		cout << "      Animation Duration: " << scene->mAnimations[0]->mDuration << endl;
		cout << "Animation TicksPerSecond: " << scene->mAnimations[0]->mTicksPerSecond << endl << endl;
	}

	aiNode* rootNode = scene->mRootNode;
	showNodeName(rootNode);
	processNode(rootNode, scene, aiMatrix4x4());
	skeleton->setRootInfo(rootNode);
	if (scene->HasAnimations() == true)
		setAnimation(scene->mAnimations[0]);
}

void Model::loadAnimation(const string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);
	auto animation = scene->mAnimations[0];
	setAnimation(animation);
}

void Model::draw(Shader& shader, DrawType drawType)
{
	if (this->startTime < 0.0f)
	{
		std::cout << "start time set!!!\n";
		startTime = (float)glfwGetTime();
	}
	float timeElapsed = (float)glfwGetTime() - startTime;
	ticksElapsed = fmod((timeElapsed * ticksPerSecond), durationInTicks);

	if (skeleton->hasBones())
	{
		shader.setBool("hasSkeleton", true);
		if (skeleton->animationActive)
		{
			skeleton->changePose(shader, drawType, ticksElapsed);
		}
		else
			skeleton->keepPose(shader, drawType);
	}
	else
	{
		shader.setBool("hasSkeleton", false);
		this->applyNodeTransform(shader);
	}
	
	if (drawType == DRAW_ENTITY)
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			Mesh* mesh = meshes[i];
			mesh->draw(shader);
		}
	}
	else if (drawType == DRAW_SKELETON)
		skeleton->draw(shader);
}

void Model::playAnimation(bool active)
{
	this->ticksElapsed = 0;
	this->skeleton->animationActive = active;
	this->skeleton->reCalculateTransform(0);
}

bool Model::isPlayingAnimation()
{
	return this->skeleton->animationActive;
}

float Model::getAniDuration()
{
	return this->durationInTicks;
}

float Model::getAniElapsed()
{
	return this->ticksElapsed;
}

void Model::changePoseStopAtTime(float delta)
{
	this->playAnimation(false);
	this->ticksElapsed = delta;
	this->skeleton->reCalculateTransform(delta);
}

uint Model::getBonesCount()
{
	return this->skeleton->boneCount;
}

uint Model::getVertexCount()
{
	return this->vertexCount;
}

void Model::processNode(aiNode* node, const aiScene* scene, aiMatrix4x4 currentTransform)
{
	vertexCount = 0;
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		//节点对象仅包含索引以索引场景中的实际对象。
		//场景包含了所有的数据
		aiMesh* meshData = scene->mMeshes[node->mMeshes[i]];
		skeleton->readBones(meshData);
		Mesh* mesh = new Mesh();
		mesh->readVertices(meshData, nodeCount);
		mesh->readIndices(meshData);
		mesh->readMaterials(scene, meshData, directory);
		mesh->setVerticesWeights(meshData, skeleton);
		mesh->normalizeBonesWeight();
		mesh->setup();
		meshes.push_back(mesh);
		vertexCount += mesh->getVertextCount();
	}
	currentTransform = currentTransform * node->mTransformation;
	NodeInfo *nodeInfo = new NodeInfo();
	nodeInfo->name = node->mName.C_Str();
	nodeInfo->localTransform = assimpToGlmMatrix(currentTransform);
	nodeIndex2Info.insert(pair<int, NodeInfo*>(nodeCount, nodeInfo));

	nodeCount += 1;
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, currentTransform);
	}
}

void Model::setAnimation(aiAnimation* animation)
{
	ticksPerSecond = (float)animation->mTicksPerSecond;
	durationInTicks = (float)animation->mDuration;
	if (skeleton->hasBones())
		skeleton->setAnimation(animation);
	else
	{
		for (uint i = 0; i < animation->mNumChannels; i++)
		{
			aiNodeAnim* aniNode = animation->mChannels[i];
			Animation* ani = new Animation(aniNode);
			setNodeAnimation(aniNode->mNodeName.C_Str(), ani);
		}
	}
}

void Model::showNodeName(aiNode* node)
{
	cout << node->mName.data << endl;
	for (uint i = 0; i < node->mNumChildren; i++) {
		showNodeName(node->mChildren[i]);
	}
}

void Model::setNodeAnimation(string nodeName, Animation* ani)
{
	for (auto it = nodeIndex2Info.begin(); it != nodeIndex2Info.end(); it++)
	{
		if (strcmp(it->second->name.c_str(), nodeName.c_str()) == 0)
		{
			it->second->ani = ani;
		}
	}
}

void Model::applyNodeTransform(Shader& shader)
{
	nodeTransforms.resize(nodeCount);
	for (uint i = 0; i < nodeCount; i++)
	{
		uint index = i;
		NodeInfo* nodeInfo = nodeIndex2Info.at(index);
		mat4 trans;
		if (nodeInfo->ani)
		{
			nodeInfo->ani->update(ticksElapsed);
			trans = nodeInfo->ani->getCurrentTransform();
		}
		else
		{
			trans = nodeInfo->localTransform;
		}
		nodeTransforms[i] = trans;
	}
	shader.setMat4("node_transforms", nodeTransforms.size(), nodeTransforms[0]);
}