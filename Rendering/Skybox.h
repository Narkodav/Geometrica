#pragma once
#include <vector>
#include <array>

#include "Rendering/CubeMap.h"
#include "Rendering/Shader.h"

class Skybox
{
private:
    static const int numOfSkyboxFloats;
    static const int numOfSkyboxVertices;
    static const float skyboxVertices[];

    bool isSet = 0;
	CubeMap* m_cubemap;
	std::vector<std::string> paths;

    unsigned int m_vertexArrayId, m_vertexBufferId;

public:
	~Skybox();

    void set(const std::vector<std::string>& paths);
    void draw(Shader& shader, const glm::mat4& viewWithoutTranspos, const glm::mat4& projection);
	void bind();
	void unbind();
};

