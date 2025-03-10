#pragma once
#include <queue>

#include "ChunkMap.h"
#include "Generator.h"
#include "GameContext.h"
#include "PhysicsManager.h"
#include "DataManagement/Blocks/DynamicBlockTemplate.h"
#include "BlockUpdateQueue.h"

class ChunkManager
{
public:
    using Subscription = MT::EventSystem<GameEventPolicy>::Subscription;

    struct ChunkMapIterface : public MapUpdateInterface,
    public PhysicsManager::MapPhysicsInterface
	{
    private:
        const ChunkMap& chunkMap;

    public:

        explicit ChunkMapIterface(const ChunkMap& map) : chunkMap(map) {}

        ~ChunkMapIterface() = default;

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

    BlockUpdateQueue m_blockUpdateQueue;

public:
    ChunkManager(const GameServicesInterface<GameEventPolicy>& gameServicesInterface, Generator& generatorHandle,
    unsigned int m_loadDistance) :
        m_chunkMap(m_loadDistance, generatorHandle, gameServicesInterface),
        m_gameServicesInterface(gameServicesInterface),
        m_lastRecordedCoords(0, 0),
        m_blockUpdateSubscription(gameServicesInterface.subscribe<GameEventTypes::BLOCK_MODIFIED>
            ([this](BlockModifiedEvent&& data)
                { 
                    handleBlockUpdate(std::move(data));
                })),
        m_blockBulkUpdateSubscription(gameServicesInterface.subscribe<GameEventTypes::BLOCK_MODIFIED_BULK>
            ([this](BlockModifiedBulkEvent&& events)
                {
                    for (auto& data : events.modifications)
                        handleBlockUpdate(std::move(data));
                })) {};


    void handleBlockUpdate(BlockModifiedEvent&& data)
    {
        auto cooeds = data.blockCoord;

        //update block
        if (data.newDynamicData != nullptr)
            m_blockUpdateQueue.push(data);
        m_chunkMap.changeBlock(std::move(data));

        //schedule updates for adjacent blocks
        for (int i = 0; i < 6; i++)
        {
            auto adjCoords = cooeds + constDirectionVectors3DHashed[i];
            auto adjData = m_chunkMap.getBlock(adjCoords);
            if (adjData.dynamicData == nullptr)
                continue;
            adjData.dynamicData->onAdjacentUpdate(
                getMapInterface(),
                m_gameServicesInterface);
            BlockModifiedEvent event(adjCoords, adjData.blockId, std::move(adjData.dynamicData));
            m_blockUpdateQueue.push(event);
        }
    }

    //void handleBlockUpdate(BlockModifiedEvent&& data)
    //{
    //    BlockModifiedBulkEvent bulk;
    //    auto cooeds = data.blockCoord;

    //    if (data.newDynamicData != nullptr)
    //    {
    //        auto dataCopy = data.newDynamicData->clone();
    //        m_chunkMap.changeBlock(std::move(data));
    //        bulk = dataCopy->update(
    //            getMapInterface(),
    //            m_gameServicesInterface);
    //    }
    //    else m_chunkMap.changeBlock(std::move(data));

    //    for (int i = 0; i < 6; i++)
    //    {
    //        auto adjCoords = cooeds + constDirectionVectors3DHashed[i];
    //        auto bulkAdj = m_chunkMap.getBlockData(adjCoords)->update(
    //            getMapInterface(),
    //            m_gameServicesInterface);
    //        for (BlockModifiedEvent& ptr : bulkAdj.modifications) {
    //            bulk.modifications.emplace_back(std::move(ptr));
    //        }
    //    }

    //    for (BlockModifiedEvent& ptr : bulk.modifications) {
    //        m_blockUpdateQueue.push(std::move(ptr));
    //    }
    //}

    inline ChunkMapIterface getMapInterface() const { return ChunkMapIterface(m_chunkMap); };
	
	void updateChunkMap(const glm::ivec2& coords);

    void updateDynamicBlocks();

	unsigned int getGeneratedAmount() { return m_chunkMap.getGeneratedAmount(); };

	unsigned int getAreaAmount() { return m_chunkMap.getAreaAmount(); };

	bool isFullyGenerated() const { return m_chunkMap.isFullyGenerated(); };

	const ChunkMap& getChunkMap() const { return m_chunkMap; };
};