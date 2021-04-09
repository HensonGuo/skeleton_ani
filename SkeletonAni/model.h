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
	void clear();
	void loadModel(const string& path);
	void loadAnimation(const string& path);
	void draw(Shader &shader, DrawType drawType);

	void playAnimation(bool active);
	bool isPlayingAnimation();
	float getAniDuration();
	float getAniElapsed();

	void changePoseStopAtTime(float delta);

	uint getBonesCount();
	uint getVertexCount();
private:
	void processNode(aiNode* node, const aiScene* scene);
	void processAnimation(const aiScene* scene);
	void showNodeName(aiNode* node);
	
	string directory;

	uint vertexCount;
	vector<Mesh*> meshes;
	Skeleton* skeleton;
};