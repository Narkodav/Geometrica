#include "TextureAtlas.h"

void TextureAtlas::set(const std::vector<std::string>& filepaths)
{
	m_filepaths = filepaths;
	if (m_id)
		clear();
	stbi_set_flip_vertically_on_load(1);
	setNames();

	void* LocalBuffer;
	int bufferWidth;
	int bufferHeight;
	int bufferBpp;

	std::vector<Utils::BinPacker::Box> boxes;
	boxes.reserve(m_filepaths.size());

	for (int i = 0; i < m_filepaths.size(); i++)
	{
		if (!stbi_info(m_filepaths[i].c_str(), &bufferWidth, &bufferHeight, &bufferBpp))
			__debugbreak();

		boxes.push_back(Utils::BinPacker::Box(i, bufferWidth + m_paddingWidth * 2, bufferHeight + m_paddingWidth * 2));
	}

	Utils::BinPacker::Bin bin = Utils::BinPacker::firstFit(boxes);

	unpackBin(bin);
	createPixelData();
	setAtlas();
}

void TextureAtlas::createPixelData()
{
	uint8_t* localBuffer;
	int sourceHeight, sourceWidth, bppBuffer;
	AtlasLocation buffer;
	size_t atlasIndex;
	size_t bufferIndex;

	m_pixelData.resize(m_width * m_height * m_bpp);
	m_pixelData.assign(m_pixelData.size(), 0);

	for (int i = 0; i < m_filepaths.size(); i++)
	{
		localBuffer = stbi_load(m_filepaths[i].c_str(), &sourceWidth, &sourceHeight, &bppBuffer, m_bpp);
		if (!localBuffer)
			__debugbreak();

		buffer = m_locations.find(m_names[i])->second;
		if (buffer.upperLeft.x + sourceWidth > m_width || buffer.upperLeft.y + sourceHeight > m_height) {
			std::cerr << "Texture doesn't fit in atlas: " << m_filepaths[i] << std::endl;
			stbi_image_free(localBuffer);
			__debugbreak();
		}

		std::vector<int> averageColor(m_bpp, 0);

		//filling padding
		for (int i = 0; i < sourceHeight * sourceWidth; i++)
		{
			for (int j = 0; j < m_bpp; j++)
				averageColor[j] += localBuffer[i * m_bpp + j];

		}

		for (int j = 0; j < m_bpp; j++)
			averageColor[j] /= sourceHeight * sourceWidth;

		for (int y = -m_paddingWidth; y < sourceHeight + m_paddingWidth; y++)
			for (int x = -m_paddingWidth; x < sourceWidth + m_paddingWidth; x++)
			{
				atlasIndex = ((buffer.upperLeft.y + y) * m_width + buffer.upperLeft.x + x) * m_bpp;
				for (int j = 0; j < m_bpp; j++)
					m_pixelData[atlasIndex + j] = averageColor[j];
			}

		//filling data
		for (int y = 0; y < sourceHeight; y++)
		{
			atlasIndex = ((buffer.upperLeft.y + y) * m_width + buffer.upperLeft.x) * m_bpp;
			bufferIndex = y * sourceWidth * m_bpp;
			std::memcpy(&m_pixelData[atlasIndex], &localBuffer[bufferIndex], sourceWidth * m_bpp);
		}
		stbi_image_free(localBuffer);
	}
}

void TextureAtlas::unpackBin(const Utils::BinPacker::Bin& bin)
{
	for (int i = 0; i < bin.boxes.size(); i++)
		m_locations[m_names[bin.boxes[i].id]] = { glm::ivec2(bin.boxes[i].TopLeftX,bin.boxes[i].TopLeftY),bin.boxes[i].width,bin.boxes[i].height, bin.boxes[i].id };

	int maxWidth = 0;
	int maxHeight = 0;

	for (auto& location : m_locations)
	{
		if (maxWidth < location.second.width + location.second.upperLeft.x)
			maxWidth = location.second.width + location.second.upperLeft.x;

		if (maxHeight < location.second.height + location.second.upperLeft.y)
			maxHeight = location.second.height + location.second.upperLeft.y;
	}
	m_width = maxWidth;
	m_height = maxHeight;

	for (auto& location : m_locations)
	{
		location.second.upperLeft.x += m_paddingWidth;
		location.second.upperLeft.y += m_paddingWidth;
		location.second.width -= m_paddingWidth * 2;
		location.second.height -= m_paddingWidth * 2;

		location.second.transforms.uScale = location.second.width / static_cast<float>(m_width);
		location.second.transforms.vScale = location.second.height / static_cast<float>(m_height);
		location.second.transforms.uOffset = location.second.upperLeft.x / static_cast<float>(m_width);
		location.second.transforms.vOffset = location.second.upperLeft.y / static_cast<float>(m_height);
	}
}

void TextureAtlas::setNames()
{
	m_names.resize(m_filepaths.size());
	int dot, slash;

	for (int i = 0; i < m_filepaths.size(); i++)
	{
		dot = 0;
		for (slash = m_filepaths[i].size() - 1; slash > -1; slash--)
			if (m_filepaths[i][slash] == '.')
			{
				dot = slash;
				slash--;
				break;
			}

		for (slash; slash > -1; slash--)
			if (m_filepaths[i][slash] == '/')
				break;

		m_names[i] = m_filepaths[i].substr(slash + 1, dot - slash - 1);
	}
}

void TextureAtlas::setAtlas()
{
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixelData.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureAtlas::save(std::string filepath)
{
	stbi_flip_vertically_on_write(1);
	if (!stbi_write_png(filepath.c_str(), m_width, m_height, m_bpp, m_pixelData.data(), m_width * m_bpp))
	{
		std::cerr << "Failed to write atlas: " << filepath << std::endl;
		return;
	}
	std::cout << "Successfully wrote atlas: " << filepath << std::endl;
}

const void TextureAtlas::displayPixelData() const
{
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			for (int bpp = 0; bpp < m_bpp; bpp++)
				std::cout << std::hex << static_cast<int>(m_pixelData[y * m_width * m_bpp + x * m_bpp + bpp]);
			std::cout << " ";
		}
		std::cout << std::endl;
	}
}

void TextureAtlas::clear()
{
	glDeleteTextures(1, &m_id);
	m_pixelData.clear();
}

void TextureAtlas::Bind(unsigned int slot /*= 0*/) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_id);
}

void TextureAtlas::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureAtlas::makeStorageBuffer()
{
	std::vector<glm::vec4> bufferData(m_locations.size());

	for(int i = 0; i < m_locations.size(); i++)
	{
		bufferData[i] = { m_locations[m_names[i]].transforms.uScale, m_locations[m_names[i]].transforms.vScale,
			m_locations[m_names[i]].transforms.uOffset, m_locations[m_names[i]].transforms.vOffset };
		m_locations[m_names[i]].index = i;
	}

	glGenBuffers(1, &m_bufferId);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferId);

	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferData.size() * sizeof(bufferData[0]), bufferData.data(), GL_STATIC_COPY_ARB);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferId);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void TextureAtlas::bindStorageBuffer() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferId);
}

void TextureAtlas::unbindStorageBuffer() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}