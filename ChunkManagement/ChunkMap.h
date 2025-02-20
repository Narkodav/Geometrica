#pragma once
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <set>
#include <list>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

#include "MultiThreading/MemoryPool.h"
#include "MultiThreading/Synchronized.h"

//#include "ChunkManagementQueue.h"
#include "Utilities/Functions.h"
#include "ChunkRegion.h"

#include "GameEvents.h"
#include "MultiThreading/EventSystem.h"
#include "GameContext.h"
#include "MultiThreading/TaskCoordinator.h"

class ChunkMap
{
public:
	enum class ChunkState
	{
		GENERATED,
		GENERATION_MARKED,
		STORAGE_MARKED,
		DIRECT_MARKED,
		BEING_GENERATED,
		BEING_STORED,
		STORED,
		ERRONEOUS
	};

	static inline const std::map<ChunkState, glm::u8vec4> mapStateColors = {
	{ChunkState::GENERATED,         glm::u8vec4(0,   255, 0,   255)},  // Bright Green - Ready/Complete
	{ChunkState::GENERATION_MARKED, glm::u8vec4(147, 112, 219, 255)},  // Medium Purple - Marked for work
	{ChunkState::STORAGE_MARKED,    glm::u8vec4(255, 140, 0,   255)},  // Dark Orange - Storage pending
	{ChunkState::DIRECT_MARKED,     glm::u8vec4(255, 215, 0,   255)},  // Gold - Direct action needed
	{ChunkState::BEING_GENERATED,   glm::u8vec4(65,  105, 225, 200)},  // Royal Blue, semi-transparent - Active generation
	{ChunkState::BEING_STORED,      glm::u8vec4(70,  130, 180, 200)},  // Steel Blue, semi-transparent - Active storage
	{ChunkState::STORED,            glm::u8vec4(47,  79,  79,  255)},  // Dark Slate Gray - Inactive/stored
	{ChunkState::ERRONEOUS,         glm::u8vec4(220, 20,  60,  255)}   // Crimson - Error state
	};

	using ChunkRegionMap = std::unordered_map<glm::ivec2, ChunkRegion>;
	using ChunkStateMap = std::unordered_map<glm::ivec2, ChunkState>;

	struct ChunkMapData
	{
		ChunkRegionMap regions;
		ChunkStateMap states;

		void clear() { regions.clear(); states.clear(); };
	};

private:
	MT::MemoryPool<Chunk> m_chunkPool;
	MT::TaskCoordinator m_loadingTaskCoordinator;
	MT::TaskCoordinator m_unloadingTaskCoordinator;
	Generator& m_generator;
	MT::EventSystem<GameEventPolicy>& m_eventSystem;
	MT::EventSystem<GameEventPolicy>::Subscription m_blockUpdateSubscription;

	MT::Synchronized<ChunkMapData> m_mapData;

	std::atomic<unsigned int> m_chunkCounter;

	unsigned int m_loadDistance;
	unsigned int m_loadDistanceSquaredWithCushion;
	std::vector<glm::ivec2> m_loadArea; //sorted by distance from center

public:
	ChunkMap(unsigned int loadDistance, Generator& generator,
		const GameContext& gameContext);

	~ChunkMap() {
		MT::ThreadPool& handle = m_loadingTaskCoordinator.getPoolHandle();
		while (handle.queueSize());
		m_mapData.getWriteAccess()->clear();
		while (m_chunkPool.getAllocatedSize())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	};

	ChunkMap(const ChunkMap& other) = delete;
	ChunkMap& operator=(const ChunkMap& other) = delete;

	ChunkMap(ChunkMap&& other) = delete;
	ChunkMap& operator=(ChunkMap&& other) = delete;

	unsigned int iterate(glm::ivec2 positionCurrent);

	void generateChunk(glm::ivec2 coords, Generator& generator);
	void storeChunk(glm::ivec2 coords);
	void coupleChunks(MT::Synchronized<ChunkMapData>::WriteAccess& mapAccess,
		ChunkRegion& region, glm::ivec2 coords);
	void decoupleChunks(MT::Synchronized<ChunkMapData>::WriteAccess& mapAccess,
		ChunkRegion& region, glm::ivec2 coords);
	/*void directStorage(glm::ivec2 coords);*/

	//void tryUpdateMap();

	unsigned int getGeneratedAmount() const { return  m_chunkCounter.load(); };
	unsigned int getAreaAmount() const { return  m_loadArea.size(); };
	bool isFullyGenerated() const { return m_chunkCounter.load() == m_loadArea.size(); };

	uint32_t getBlockId(glm::ivec3 blockCoords) const {
		glm::ivec2 chunkCoords = Utils::tileToChunkCoord(blockCoords, constChunkSize);
		glm::ivec3 localCoords = Utils::globalToLocal(blockCoords, constChunkSize);
		if (localCoords.y < 0 || localCoords.y >= constWorldHeight)
			return DataRepository::airId;
		auto access = m_mapData.getReadAccess();
		auto chunk = access->regions.find(chunkCoords);
		if (chunk != access->regions.end())
			return chunk->second.getCenter()->getReadAccess().data.getBlockId(localCoords.y, localCoords.x, localCoords.z);
		return 0;
	};

	uint32_t getBlockId(glm::ivec3 blockCoords, const MT::Synchronized<ChunkMapData>::WriteAccess& access) const {
		glm::ivec2 chunkCoords = Utils::tileToChunkCoord(blockCoords, constChunkSize);
		glm::ivec3 localCoords = Utils::globalToLocal(blockCoords, constChunkSize);
		if (localCoords.y < 0 || localCoords.y >= constWorldHeight)
			return DataRepository::airId;
		auto chunk = access->regions.find(chunkCoords);
		if (chunk != access->regions.end())
			return chunk->second.getCenter()->getReadAccess().data.getBlockId(localCoords.y, localCoords.x, localCoords.z);
		return 0;
	};

	uint32_t getBlockId(glm::ivec3 blockCoords, const MT::Synchronized<ChunkMapData>::ReadAccess& access) const {
		glm::ivec2 chunkCoords = Utils::tileToChunkCoord(blockCoords, constChunkSize);
		glm::ivec3 localCoords = Utils::globalToLocal(blockCoords, constChunkSize);
		if (localCoords.y < 0 || localCoords.y >= constWorldHeight)
			return DataRepository::airId;
		auto chunk = access->regions.find(chunkCoords);
		if (chunk != access->regions.end())
			return chunk->second.getCenter()->getReadAccess().data.getBlockId(localCoords.y, localCoords.x, localCoords.z);
		return 0;
	};

	std::unique_ptr<DynamicBlockDataTemplate> getBlockData(glm::ivec3 blockCoords) const {
		glm::ivec2 chunkCoords = Utils::tileToChunkCoord(blockCoords, constChunkSize);
		glm::ivec3 localCoords = Utils::globalToLocal(blockCoords, constChunkSize);
		if (localCoords.y < 0 || localCoords.y >= constWorldHeight)
			return nullptr;
		auto access = m_mapData.getReadAccess();
		auto chunk = access->regions.find(chunkCoords);
		if (chunk != access->regions.end())
			return std::move(chunk->second.getCenter()->getReadAccess().data.getBlockData(
				localCoords.y, localCoords.x, localCoords.z));
		return nullptr;
	};

	BlockData getBlock(glm::ivec3 blockCoords) const {
		glm::ivec2 chunkCoords = Utils::tileToChunkCoord(blockCoords, constChunkSize);
		glm::ivec3 localCoords = Utils::globalToLocal(blockCoords, constChunkSize);
		if (localCoords.y < 0 || localCoords.y >= constWorldHeight)
			return{ DataRepository::airId, nullptr };
		auto access = m_mapData.getReadAccess();
		auto chunk = access->regions.find(chunkCoords);
		if (chunk != access->regions.end())
			return chunk->second.getCenter()->getReadAccess().data.getBlock(localCoords.y, localCoords.x, localCoords.z);
		return{ DataRepository::airId, nullptr };
	};

	auto getMapDataAccess() const { return m_mapData.getReadAccess(); };

private:
	void changeBlock(const BlockModifiedEvent& blockModification); //use the event system
};

