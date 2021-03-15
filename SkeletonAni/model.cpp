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

	directory = path.substr(0, path.find_last_of('/'));

	cout << " \n------------------------------- Details Info-------------------------------------------- \n " << endl;
	cout << "              Animations: " << scene->HasAnimations() << endl;
	cout << "                  Meshes: " << scene->mNumMeshes << endl;
	cout << "              Bone Count: " << scene->mAnimations[0]->mNumChannels << endl;
	cout << "      Animation Duration: " << scene->mAnimations[0]->mDuration << endl;
	cout << "Animation TicksPerSecond: " << scene->mAnimations[0]->mTicksPerSecond << endl << endl;

	showNodeName(scene->mRootNode);

	readVertices(aimesh);
	readIndices(aimesh);
	readBones(aimesh);
	readSkeleton(skeleton, scene->mRootNode, boneInfo);
	if (aimesh->mMaterialIndex >= 0)
	{
		readMaterial(scene->mMaterials[aimesh->mMaterialIndex]);
	}	
	readAnimation(scene);
	identity = mat4(1.0);
	currentPose.resize(boneCount, identity);
	mesh = Mesh(vertices, indices, materials);
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
	//计算插值位置
	fp = getTimeFraction(btt.positionTimestamps, dt);

	vec3 position1 = btt.positions[fp.first - 1];
	vec3 position2 = btt.positions[fp.first];

	vec3 position = glm::mix(position1, position2, fp.second);

	//计算插值旋转
	fp = getTimeFraction(btt.rotationTimestamps, dt);
	quat rotation1 = btt.rotations[fp.first - 1];
	quat rotation2 = btt.rotations[fp.first];

	quat rotation = glm::slerp(rotation1, rotation2, fp.second);

	//计算插值缩放
	fp = getTimeFraction(btt.scaleTimestamps, dt);
	vec3 scale1 = btt.scales[fp.first - 1];
	vec3 scale2 = btt.scales[fp.first];

	vec3 scale = glm::mix(scale1, scale2, fp.second);

	mat4 positionMat = mat4(1.0),
		scaleMat = mat4(1.0);


	//计算局部变换
	positionMat = glm::translate(positionMat, position);
	mat4 rotationMat = glm::toMat4(rotation);
	scaleMat = glm::scale(scaleMat, scale);
	mat4 localTransform = positionMat * rotationMat * scaleMat;
	mat4 globalTransform = parentTransform * localTransform;

	output[skeletion.id] = globalInverseTransform * globalTransform * skeletion.offset;
	//更新子骨骼的值
	for (Bone& child : skeletion.children) {
		getPose(output, child, dt, globalTransform, globalInverseTransform);
	}
	//cout << dt << " => " << position.x << ":" << position.y << ":" << position.z << ":" << endl;
}

void Model::readVertices(aiMesh* aimesh)
{
	vertices = {};
	for (unsigned int i = 0; i < aimesh->mNumVertices; i++) {
		//位置
		Vertex vertex;
		vertex.position = assimpToGlmVec3(aimesh->mVertices[i]);
		//法线
		if (aimesh->mNormals) {
			vertex.normal = assimpToGlmVec3(aimesh->mNormals[i]);
		}
		else
		{
			vertex.normal = vec3();
		}
		//uv
		vec2 vec;
		vec.x = aimesh->mTextureCoords[0][i].x;
		vec.y = aimesh->mTextureCoords[0][i].y;
		vertex.uv = vec;

		vertex.boneIds = ivec4(0);
		vertex.boneWeights = vec4(0.0f);

		vertices.push_back(vertex);
	}
}

void Model::readIndices(aiMesh* aimesh)
{
	indices = {};
	for (unsigned int i = 0; i < aimesh->mNumFaces; i++) {
		aiFace& face = aimesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
}

void Model::readBones(aiMesh* aimesh)
{
	vector<uint> boneCounts;
	boneCounts.resize(vertices.size(), 0);
	boneCount = aimesh->mNumBones;

	//循环每个骨骼
	for (uint i = 0; i < boneCount; i++) {
		aiBone* bone = aimesh->mBones[i];
		mat4 m = assimpToGlmMatrix(bone->mOffsetMatrix);
		boneInfo[bone->mName.C_Str()] = { i, m };

		//循环每个顶点
		for (uint j = 0; j < bone->mNumWeights; j++) {
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
				//cout << "err: 无法将骨骼分配给顶点" << endl;
				break;

			}
		}
	}
}

bool Model::readSkeleton(Bone& boneOutput, aiNode* node, unordered_map<string, pair<int, mat4>>& boneInfoTable)
{
	if (boneInfoTable.find(node->mName.C_Str()) != boneInfoTable.end()) { // if node is actually a bone
		boneOutput.name = node->mName.C_Str();
		boneOutput.id = boneInfoTable[boneOutput.name].first;
		boneOutput.offset = boneInfoTable[boneOutput.name].second;

		for (uint i = 0; i < node->mNumChildren; i++) {
			Bone child;
			readSkeleton(child, node->mChildren[i], boneInfoTable);
			boneOutput.children.push_back(child);
		}
		return true;
	}
	else { // find bones in children
		for (uint i = 0; i < node->mNumChildren; i++) {
			if (readSkeleton(boneOutput, node->mChildren[i], boneInfoTable)) {
				return true;
			}

		}
	}
	return false;
}

void Model::readMaterial(aiMaterial* material)
{
	//漫反射贴图
	Material* diffuseMaterial = new Material(material, aiTextureType_DIFFUSE, "diffuse", directory);
	materials.push_back(diffuseMaterial);
	//高光贴图
	Material* specularMaterial = new Material(material, aiTextureType_DIFFUSE, "specular", directory);
	materials.push_back(specularMaterial);
}

void Model::readAnimation(const aiScene* scene)
{
	//加载第一个动画
	aiAnimation* anim = scene->mAnimations[0];

	if (anim->mTicksPerSecond != 0.0f)
		animation.ticksPerSecond = anim->mTicksPerSecond;
	else
		animation.ticksPerSecond = 1;


	animation.duration = anim->mDuration * anim->mTicksPerSecond;
	animation.boneTransforms = {};

	//加载位置每个骨骼的旋转和缩放
	//每个通道代表每个骨骼
	for (uint i = 0; i < anim->mNumChannels; i++) {
		aiNodeAnim* channel = anim->mChannels[i];
		BoneTransformTrack track;
		for (uint j = 0; j < channel->mNumPositionKeys; j++) {
			track.positionTimestamps.push_back(channel->mPositionKeys[j].mTime);
			track.positions.push_back(assimpToGlmVec3(channel->mPositionKeys[j].mValue));
		}
		for (uint j = 0; j < channel->mNumRotationKeys; j++) {
			track.rotationTimestamps.push_back(channel->mRotationKeys[j].mTime);
			track.rotations.push_back(assimpToGlmQuat(channel->mRotationKeys[j].mValue));

		}
		for (uint j = 0; j < channel->mNumScalingKeys; j++) {
			track.scaleTimestamps.push_back(channel->mScalingKeys[j].mTime);
			track.scales.push_back(assimpToGlmVec3(channel->mScalingKeys[j].mValue));

		}
		animation.boneTransforms[channel->mNodeName.C_Str()] = track;
	}
}

void Model::normalizeBonesWeight()
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

void Model::showNodeName(aiNode* node)
{
	cout << node->mName.data << endl;
	for (uint i = 0; i < node->mNumChildren; i++) {
		showNodeName(node->mChildren[i]);
	}
}
