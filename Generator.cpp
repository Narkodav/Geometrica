#include "Generator.h"

Generator::Generator()
{
	//std::random_device rd;
	//m_randomGenerator.seed(rd());
	m_seed = 0;
}

void Generator::set(unsigned int seed)
{ 
	m_seed = seed; 
	m_perlinNoise2d.setSeed(m_seed);
	m_perlinNoise3d1.setSeed(m_seed);
	m_perlinNoise3d2.setSeed(m_seed ^ UINT_MAX);
	m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_AIR)] = DataRepository::airId;
	m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_DIRT)] = DataRepository::getItem("dirt_block").id;
	m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_GRASS)] = DataRepository::getItem("grass_block").id;
	m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_STONE)] = DataRepository::getItem("stone_block").id;
}

bool Generator::shouldBeCave(int x, int y, int z) 
{
	float caveDensity = (m_perlinNoise3d1.getFbm(x, y, z, 3, 0.04f) + 1) / 2;
	caveDensity *= caveDensity;
	// Create the worm effect by checking if noise is within a specific range

	// Check if the noise value is within our "worm" range
	if (caveDensity >= 0.4f) return 0;

	caveDensity += pow((m_perlinNoise3d2.getFbm(x, y, z, 3, 0.04f) + 1) / 2, 2);
	if (caveDensity >= 0.4f) return 0;

	return 1;
}

ChunkData Generator::getChunkData(glm::ivec2 coords)
{
	ChunkData data;
	data.getCoords() = coords;
	glm::ivec3 coords000 = glm::vec3(coords.x * constChunkSize, 0, coords.y * constChunkSize);
	int height = 0;
	float amplitudeHight = 16.f;
	float density;
	for (int x = 0; x < constChunkSize; x++)
		for (int z = 0; z < constChunkSize; z++)
		{
			height = m_groundLevel + amplitudeHight * m_perlinNoise2d.getFbm(x + coords000.x, z + coords000.z, 3, 0.02f);
			for (int y = 0; y < height; y++)
			{
				if (shouldBeCave(x + coords000.x, y, z + coords000.z))
					data.getBlock(y, x, z) = m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_AIR)];
				else data.getBlock(y, x, z) = m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_GRASS)];
			}
			for (int y = height; y < constWorldHeight; y++)
			{
				data.getBlock(y, x, z) = m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_AIR)];
			}
		}
	return data;
}