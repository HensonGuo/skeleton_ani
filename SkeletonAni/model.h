#pragma once

#include "mesh.h"
#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "material.h"
#include "skeleton.h"

#include <iostream>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"

using namespace std;
using namespace glm;


class Model
{
public:
	Model(const string& path);
	void loadModel(const string& path);
	void draw(Shader &shader);
	void playAnimation(bool active);

private:
	void readVertices(aiMesh* aimesh);
	void readIndices(aiMesh* aimesh);
	void setVerticesWeights(aiMesh* aimesh);
	void readSkeleton(const aiScene* scene, aiMesh* mesh, aiNode* node);
	void readMaterial(aiMaterial* material);
	void normalizeBonesWeight();
	void showNodeName(aiNode* node);
	
	string directory;

	Mesh mesh;	
	vector<Material*> materials = {};
	vector<Vertex> vertices = {};
	vector<uint> indices = {};
	Skeleton* skeleton;
};