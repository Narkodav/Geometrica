#include "CubeMap.h"
#include <iostream>

CubeMap::CubeMap(const std::vector<std::string>& paths) : m_ID(0), m_FilePaths(paths)
{
	stbi_set_flip_vertically_on_load(0);
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	m_Widths.resize((int)textureDir::DIR_NUMBER);
	m_Heights.resize((int)textureDir::DIR_NUMBER);
	m_BPPs.resize((int)textureDir::DIR_NUMBER);

	for (unsigned int i = 0; i < paths.size(); i++)
	{
		m_LocalBuffer = stbi_load(paths[i].c_str(), &m_Widths[i], &m_Heights[i], &m_BPPs[i], 4);
		if (!m_LocalBuffer)
			__debugbreak();
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, m_Widths[i], m_Heights[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
		stbi_image_free(m_LocalBuffer);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

CubeMap::~CubeMap()
{
	if (m_ID)
		glDeleteTextures(1, &m_ID);
}

void CubeMap::Bind(unsigned int slot /*= 0*/) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
}

void CubeMap::Unbind() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}