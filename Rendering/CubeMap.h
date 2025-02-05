#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>
#include "stb_image/stb_image.h"

enum class textureDir
{
	DIR_POSITIVE_X = 0,
	DIR_NEGATIVE_X,
	DIR_POSITIVE_Y,
	DIR_NEGATIVE_Y,
	DIR_POSITIVE_Z,
	DIR_NEGATIVE_Z,
	DIR_NUMBER
};

class CubeMap
{
private:
	unsigned int m_ID;
	std::vector<std::string> m_FilePaths;
	unsigned char* m_LocalBuffer;
	std::vector<int> m_Widths, m_Heights, m_BPPs;

public:
	CubeMap(const std::vector<std::string>& paths);
	~CubeMap();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth(textureDir dir) const { return m_Widths[(int)dir]; };
	inline int GetHeight(textureDir dir) const { return m_Heights[(int)dir]; };
	inline int GetId() const { return m_ID; };
};


