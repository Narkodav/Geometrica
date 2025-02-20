#include "ChunkManager.h"

void ChunkManager::updateChunkMap(const glm::ivec2& coords)
{
	m_chunkMap.iterate(coords);
	m_lastRecordedCoords = coords;
}

void ChunkManager::updateDynamicBlocks(const glm::ivec2& coords)
{

}