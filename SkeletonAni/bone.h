#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <assimp/scene.h>

#include "utils.h"
#include "animation.h"


// ����
class Bone {
public:
	int id = 0;
	string name = "";
	mat4 localTransform;
	/*ƫ�ƾ��󽫶����ģ�Ϳռ�ת��Ϊ�����ռ�*/
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