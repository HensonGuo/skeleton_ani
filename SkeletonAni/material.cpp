#include "material.h"


Material::Material(aiScene const* scene, aiMaterial* mat, aiTextureType type, string typeName, const string directory)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString ai_str;
		int locationID;
		mat->GetTexture(type, i, &ai_str);

		string filename = string(ai_str.C_Str());
		const aiTexture* aitexture = scene->GetEmbeddedTexture(filename.c_str());
		if (!aitexture)
			filename = directory + '/' + filename;

		// 检查之前是否加载了纹理，如果是，则继续下一次迭代：跳过加载新纹理
		bool loaded = false;
		for (unsigned int j = 0; j < textures.size(); j++)
		{
			if (textures[j].path.C_Str() == filename)
			{
				loaded = true;
				break;// 已加载具有相同文件路径的纹理，继续下一个（优化）。
			}
		}
		if (!loaded)
		{
			Texture texture;
			texture.type = typeName;
			if (aitexture)
			{
				texture.id = loadImageFromMemory(aitexture);
			}
			else
			{
				texture.id = loadImage(filename.c_str());
			}
			texture.path = filename;
			textures.push_back(texture);
		}
	}
}

GLuint Material::loadImage(const char* imagePath)
{
	int width, height, nrComponents;
	unsigned char* data = stbi_load(imagePath, &width, &height, &nrComponents, 0);
	if (data)
	{
		return bind(data, width, height, nrComponents);
	}
	else 
	{
		cout << "纹理无法从此路径加载: " << imagePath << endl;
		stbi_image_free(data);
		return 0;
	}
}

GLuint Material::loadImageFromMemory(const aiTexture* texture)
{
	int width, height, nrComponents;

	unsigned char* data;
	if (texture->mHeight == 0)
	{
		data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth, &width, &height, &nrComponents, 0);
	}
	else
	{
		data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth * texture->mHeight, &width, &height, &nrComponents, 0);
	}
	return bind(data, width, height, nrComponents);
}

void Material::draw(Shader& shader)
{
	int diffuseIndex = 1;
	int specularIndex = 1;

	for (int i = 0; i < textures.size(); i++) {
		glActiveTexture(textures[i].id);

		string number;
		string name = textures[i].type.C_Str();
		if (name == "diffuse") {
			number = to_string(diffuseIndex++);
		}

		else if (name == "specular") {
			number = to_string(specularIndex++);
		}

		glBindTexture(GL_TEXTURE_2D, textures[i].id);
		glUniform1i(glGetUniformLocation(shader.ID, ("material." + name + number).c_str()), i);
	}
}

void Material::reset()
{
	for (int i = 0; i < textures.size(); i++) {
		glActiveTexture(textures[i].id);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

GLuint Material::bind(unsigned char* data, int width, int height, int comp)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	GLenum format = 0;
	if (comp == 1)
		format = GL_RED;
	else if (comp == 3)
		format = GL_RGB;
	else if (comp == 4)
		format = GL_RGBA;

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	return textureID;
}
