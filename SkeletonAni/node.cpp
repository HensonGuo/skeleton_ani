#include "node.h"

NodeInfo::~NodeInfo()
{
	parent = NULL;
	for (auto it = children.begin(); it != children.end(); it++)
	{
		*it = NULL;
	}
	children.clear();
	delete ani;
	ani = NULL;
}

mat4 NodeInfo::getTransform(float delta)
{
	if (ani && ani->hasAnimaiton())
	{
		ani->update(delta);
		return ani->getCurrentTransform();
	}
	else
	{
		return localTransform;
	}
}

void NodeInfo::setAnimation(Animation* ani)
{
	this->ani = ani;
}
