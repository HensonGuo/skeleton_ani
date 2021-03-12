#include "model.h"

Model::Model(const string& path)
{
	this->loadModel(path);
}

void Model::loadModel(const string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
	}
	aiMesh* aimesh = scene->mMeshes[0];

	globalInverseTransform = assimpToGlmMatrix(scene->mRootNode->mTransformation);
	globalInverseTransform = glm::inverse(globalInverseTransform);

	vertices = {};
	indices = {};

	//load position, normal, uvglm::
	for (unsigned int i = 0; i < aimesh->mNumVertices; i++) {
		//λ��
		Vertex vertex;
		vec3 vector;
		vector.x = aimesh->mVertices[i].x;
		vector.y = aimesh->mVertices[i].y;
		vector.z = aimesh->mVertices[i].z;
		vertex.position = vector;
		//����
		vector.x = aimesh->mNormals[i].x;
		vector.y = aimesh->mNormals[i].y;
		vector.z = aimesh->mNormals[i].z;
		vertex.normal = vector;
		//uv
		vec2 vec;
		vec.x = aimesh->mTextureCoords[0][i].x;
		vec.y = aimesh->mTextureCoords[0][i].y;
		vertex.uv = vec;

		vertex.boneIds = ivec4(0);
		vertex.boneWeights = vec4(0.0f);

		vertices.push_back(vertex);
	}

	//��boneData���ص�����
	unordered_map<string, pair<int, mat4>> boneInfo = {};
	vector<uint> boneCounts;
	boneCounts.resize(vertices.size(), 0);
	boneCount = aimesh->mNumBones;

	//ѭ��ÿ������
	for (uint i = 0; i < boneCount; i++) {
		aiBone* bone = aimesh->mBones[i];
		mat4 m = assimpToGlmMatrix(bone->mOffsetMatrix);
		boneInfo[bone->mName.C_Str()] = { i, m };

		//ѭ��ÿ������
		for (int j = 0; j < bone->mNumWeights; j++) {
			uint id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			boneCounts[id]++;
			switch (boneCounts[id]) {
			case 1:
				vertices[id].boneIds.x = i;
				vertices[id].boneWeights.x = weight;
				break;
			case 2:
				vertices[id].boneIds.y = i;
				vertices[id].boneWeights.y = weight;
				break;
			case 3:
				vertices[id].boneIds.z = i;
				vertices[id].boneWeights.z = weight;
				break;
			case 4:
				vertices[id].boneIds.w = i;
				vertices[id].boneWeights.w = weight;
				break;
			default:
				//cout << "err: �޷����������������" << endl;
				break;

			}
		}
	}

	//��Ȩ�ر�׼����ʹ����Ȩ���ܺ�Ϊ1
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


	//���� indices
	for (int i = 0; i < aimesh->mNumFaces; i++) {
		aiFace& face = aimesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	readSkeleton(skeleton, scene->mRootNode, boneInfo);
	readTexture("./../resources/diffuse.png");
	readAnimation(scene);
	identity = mat4(1.0);
	currentPose.resize(boneCount, identity);
	mesh = Mesh(vertices, indices, textureId);
}

void Model::draw(Shader& shader)
{
	mesh.draw(shader.ID);
	float elapsedTime = glfwGetTime();
	getPose(currentPose, skeleton, elapsedTime, identity, globalInverseTransform);
	shader.setMat4("bone_transforms", boneCount, currentPose[0]);
}

void Model::getPose(vector<mat4>& output, Bone& skeletion, float dt, mat4& parentTransform, mat4& globalInverseTransform)
{
	BoneTransformTrack& btt = animation.boneTransforms[skeletion.name];
	dt = fmod(dt, animation.duration);
	pair<uint, float> fp;
	//�����ֵλ��
	fp = getTimeFraction(btt.positionTimestamps, dt);

	vec3 position1 = btt.positions[fp.first - 1];
	vec3 position2 = btt.positions[fp.first];

	vec3 position = glm::mix(position1, position2, fp.second);

	//�����ֵ��ת
	fp = getTimeFraction(btt.rotationTimestamps, dt);
	quat rotation1 = btt.rotations[fp.first - 1];
	quat rotation2 = btt.rotations[fp.first];

	quat rotation = glm::slerp(rotation1, rotation2, fp.second);

	//�����ֵ����
	fp = getTimeFraction(btt.scaleTimestamps, dt);
	vec3 scale1 = btt.scales[fp.first - 1];
	vec3 scale2 = btt.scales[fp.first];

	vec3 scale = glm::mix(scale1, scale2, fp.second);

	mat4 positionMat = mat4(1.0),
		scaleMat = mat4(1.0);


	//����ֲ��任
	positionMat = glm::translate(positionMat, position);
	mat4 rotationMat = glm::toMat4(rotation);
	scaleMat = glm::scale(scaleMat, scale);
	mat4 localTransform = positionMat * rotationMat * scaleMat;
	mat4 globalTransform = parentTransform * localTransform;

	output[skeletion.id] = globalInverseTransform * globalTransform * skeletion.offset;
	//�����ӹ�����ֵ
	for (Bone& child : skeletion.children) {
		getPose(output, child, dt, globalTransform, globalInverseTransform);
	}
	//cout << dt << " => " << position.x << ":" << position.y << ":" << position.z << ":" << endl;
}

bool Model::readSkeleton(Bone& boneOutput, aiNode* node, unordered_map<string, pair<int, mat4>>& boneInfoTable)
{
	if (boneInfoTable.find(node->mName.C_Str()) != boneInfoTable.end()) { // if node is actually a bone
		boneOutput.name = node->mName.C_Str();
		boneOutput.id = boneInfoTable[boneOutput.name].first;
		boneOutput.offset = boneInfoTable[boneOutput.name].second;

		for (int i = 0; i < node->mNumChildren; i++) {
			Bone child;
			readSkeleton(child, node->mChildren[i], boneInfoTable);
			boneOutput.children.push_back(child);
		}
		return true;
	}
	else { // find bones in children
		for (int i = 0; i < node->mNumChildren; i++) {
			if (readSkeleton(boneOutput, node->mChildren[i], boneInfoTable)) {
				return true;
			}

		}
	}
	return false;
}

void Model::readTexture(const string& path)
{
	int width, height, nrChannels;
	byte* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Model::readAnimation(const aiScene* scene)
{
	//���ص�һ������
	aiAnimation* anim = scene->mAnimations[0];

	if (anim->mTicksPerSecond != 0.0f)
		animation.ticksPerSecond = anim->mTicksPerSecond;
	else
		animation.ticksPerSecond = 1;


	animation.duration = anim->mDuration * anim->mTicksPerSecond;
	animation.boneTransforms = {};

	//����λ��ÿ����������ת������
	//ÿ��ͨ������ÿ������
	for (int i = 0; i < anim->mNumChannels; i++) {
		aiNodeAnim* channel = anim->mChannels[i];
		BoneTransformTrack track;
		for (int j = 0; j < channel->mNumPositionKeys; j++) {
			track.positionTimestamps.push_back(channel->mPositionKeys[j].mTime);
			track.positions.push_back(assimpToGlmVec3(channel->mPositionKeys[j].mValue));
		}
		for (int j = 0; j < channel->mNumRotationKeys; j++) {
			track.rotationTimestamps.push_back(channel->mRotationKeys[j].mTime);
			track.rotations.push_back(assimpToGlmQuat(channel->mRotationKeys[j].mValue));

		}
		for (int j = 0; j < channel->mNumScalingKeys; j++) {
			track.scaleTimestamps.push_back(channel->mScalingKeys[j].mTime);
			track.scales.push_back(assimpToGlmVec3(channel->mScalingKeys[j].mValue));

		}
		animation.boneTransforms[channel->mNodeName.C_Str()] = track;
	}
}

pair<uint, float> Model::getTimeFraction(vector<float>& times, float& dt)
{
	uint segment = 0;
	while (dt > times[segment])
		segment++;
	float start = times[segment - 1];
	float end = times[segment];
	float frac = (dt - start) / (end - start);
	return { segment, frac };
}
