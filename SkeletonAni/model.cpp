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
	processNode(rootNode, scene);
	processAnimation(scene);
	skeleton->setRootInfo(rootNode);
}

void Model::loadAnimation(const string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);
	auto animation = scene->mAnimations[0];
	this->skeleton->setAnimation(animation);
}

void Model::draw(Shader& shader, DrawType drawType)
{
	if (skeleton->animationActive)
		skeleton->changePose(shader, drawType);
	else
		skeleton->keepPose(shader, drawType);

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
	this->skeleton->animationActive = active;
	this->skeleton->reCalculateTransform(0);
}

bool Model::isPlayingAnimation()
{
	return this->skeleton->animationActive;
}

float Model::getAniDuration()
{
	return this->skeleton->durationInTicks;
}

float Model::getAniElapsed()
{
	return this->skeleton->ticksElapsed;
}

void Model::changePoseStopAtTime(float delta)
{
	this->playAnimation(false);
	this->skeleton->reCalculateTransform(delta);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		//节点对象仅包含索引以索引场景中的实际对象。
		//场景包含了所有的数据
		aiMesh* meshData = scene->mMeshes[node->mMeshes[i]];
		skeleton->readBones(meshData);
		Mesh* mesh = new Mesh();
		mesh->readVertices(meshData);
		mesh->readIndices(meshData);
		mesh->readMaterials(scene, meshData, directory);
		mesh->setVerticesWeights(meshData, skeleton);
		mesh->normalizeBonesWeight();
		mesh->setup();
		meshes.push_back(mesh);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

void Model::processAnimation(const aiScene* scene)
{
	if (scene->HasAnimations() == true)
		skeleton->setAnimation(scene->mAnimations[0]);
}

void Model::showNodeName(aiNode* node)
{
	cout << node->mName.data << endl;
	for (uint i = 0; i < node->mNumChildren; i++) {
		showNodeName(node->mChildren[i]);
	}
}