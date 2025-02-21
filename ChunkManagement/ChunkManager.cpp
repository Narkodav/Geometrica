#include "ChunkManager.h"

void ChunkManager::updateChunkMap(const glm::ivec2& coords)
{
	m_chunkMap.iterate(coords);
	m_lastRecordedCoords = coords;
}

void ChunkManager::updateDynamicBlocks()
{

	BlockModifiedBulkEvent bulkModification;
	{
		auto access = m_chunkMap.getMapDataAccess();
		for(auto& region : access->regions)
		{
			BlockModifiedBulkEvent bulkModificationBuffer =
				region.second.getCenter()->getReadAccess().data.updateDynamicBlocks(getMapQuery(),
					m_gameServicesInterface);
			for (BlockModifiedEvent& ptr : bulkModificationBuffer.modifications) {
				bulkModification.modifications.emplace_back(std::move(ptr));
			}
		}
	}

	if (!bulkModification.modifications.empty())
		m_gameServicesInterface.emit<GameEventTypes::BLOCK_MODIFIED_BULK>(bulkModification);
}