#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>


class Shader
{
public:
	// ����һ��ID
	unsigned int ID;
	// ���캯��
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	// ʹ��/�������
	void use() { glUseProgram(ID); }
	void unuse() { glUseProgram(0); }
	// ���ߺ���,���ڻ�ȡ������ɫ����uniformλ��
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;
	void setMat4(const std::string& name, GLsizei count, const glm::mat4& mat) const;

private:
	// ������������Ƿ����
	void checkCompileErrors(unsigned int shader, std::string type);
};
