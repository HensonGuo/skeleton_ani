#include "material.h"


Material::Material(aiMaterial* mat, aiTextureType type, string typeName, const string directory)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString ai_str;
		mat->GetTexture(type, i, &ai_str);

		string filename = string(ai_str.C_Str());
		filename = directory + '/' + filename;

		// ���֮ǰ�Ƿ��������������ǣ��������һ�ε�������������������
		bool loaded = false;
		for (unsigned int j = 0; j < textures.size(); j++)
		{
			if (textures[j].path.C_Str() == filename)
			{
				loaded = true;
				break;// �Ѽ��ؾ�����ͬ�ļ�·��������������һ�����Ż�����
			}
		}
		if (!loaded)
		{
			Texture texture;
			texture.type = typeName;
			texture.id = loadImage(filename.c_str());
			texture.path = ai_str;
			textures.push_back(texture);
		}
	}
}

GLuint Material::loadImage(const char* imagePath)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(imagePath, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = 0;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		cout << "�����޷��Ӵ�·������: " << imagePath << endl;
		stbi_image_free(data);
	}
	return textureID;
}

void Material::draw(Shader& shader)
{
	int diffuseIndex = 1;
	int specularIndex = 1;

	for (int i = 0; i < textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);

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
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
