#include "Texture.h"


Texture::Texture() : m_ID(0), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
	
}

Texture::Texture(const std::string& path) : m_ID(0), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
	load(path);
}

Texture::~Texture()
{
	if (m_ID)
		clear();
}

bool Texture::operator==(Texture& other) const noexcept {
	return this->m_ID == other.m_ID;

};

void Texture::load(const std::string& path)
{
	if (m_ID)
		clear();
	m_FilePath = path;
	stbi_set_flip_vertically_on_load(1);
	m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);

	if (m_LocalBuffer)
	{
		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(m_LocalBuffer);
	}
}

void Texture::clear()
{
	glDeleteTextures(1, &m_ID);
}

void Texture::Bind(unsigned int slot /*= 0*/) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
