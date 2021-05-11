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
	skeleton->setRootInfo(rootNode, nodeName2LocalTransform);
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
	if (skeleton->hasBones())
	{
		shader.setBool("hasSkeleton", true);
		if (skeleton->animationActive)
		{
			if (this->startTime < 0.0f)
			{
				std::cout << "start time set!!!\n";
				startTime = (float)glfwGetTime();
			}
			float timeElapsed = (float)glfwGetTime() - startTime;
			ticksElapsed = fmod((timeElapsed * ticksPerSecond), durationInTicks);

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
		//�ڵ������������������������е�ʵ�ʶ���
		//�������������е�����
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
	nodeName2LocalTransform.insert(pair<string, mat4>(node->mName.C_Str(), assimpToGlmMatrix(currentTransform)));
	nodeIndex2Name.insert(pair<int, string>(nodeCount, node->mName.C_Str()));
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
	skeleton->setAnimation(animation);
}

void Model::showNodeName(aiNode* node)
{
	cout << node->mName.data << endl;
	for (uint i = 0; i < node->mNumChildren; i++) {
		showNodeName(node->mChildren[i]);
	}
}

void Model::applyNodeTransform(Shader& shader)
{
	nodeTransforms.resize(nodeCount);
	for (uint i = 0; i < nodeCount; i++)
	{
		uint index = i;
		string nodeName = nodeIndex2Name.at(index);
		mat4 trans = nodeName2LocalTransform.at(nodeName);
		nodeTransforms[i] = trans;
	}
	shader.setMat4("node_transforms", nodeTransforms.size(), nodeTransforms[0]);
}
