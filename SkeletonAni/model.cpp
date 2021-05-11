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
	for (auto it = nodeName2Info.begin(); it != nodeName2Info.end(); it++)
	{
		delete it->second;
		it->second = NULL;
	}
	nodeName2Info.clear();
	rootNode = NULL;
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

	aiNode* aiRootNode = scene->mRootNode;
	showNodeName(aiRootNode);
	rootNode = processNode(aiRootNode, scene, aiMatrix4x4());
	skeleton->setRootInfo(aiRootNode);
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
		nodeTransforms.resize(nodeCount);
		this->calculateNodeTransform(rootNode, mat4(), ticksElapsed);
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

NodeInfo* Model::processNode(aiNode* node, const aiScene* scene, aiMatrix4x4 currentTransform)
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
	nodeInfo->id = nodeCount;
	nodeInfo->name = node->mName.C_Str();
	nodeInfo->localTransform = assimpToGlmMatrix(currentTransform);
	nodeName2Info.insert(pair<string, NodeInfo*>(nodeInfo->name, nodeInfo));
	nodeCount += 1;

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		NodeInfo *subNode = processNode(node->mChildren[i], scene, currentTransform);
		subNode->parent = nodeInfo;
		nodeInfo->children.push_back(subNode);
	}
	return nodeInfo;
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
			NodeInfo* nodeInfo = nodeName2Info.at(aniNode->mNodeName.C_Str());
			nodeInfo->ani = ani;
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

void Model::calculateNodeTransform(NodeInfo* node, glm::mat4 parentTransform, float delta)
{
	glm::mat4 nodeTransform = node->getTransform(delta);
	glm::mat4 finalTransformation = parentTransform * nodeTransform;
	nodeTransforms[node->id] = finalTransformation;

	for (int i = 0; i < node->children.size(); i++)
		calculateNodeTransform(node->children[i], finalTransformation, delta);
}

void Model::applyNodeTransform(Shader& shader)
{
	shader.setMat4("node_transforms", nodeTransforms.size(), nodeTransforms[0]);
}