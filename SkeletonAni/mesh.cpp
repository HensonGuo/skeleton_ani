#include "mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	for (auto it = materials.begin(); it != materials.end(); it++)
	{
		delete* it;
		*it = NULL;
	}
	materials.clear();
}

Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Material*> &materials)
{
	this->vertices = vertices;
	this->indices = indices;
	this->materials = materials;
	this->setup();
}

void Mesh::draw(Shader& shader)
{
	//材质
	for (int i = 0; i < materials.size(); i++) {
		this->materials[i]->draw(shader);
	}
	//顶点
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	//重置
	for (int i = 0; i < materials.size(); i++) {
		this->materials[i]->reset();
	}
	glBindVertexArray(0);
}

void Mesh::readVertices(aiMesh* aimesh, uint nodeId)
{
	vertices.clear();
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
		vertex.nodeId = nodeId;
		vertices.push_back(vertex);
	}
}

void Mesh::readIndices(aiMesh* aimesh)
{
	indices.clear();
	for (unsigned int i = 0; i < aimesh->mNumFaces; i++) {
		aiFace& face = aimesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
}

void Mesh::readMaterials(aiScene const* scene, aiMesh* aimesh, string& directory)
{
	aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];
	//漫反射贴图
	Material* diffuseMaterial = new Material(scene, material, aiTextureType_DIFFUSE, "diffuse", directory);
	materials.push_back(diffuseMaterial);
	//镜面贴图
// 	Material* specularMaterial = new Material(scene, material, aiTextureType_SPECULAR, "specular", directory);
// 	materials.push_back(specularMaterial);
}

void Mesh::setVerticesWeights(aiMesh* aimesh, Skeleton* skeleton)
{
	vector<uint> boneCounts;
	boneCounts.resize(vertices.size(), 0);
	uint boneCount = aimesh->mNumBones;

	//循环每个骨骼
	for (uint i = 0; i < boneCount; i++) {
		aiBone* bone = aimesh->mBones[i];
		uint boneId = skeleton->boneName2Index.at(bone->mName.C_Str());
		//循环每个顶点
		for (uint j = 0; j < bone->mNumWeights; j++) {
			uint id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			boneCounts[id]++;
			switch (boneCounts[id]) {
			case 1:
				vertices[id].boneIds.x = boneId;
				vertices[id].boneWeights.x = weight;
				break;
			case 2:
				vertices[id].boneIds.y = boneId;
				vertices[id].boneWeights.y = weight;
				break;
			case 3:
				vertices[id].boneIds.z = boneId;
				vertices[id].boneWeights.z = weight;
				break;
			case 4:
				vertices[id].boneIds.w = boneId;
				vertices[id].boneWeights.w = weight;
				break;
			}
		}
	}
}

void Mesh::normalizeBonesWeight()
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

void Mesh::setup()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, boneIds));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, boneWeights));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, nodeId));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);
	glBindVertexArray(0);
}

uint Mesh::getVertextCount()
{
	return vertices.size();
}
