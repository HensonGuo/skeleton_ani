#pragma once
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "animation.h"

class NodeInfo
{
public:
	uint id;
	std::string name;
	glm::mat4 localTransform;
	NodeInfo* parent;
	vector<NodeInfo*> children = {};
	Animation* ani;
	~NodeInfo();
	mat4 getTransform(float delta);
	void setAnimation(Animation* ani);
};