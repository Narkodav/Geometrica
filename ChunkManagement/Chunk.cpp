#include "Chunk.h"

void Chunk::generateChunk(const glm::ivec2& coords, Generator& generator)
{
	std::lock_guard<std::shared_mutex> lock(m_mutex);
	m_data = generator.getChunkData(coords);
}

void Chunk::storeChunk()
{
	std::lock_guard<std::shared_mutex> lock(m_mutex);
	m_data.clear();
}



