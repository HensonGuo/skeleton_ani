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
	//normalizeBonesWeight();
	mesh = Mesh(vertices, indices, materials);
}

void Model::draw(Shader& shader)
{
	mesh.draw(shader);
	skeleton->draw(shader);
}


void Model::readVertices(aiMesh* aimesh)
{
	vertices = {};
	for (unsigned int i = 0; i < aimesh->mNumVertices; i++) {
		//位置
		Vertex vertex;
		vertex.position = assimpToGlmVec3(aimesh->mVertices[i]);
		//法线
		if (aimesh->HasNormals()) {
			vertex.normal = assimpToGlmVec3(aimesh->mNormals[i]);
		}
		else
		{
			vertex.normal = vec3();
		}
		//纹理
		vec2 vec;
		if (aimesh->HasTextureCoords(0))
		{
			//一个顶点最多可以包含8个不同的纹理坐标。非多个纹理坐标的模型，取第一组（0）
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
	boneCount = aimesh->mNumBones;

	//循环每个骨骼
	for (uint i = 0; i < boneCount; i++) {
		aiBone* bone = aimesh->mBones[i];
		uint boneId = skeleton->boneName2Index.at(bone->mName.C_Str());
		//循环每个顶点
		for (uint j = 0; j < bone->mNumWeights; j++) {
// 			uint id = bone->mWeights[j].mVertexId;
// 			float weight = bone->mWeights[j].mWeight;
// 			boneCounts[id]++;
// 			switch (boneCounts[id]) {
// 			case 1:
// 				vertices[id].boneIds.x = i;
// 				vertices[id].boneWeights.x = weight;
// 				break;
// 			case 2:
// 				vertices[id].boneIds.y = i;
// 				vertices[id].boneWeights.y = weight;
// 				break;
// 			case 3:
// 				vertices[id].boneIds.z = i;
// 				vertices[id].boneWeights.z = weight;
// 				break;
// 			case 4:
// 				vertices[id].boneIds.w = i;
// 				vertices[id].boneWeights.w = weight;
// 				break;

			aiVertexWeight var = bone->mWeights[j];
			uint index = indices[var.mVertexId];
			Vertex* vertex = &vertices[index];
			float weight = var.mWeight;

			bool finded = false;
			for (unsigned int k = 0; k < 4; k++)
			{
				if (vertex->boneWeights[k] < 0.0f)
				{
					vertex->boneWeights[k] = weight;
					vertex->boneIds[k] = boneId;
					finded = true;
					break;
				}
			}
			if (finded)
				continue;

			// 如果顶点受4个以上关节的影响，则保持较大值的顶点
			for (unsigned int k = 0; k < 4; k++)
			{
				if (weight > vertex->boneWeights[k])
				{
					vertex->boneWeights[k] = weight;
					vertex->boneIds[k] = boneId;
					break;
				}
 			}
		}
	}
}

void Model::readSkeleton(const aiScene* scene, aiMesh* mesh, aiNode* node)
{
	skeleton->readBones(mesh, node);
	if (scene->HasAnimations() == false)
		return;
	//加载第一个动画
	aiAnimation* ani = scene->mAnimations[0];
	skeleton->readAnimation(ani);
}

void Model::readMaterial(aiMaterial* material)
{
	//漫反射贴图
	Material* diffuseMaterial = new Material(material, aiTextureType_DIFFUSE, "diffuse", directory);
	materials.push_back(diffuseMaterial);
	//高光贴图
	Material* specularMaterial = new Material(material, aiTextureType_DIFFUSE, "specular", directory);
	materials.push_back(specularMaterial);
}

void Model::normalizeBonesWeight()
{
	//将权重标准化，使所有权重总和为1
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
