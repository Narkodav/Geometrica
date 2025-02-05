#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "glm/glm.hpp"

struct ShaderProgramSource
{
	std::string VertexSource, FragmentSource;
};

class Shader
{
private:
	unsigned int m_ID;
	std::string m_FilePath;
	std::unordered_map<std::string, int> m_UniformLocationCache;
public:
	Shader();
	Shader(const std::string& filepath);
	~Shader();

	void set(const std::string& filepath);

	void Bind() const;
	void Unbind() const;

	void SetUniform1i(const std::string& name, int value);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
	int GetUniformLocation(const std::string& name);

private:
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CreateShader(const ShaderProgramSource sources);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	
	
};

