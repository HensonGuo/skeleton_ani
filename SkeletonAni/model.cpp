#include "model.h"

Model::Model(const string& path)
{
	this->skeleton = new Skeleton();
	this->loadModel(path);
}

void Model::loadModel(const string& path)
{
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

	aiMesh* aimesh = scene->mMeshes[0];
	aiNode* rootNode = scene->mRootNode;
	showNodeName(rootNode);
	readVertices(aimesh);
	readIndices(aimesh);
	readSkeleton(scene, aimesh, rootNode);

	if (aimesh->mMaterialIndex >= 0)
	{
		readMaterial(scene->mMaterials[aimesh->mMaterialIndex]);
	}

	setVerticesWeights(aimesh);
	normalizeBonesWeight();
	mesh = Mesh(vertices, indices, materials);
}

void Model::draw(Shader& shader)
{
	mesh.draw(shader);
	skeleton->draw(shader);
}

void Model::playAnimation(bool active)
{
	this->skeleton->animationActive = active;
}


void Model::readVertices(aiMesh* aimesh)
{
	vertices = {};
	for (unsigned int i = 0; i < aimesh->mNumVertices; i++) {
		//λ��
		Vertex vertex;
		vertex.position = assimpToGlmVec3(aimesh->mVertices[i]);
		//����
		if (aimesh->HasNormals()) {
			vertex.normal = assimpToGlmVec3(aimesh->mNormals[i]);
		}
		else
		{
			vertex.normal = vec3();
		}
		//����
		vec2 vec;
		if (aimesh->HasTextureCoords(0))
		{
			//һ�����������԰���8����ͬ���������ꡣ�Ƕ�����������ģ�ͣ�ȡ��һ�飨0��
			vec.x = aimesh->mTextureCoords[0][i].x;
			vec.y = aimesh->mTextureCoords[0][i].y;
		}
		vertex.texCoords = vec;
		vertices.push_back(vertex);
	}
}

void Model::readIndices(aiMesh* aimesh)
{
	indices = {};
	for (unsigned int i = 0; i < aimesh->mNumFaces; i++) {
		aiFace& face = aimesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
}

void Model::setVerticesWeights(aiMesh* aimesh)
{
	vector<uint> boneCounts;
	boneCounts.resize(vertices.size(), 0);
	uint boneCount = aimesh->mNumBones;

	//ѭ��ÿ������
	for (uint i = 0; i < boneCount; i++) {
		aiBone* bone = aimesh->mBones[i];
		uint boneId = skeleton->boneName2Index.at(bone->mName.C_Str());
		//ѭ��ÿ������
		for (uint j = 0; j < bone->mNumWeights; j++) {
			uint id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			boneCounts[id]++;
			switch (boneCounts[id]) {
			case 1:
				vertices[id].boneIds.x = i;
				vertices[id].boneWeights.x = weight;
				break;
			case 2:
				vertices[id].boneIds.y = i;
				vertices[id].boneWeights.y = weight;
				break;
			case 3:
				vertices[id].boneIds.z = i;
				vertices[id].boneWeights.z = weight;
				break;
			case 4:
				vertices[id].boneIds.w = i;
				vertices[id].boneWeights.w = weight;
				break;
 			}
		}
	}
}

void Model::readSkeleton(const aiScene* scene, aiMesh* mesh, aiNode* node)
{
	skeleton->readBones(mesh, node);
	if (scene->HasAnimations() == false)
		return;
	//���ص�һ������
	aiAnimation* ani = scene->mAnimations[0];
	skeleton->readAnimation(ani);
}

void Model::readMaterial(aiMaterial* material)
{
	//��������ͼ
	Material* diffuseMaterial = new Material(material, aiTextureType_DIFFUSE, "diffuse", directory);
	materials.push_back(diffuseMaterial);
	//�߹���ͼ
	Material* specularMaterial = new Material(material, aiTextureType_DIFFUSE, "specular", directory);
	materials.push_back(specularMaterial);
}

void Model::normalizeBonesWeight()
{
	//��Ȩ�ر�׼����ʹ����Ȩ���ܺ�Ϊ1
	for (int i = 0; i < vertices.size(); i++) {
		vec4& boneWeights = vertices[i].boneWeights;
		float totalWeight = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
		if (totalWeight > 0.0f) {
			vertices[i].boneWeights = vec4(
				boneWeights.x / totalWeight,
				boneWeights.y / totalWeight,
				boneWeights.z / totalWeight,
				boneWeights.w / totalWeight
			);
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