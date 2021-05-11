#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <assimp/scene.h>

#include "utils.h"
#include "animation.h"


// 骨骼
class Bone {
public:
	int id = 0;
	string name = "";
	mat4 localTransform;
	/*偏移矩阵将顶点从模型空间转换为骨骼空间*/
	mat4 offset;
	vec3 position;

	Bone* parent;
	vector<Bone*> children = {};

	Bone(const string& name, int ID);
	~Bone();
	void update(float delta);
	mat4 getTransform(float delta);
	void setAnimation(Animation* ani, bool align=true);
	void clear();
private:
	Animation* ani;
};