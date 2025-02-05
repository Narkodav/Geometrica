#include "ChunkManager.h"

void ChunkManager::iterate(glm::ivec2 newCoords)
{
	m_chunkMap.iterate(newCoords);
	m_lastRecordedCoords = newCoords;
}