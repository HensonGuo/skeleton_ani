#include "bone.h"

Bone::Bone(const string& name, int ID)
	:name(name),
	id(ID),
	localTransform(1.0f)
{
}

Bone::~Bone()
{
	parent = NULL;
	for (auto it = children.begin(); it != children.end(); it++)
	{
		*it = NULL;
	}
	children.clear();
	clear();
}

void Bone::update(float delta)
{
	if (ani)
		ani->update(delta);
}

mat4 Bone::getTransform(float delta)
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
	return mat4();
}

void Bone::setAnimation(Animation* ani, bool align)
{
	this->ani = ani;
	for (int positionIndex = 0; positionIndex < ani->numPostions; ++positionIndex)
	{
		if (align)
			ani->positions[positionIndex].position = vec3(localTransform[3][0], localTransform[3][1], localTransform[3][2]);
	}
}

void Bone::clear()
{
	delete ani;
	ani = NULL;
}