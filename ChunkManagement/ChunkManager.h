#pragma once
#include <queue>

#include "ChunkMap.h"
#include "Generator.h"
#include "GameContext.h"
#include "PhysicsManager.h"
#include "DataManagement/Blocks/DynamicBlockTemplate.h"

class ChunkManager
{
public:
    using Subscription = MT::EventSystem<GameEventPolicy>::Subscription;

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
    GameServicesInterface<GameEventPolicy> m_gameServicesInterface;

    Subscription m_blockUpdateSubscription;
    Subscription m_blockBulkUpdateSubscription;

    /*std::unordered_set<glm::ivec3> m_blockUpdateQueue;*/

public:
    ChunkManager(const GameServicesInterface<GameEventPolicy>& gameServicesInterface, Generator& generatorHandle,
    unsigned int m_loadDistance) :
        m_chunkMap(m_loadDistance, generatorHandle, gameServicesInterface),
        m_gameServicesInterface(gameServicesInterface),
        m_lastRecordedCoords(0, 0),
        m_blockUpdateSubscription(gameServicesInterface.subscribe<GameEventTypes::BLOCK_MODIFIED>
            ([this](BlockModifiedEvent&& data)
                { 
                    m_chunkMap.changeBlock(std::move(data));
                })),
        m_blockBulkUpdateSubscription(gameServicesInterface.subscribe<GameEventTypes::BLOCK_MODIFIED_BULK>
            ([this](BlockModifiedBulkEvent&& events)
                {
                    for (auto& data : events.modifications)
                        m_chunkMap.changeBlock(std::move(data));
                })) {};


    ChunkMapQuery getMapQuery() const { return ChunkMapQuery(m_chunkMap); };
	
	void updateChunkMap(const glm::ivec2& coords);

    void updateDynamicBlocks();

	unsigned int getGeneratedAmount() { return m_chunkMap.getGeneratedAmount(); };

	unsigned int getAreaAmount() { return m_chunkMap.getAreaAmount(); };

	bool isFullyGenerated() const { return m_chunkMap.isFullyGenerated(); };

	const ChunkMap& getChunkMap() const { return m_chunkMap; };
};