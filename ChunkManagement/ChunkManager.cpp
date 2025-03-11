#include "ChunkManager.h"

void ChunkManager::updateChunkMap(const glm::ivec2& coords)
{
	m_chunkMap.iterate(coords);
	m_lastRecordedCoords = coords;
}

void ChunkManager::updateDynamicBlocks()
{
	BlockModifiedBulkEvent bulkModification;
	std::map<glm::ivec3, size_t, ComparatorIvec3> uniqueCoords; //coord and index
	{
		auto access = m_chunkMap.getMapDataAccess();
		while (!m_blockUpdateQueue.empty() &&
			m_blockUpdateQueue.top().tick <= m_gameServicesInterface.getGlobalTime())
		{
			auto data = m_blockUpdateQueue.top();
			m_blockUpdateQueue.pop();

			glm::ivec2 chunkCoords = Utils::tileToChunkCoord(data.coord, constChunkSize);
			glm::ivec3 localCoords = Utils::globalToLocal(data.coord, constChunkSize);

			const auto& it = access->regions.find(chunkCoords);
			if (it != access->regions.end())
			{
				BlockModifiedBulkEvent event;
				event = it->second.getCenter()->getReadAccess().data.updateDynamicBlock(getMapInterface(),
					m_gameServicesInterface, localCoords);

				for (BlockModifiedEvent& ptr : event.modifications) {
                    auto it = uniqueCoords.find(ptr.blockCoord);
                    if (it != uniqueCoords.end())
                    {
						auto& itVec = bulkModification.modifications[it->second];
                        if (ptr.newDynamicData->canOverride(itVec.newDynamicData->getType()))
							itVec = std::move(ptr);
                    }
                    else
                    {
						uniqueCoords[ptr.blockCoord] = bulkModification.modifications.size();
						bulkModification.modifications.push_back(std::move(ptr));
                    }
				}
			}
		}
	}
	if (!bulkModification.modifications.empty())
		m_gameServicesInterface.emit<GameEventTypes::BLOCK_MODIFIED_BULK>(std::move(bulkModification));
}
