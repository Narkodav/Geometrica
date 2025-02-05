#pragma once
#include <GL/glew.h>
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

#include "Utilities/BinPacker.h"

struct AtlasTransforms
{
	float uScale;
	float vScale;
	float uOffset;
	float vOffset;
};

struct AtlasLocation
{
	glm::ivec2 upperLeft;
	unsigned int width;
	unsigned int height;

	unsigned int index;
	//data for passing to the renderer
	AtlasTransforms transforms;
};

class TextureAtlas //top left is x = 0 and y = 0, down is positive y, right is positive x
{
private:
	unsigned int m_id = 0;
	unsigned int m_bufferId = 0;

	std::vector<uint8_t> m_pixelData;
	const int m_bpp = 4; //RGBA
	const int m_paddingWidth = 8; //border around a texture
	unsigned int m_width;
	unsigned int m_height;

	std::vector<std::string> m_filepaths; //corresponds to names
	std::vector<std::string> m_names;
	std::unordered_map<std::string, AtlasLocation> m_locations; //texture name - index

	void setNames();
	void unpackBin(const Utils::BinPacker::Bin& bin);
	void createPixelData();
	void setAtlas();

public:
	void set(const std::vector<std::string>& filepaths);
	void clear();
	void save(std::string filepath);
	const void displayPixelData() const;

	void makeStorageBuffer();
	void bindStorageBuffer() const;
	void unbindStorageBuffer() const;

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	glm::vec2 getTextureCoords(std::string name, glm::vec2 coords) const {
		return { coords.x * m_locations.find(name)->second.transforms.uScale + m_locations.find(name)->second.transforms.uOffset,
		coords.y * m_locations.find(name)->second.transforms.vScale + m_locations.find(name)->second.transforms.vOffset };
	};

	AtlasTransforms getTextureAtlasTransforms(std::string name) const { return  m_locations.find(name)->second.transforms; };
	
	unsigned int getStorageBufferIndex(std::string name) const { return m_locations.find(name)->second.index; };

	unsigned int getId() const { return m_id; };


};

