#pragma once

#include "mesh.h"
#include "bone.h"
#include "utils.h"
#include "shader.h"
#include "material.h"
#include "skeleton.h"
#include "constants.h"

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
	Model();
	Model(const string& path);
	void loadModel(const string& path);
	void draw(Shader &shader, DrawType drawType);
	void playAnimation(bool active);
	bool isPlayingAnimation();

	float getAniDuration();
	float getAniElapsed();
	void changePose(float delta);
private:
	void readVertices(aiMesh* aimesh);
	void readIndices(aiMesh* aimesh);
	void setVerticesWeights(aiMesh* aimesh);
	void readSkeleton(const aiScene* scene, aiMesh* mesh, aiNode* node);
	void readMaterial(aiScene const* scene, aiMaterial* material);
	void normalizeBonesWeight();
	void showNodeName(aiNode* node);
	
	string directory;
	bool onlyDrawSkeleton;

	Mesh mesh;	
	vector<Material*> materials = {};
	vector<Vertex> vertices = {};
	vector<uint> indices = {};
	Skeleton* skeleton;
};