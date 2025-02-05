#pragma once
#include <vector>
#include <unordered_map>
#include <set>

#include "GameEvents.h"
#include "MultiThreading/EventSystem.h"
#include "Rendering/ChunkMesh.h"
#include "Rendering/Skybox.h"
#include "logger/GlobalParamsLogger.h"
#include "MultiThreading/MemoryPool.h"
#include "MultiThreading/Synchronized.h"
#include "GameContext.h"
#include "ChunkManagement/ChunkMap.h"
#include "Rendering/MeshUpdateQueue.h"
#include "MultiThreading/TaskCoordinator.h"
#include "Utilities/UniqueOrderedQueue.h"
#include "MultiThreading/ThreadPool.h"
#include "Rendering/BlockHighlight.h"

class WorldMesh
{
public:

	struct BufferQueueElem
	{
		glm::ivec2 coord;
		MT::MemoryPool<MT::Synchronized<ChunkMesh>>::SharedPointer meshPtr;

		struct Comparator
		{
			bool operator()(const BufferQueueElem& a,
				const BufferQueueElem& b) const
			{
				ComparatorIvec2 comp;
				return comp(a.coord, b.coord);
			}
		};

		bool operator==(const BufferQueueElem& other) const {
			return coord == other.coord;
		}

		struct Hash {
			size_t operator()(const BufferQueueElem& elem) const {
				// Combine hash of x and y coordinates
				size_t h1 = std::hash<int>{}(elem.coord.x);
				size_t h2 = std::hash<int>{}(elem.coord.y);
				return h1 ^ (h2 << 1);
			}
		};
	};

	using BufferQueue = MT::Synchronized<Utils::UniqueOrderedQueue<BufferQueueElem,
		BufferQueueElem::Comparator, BufferQueueElem::Hash>>;

	enum class MeshState
	{
		BEING_MESHED,
		BEING_UNMESHED,
		MARKED_FOR_BUFFER_LOAD,
		MARKED_FOR_BUFFER_UNLOAD,
		MESHED,
	};

	using MeshMap = std::unordered_map<glm::ivec2,
		MT::MemoryPool<MT::Synchronized<ChunkMesh>>::SharedPointer>;
	using MeshStateMap = std::unordered_map<glm::ivec2, MeshState>;

	struct WorldMeshData
	{
		MeshMap meshes;
		MeshStateMap states;

		void clear() { meshes.clear(); states.clear(); };
	};

private:
	static inline const std::chrono::duration<double, std::micro> bufferLoadTimeLimit{ 20 };
	static inline const std::chrono::duration<double, std::micro> bufferUnloadTimeLimit{ 20 };

	MT::EventSystem<GameEventPolicy>& m_eventSystem;
	MT::EventSystem<GameEventPolicy>::Subscription m_blockUpdateSubscription;

	MT::TaskCoordinator m_taskMeshCoordinator;
	MT::TaskCoordinator m_taskUnmeshCoordinator;

	MT::MemoryPool<MT::Synchronized<ChunkMesh>> m_chunkMeshPool;

	MT::Synchronized<WorldMeshData> m_meshData;
	const ChunkMap& m_chunkMap;

	Skybox m_skybox;
	std::optional<BlockHighlight> m_blockHighlight;

	BufferQueue m_bufferLoadQueue;
	BufferQueue m_bufferUnloadQueue;

	std::unordered_set<glm::ivec2> m_chunksToModify;

	std::atomic<unsigned int> m_numOfMeshes;
	std::atomic<unsigned int> m_numOfLoadedMeshes;

	std::vector<glm::ivec2> m_renderArea;
	unsigned int m_renderAreaRadius;
	unsigned int m_renderAreaRadiusWithPaddingSquared;
public:

	WorldMesh(int renderAreaRadius, GameContext gameContext, const ChunkMap& chunkMap);

	~WorldMesh() { 
		MT::ThreadPool& handle = m_taskMeshCoordinator.getPoolHandle();
		while (handle.queueSize());
		m_bufferLoadQueue.getWriteAccess()->clear();
		m_bufferUnloadQueue.getWriteAccess()->clear();
		m_meshData.getWriteAccess()->clear();
		while(m_chunkMeshPool.getAllocatedSize()) 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	};

	void set();

	void meshChunk(ChunkRegionData chunkData);
	void unmeshChunk(glm::ivec2 coords);

	void iterate(glm::ivec2 newCoords);

	void processBuffers();

	void drawChunks(Shader& shaderCuboid, Shader& shaderBlock, glm::mat4 view, glm::mat4 projection);
	void drawSkybox(Shader& shader, glm::mat4 viewWithoutTranspos, glm::mat4 projection);
	void drawBlockHighlight(Shader& shader, glm::mat4 view, glm::mat4 projection, Area selectedArea);

	void blockModified(BlockRemeshEvent data);
};

