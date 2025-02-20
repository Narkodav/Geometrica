#pragma once

#include "ChunkMap.h"
#include "Generator.h"
#include "GameContext.h"
#include "PhysicsManager.h"

class ChunkManager
{
public:
    struct ChunkMapQuery : public MapUpdateQueryInterface,
    public PhysicsManager::MapQueryInterface
	{
    private:
        const ChunkMap& chunkMap;

    public:

        explicit ChunkMapQuery(const ChunkMap& map) : chunkMap(map) {}

        ~ChunkMapQuery() = default;

        uint32_t getBlockId(glm::ivec3 blockCoords) const override {
            return chunkMap.getBlockId(blockCoords);
        }

        std::unique_ptr<DynamicBlockDataTemplate> getBlockData(glm::ivec3 blockCoords) const override {
            return chunkMap.getBlockData(blockCoords);
        }

        BlockData getBlock(glm::ivec3 blockCoords) const override {
            return chunkMap.getBlock(blockCoords);
        }
    };

private:
	ChunkMap m_chunkMap;
	glm::ivec2 m_lastRecordedCoords;
	unsigned int m_simulationDistance;

public:
	ChunkManager(const GameContext& gameContext, Generator& generatorHandle, unsigned int m_loadDistance) :
		m_chunkMap(m_loadDistance, generatorHandle, gameContext),
		m_lastRecordedCoords(0, 0) {};


    ChunkMapQuery getMapQuery() const { return ChunkMapQuery(m_chunkMap); };
	
	void updateChunkMap(const glm::ivec2& coords);

    void updateDynamicBlocks(const glm::ivec2& coords);

	unsigned int getGeneratedAmount() { return m_chunkMap.getGeneratedAmount(); };

	unsigned int getAreaAmount() { return m_chunkMap.getAreaAmount(); };

	bool isFullyGenerated() const { return m_chunkMap.isFullyGenerated(); };

	const ChunkMap& getChunkMap() const { return m_chunkMap; };
};