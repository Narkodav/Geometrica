#pragma once

#include "ChunkMap.h"
#include "Generator.h"
#include "GameContext.h"

class ChunkManager
{
private:
	ChunkMap m_chunkMap;
	glm::ivec2 m_lastRecordedCoords;

	MT::EventSystem<GameEventPolicy>::Subscription m_blockUpdateSubscription;

public:
	ChunkManager(const GameContext& gameContext, Generator& generatorHandle, unsigned int m_loadDistance) :
		m_chunkMap(m_loadDistance, generatorHandle, gameContext),
		m_lastRecordedCoords(0, 0),
		m_blockUpdateSubscription(gameContext.gameEvents.subscribe<GameEventTypes::BLOCK_MODIFIED>
			([this](BlockModifiedEvent data)
				{ m_chunkMap.changeBlock(data.blockCoord, data.blockId); })) {};

	
	void iterate(glm::ivec2 newCoords);

	unsigned int getGeneratedAmount() { return m_chunkMap.getGeneratedAmount(); };

	unsigned int getAreaAmount() { return m_chunkMap.getAreaAmount(); };

	bool isFullyGenerated() const { return m_chunkMap.isFullyGenerated(); };

	const ChunkMap& getChunkMap() const { return m_chunkMap; };
};