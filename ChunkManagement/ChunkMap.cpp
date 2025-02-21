#include "ChunkMap.h"

ChunkMap::ChunkMap(unsigned int loadDistance, Generator& generator,
	const GameServicesInterface<GameEventPolicy>& gameServicesInterface) :
	m_loadDistance(loadDistance), m_loadingTaskCoordinator(gameServicesInterface.getTaskCoordinator(gameServicesInterface.getWorkerAmount() / 4)),
	m_unloadingTaskCoordinator(gameServicesInterface.getTaskCoordinator(gameServicesInterface.getWorkerAmount() / 4)),
	m_generator(generator), m_chunkCounter(0), m_eventSystemInterface(gameServicesInterface)
{
	m_loadDistanceSquaredWithCushion = pow(m_loadDistance + 0.5, 2);
	m_chunkPool.set(pow((m_loadDistance * 2 + 1), 2) * 2);

	m_loadArea = Utils::getSortedCircleCoords(m_loadDistance, 0.5);
}

unsigned int ChunkMap::iterate(glm::ivec2 positionCurrent)
{
	unsigned int taskAmount = 0;
	if (!(m_loadingTaskCoordinator.canAddTask() || m_unloadingTaskCoordinator.canAddTask()))
		return taskAmount;
	auto mapAccess = m_mapData.getWriteAccess();

	for (int i = 0; i < m_loadArea.size(); i++)
	{
		glm::ivec2 globalCoord = m_loadArea[i] + positionCurrent;

		auto genChunk = mapAccess->states.find(globalCoord);

		//generation
		if (genChunk == mapAccess->states.end())
		{
			taskAmount++;
			if (!m_loadingTaskCoordinator.tryAddTask([this, globalCoord] {
				generateChunk(globalCoord, m_generator); }))
				break;
			mapAccess->states.emplace(globalCoord, ChunkState::BEING_GENERATED);
		}
		else if (genChunk->second == ChunkState::STORED)
		{
			taskAmount++;
			if (!m_loadingTaskCoordinator.tryAddTask([this, globalCoord] {
				generateChunk(globalCoord, m_generator); }))
				break;
			genChunk->second = ChunkState::BEING_GENERATED;
		}
		else if (genChunk->second == ChunkState::ERRONEOUS)
		{
			__debugbreak();
		}
	}

	for (auto& region : mapAccess->regions)
	{
		//storage
		auto relativeCoord = region.first - positionCurrent;
		auto state = mapAccess->states.find(region.first);
		if (state == mapAccess->states.end())
			__debugbreak();
		if (glm::dot(glm::vec2(relativeCoord), glm::vec2(relativeCoord)) > m_loadDistanceSquaredWithCushion &&
			state != mapAccess->states.end() && state->second == ChunkState::GENERATED)
		{
			taskAmount++;
			if (!m_unloadingTaskCoordinator.tryAddTask([this, coord = region.first] {
				storeChunk(coord); }))
				break;
			state->second = ChunkState::BEING_STORED;
		}
	}
	return taskAmount;
}

void ChunkMap::generateChunk(glm::ivec2 coords, Generator& generator)
{
	MT::MemoryPool<Chunk>::SharedPointer chunk = m_chunkPool.makeShared();
	chunk->generateChunk(coords, generator);
	auto mapAccess = m_mapData.getWriteAccess();

	auto& region = mapAccess->regions.emplace(coords, chunk).first->second;
	coupleChunks(mapAccess, region, coords);

	auto it = mapAccess->states.find(coords);
	if (it == mapAccess->states.end())
		__debugbreak();
	it->second = ChunkState::GENERATED;
}

void ChunkMap::storeChunk(glm::ivec2 coords)
{
	bool erroneous = 0;
	MT::MemoryPool<Chunk>::SharedPointer chunk;
	{
		auto mapAccess = m_mapData.getWriteAccess();
		auto it = mapAccess->regions.find(coords);
		if (it == mapAccess->regions.end())
		{
			erroneous = 1;
			__debugbreak();
		}
		else
		{
			decoupleChunks(mapAccess, it->second, coords);
			chunk = it->second.getCenter();
			mapAccess->regions.erase(it);
		}
		

		auto itState = mapAccess->states.find(coords);
		if (itState == mapAccess->states.end() || erroneous)
		{
			mapAccess->states.emplace(coords, ChunkState::ERRONEOUS);
			__debugbreak();
			return;
		}
		else
		{
			itState->second = ChunkState::STORED;
		}
	}
	chunk->storeChunk();
}

void ChunkMap::coupleChunks(MT::Synchronized<ChunkMapData>::WriteAccess& mapAccess,
	ChunkRegion& region, glm::ivec2 coords)
{
	for (int i = 0; i < constDirectionVectors2DHashed.size(); i++)
	{
		auto adj = mapAccess->regions.find(coords + constDirectionVectors2DHashed[i]);
		if (adj != mapAccess->regions.end())
		{
			region.addAdjacentChunk(adj->second.getCenter(), 
			static_cast<Directions2DHashed>(i));

			adj->second.addAdjacentChunk(region.getCenter(), 
			static_cast<Directions2DHashed>(getOpposite2D(i)));
		}
	}
}

void ChunkMap::decoupleChunks(MT::Synchronized<ChunkMapData>::WriteAccess& mapAccess,
	ChunkRegion& region, glm::ivec2 coords)
{
	for (int i = 0; i < constDirectionVectors2DHashed.size(); i++)
	{
		auto adj = mapAccess->regions.find(coords + constDirectionVectors2DHashed[i]);
		if (adj != mapAccess->regions.end())
		{
			region.removeAdjacentChunk(static_cast<Directions2DHashed>(i));
			adj->second.removeAdjacentChunk(static_cast<Directions2DHashed>(getOpposite2D(i)));
		}
	}
}

void ChunkMap::changeBlock(const BlockModifiedEvent& blockModification)
{
	glm::ivec2 chunkCoords = Utils::tileToChunkCoord(blockModification.blockCoord, constChunkSize);
	glm::ivec3 localCoords = Utils::globalToLocal(blockModification.blockCoord, constChunkSize);
	if (localCoords.y < 0 || localCoords.y >= constWorldHeight)
		return;
	auto access = m_mapData.getWriteAccess();
	auto chunk = access->regions.find(chunkCoords);
	auto state = access->states.find(chunkCoords);

	if (chunk != access->regions.end() && state->second == ChunkState::GENERATED)
	{
		unsigned int previousId;
		{
			auto chunkAccess = chunk->second.getCenter()->getWriteAccess();
			previousId = chunkAccess.data.getBlockId(localCoords.y, localCoords.x, localCoords.z);
			chunkAccess.data.setBlock(localCoords.y, localCoords.x, localCoords.z, blockModification.blockId);
		}
		coupleChunks(access, chunk->second, chunkCoords);
		m_eventSystemInterface.emit<GameEventTypes::BLOCK_REMESH>(
			BlockRemeshEvent{ blockModification.blockCoord, blockModification.blockId, previousId });

		for (int i = 0; i < constDirectionVectors3DHashed.size(); i++)
		{
			glm::ivec3 adjCoord = blockModification.blockCoord + constDirectionVectors3DHashed[i];
			unsigned int currentId = getBlockId(adjCoord, access);
			m_eventSystemInterface.emit<GameEventTypes::BLOCK_REMESH>(
				BlockRemeshEvent{ adjCoord, currentId, currentId });
		}

		
	}
}