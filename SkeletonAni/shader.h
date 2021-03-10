#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>


class Shader
{
public:
	// 生成一个ID
	unsigned int ID;
	// 构造函数
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	// 使用/激活程序
	void use() { glUseProgram(ID); }
	void unuse() { glUseProgram(0); }
	// 工具函数,用于获取所有着色器的uniform位置
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;
	void setMat4(const std::string& name, GLsizei count, const glm::mat4& mat) const;

private:
	// 检查编译或链接是否出错
	void checkCompileErrors(unsigned int shader, std::string type);
};
