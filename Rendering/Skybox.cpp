#include "Skybox.h"

const int Skybox::numOfSkyboxFloats = 108;
const int Skybox::numOfSkyboxVertices = 36;
const float Skybox::skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

void Skybox::set(const std::vector<std::string>& paths)
{
	if(isSet)
		delete m_cubemap;
	m_cubemap = new CubeMap(paths);
	glGenVertexArrays(1, &m_vertexArrayId);
	glBindVertexArray(m_vertexArrayId);

	glGenBuffers(1, &m_vertexBufferId);

	//populating position buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices[0]) * numOfSkyboxFloats, (void*)skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	isSet = 1;
}

Skybox::~Skybox()
{
	if(isSet)
		delete m_cubemap;
}

void Skybox::draw(Shader& shader, const glm::mat4& viewWithoutTranspos, const glm::mat4& projection)
{
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	shader.Bind();
	glBindVertexArray(m_vertexArrayId);
	m_cubemap->Bind(0);
	glUniformMatrix4fv(shader.GetUniformLocation("u_viewTransform"), 1, GL_FALSE, &viewWithoutTranspos[0][0]);
	glUniformMatrix4fv(shader.GetUniformLocation("u_projectionTransform"), 1, GL_FALSE, &projection[0][0]);
	glUniform1i(shader.GetUniformLocation("skybox"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

void Skybox::bind()
{
	glBindVertexArray(m_vertexArrayId);
	m_cubemap->Bind();
}

void Skybox::unbind()
{
	m_cubemap->Unbind();
	glBindVertexArray(0);
}