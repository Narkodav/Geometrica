#pragma once
#include <vector>
#include <queue>
#include <atomic>
#include <unordered_map>
#include <map>
#include <shared_mutex>
#include <functional>
#include <array>

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/compatibility.hpp>

#include "ChunkManagement/ChunkRegion.h"
#include "platformCommon.h"
#include "GameEvents.h"
#include "CuboidMesher.h"

class ChunkMesh //coords are YXZ for transferring layer by layer
{
private:

	ChunkRegionData m_regionData;

	//std::vector<MeshData> m_uniqueMeshesCache; //for non cuboids
	//std::map<std::string, unsigned int> m_uniqueMeshesMap; //for easy search

	//CuboidMeshData m_cuboidMeshData;

	std::array<std::unique_ptr<MesherTemplate>, 
	static_cast<unsigned int>(BlockMesherType::MESHING_NUM)> m_meshers;

	glm::ivec2 m_chunkCoords;

	std::queue<BlockRemeshEvent> m_blockModificationQueue;
	
	bool m_isSet = 0;

public:
	
	bool m_isBeingUsed = 0;

	ChunkMesh();
	~ChunkMesh();

	ChunkMesh(const ChunkMesh& other) = delete;
	ChunkMesh& operator=(const ChunkMesh& other) = delete;

	void set(ChunkRegionData& regionData);
	void loadBuffers(unsigned int offset = 0);
	void applyUpdates();

	void replaceBlock(const BlockRemeshEvent& data, Chunk::ReadAccess& access);

	void draw(BlockMesherType mesherType) const;

	void deleteBuffers();
	void release();

	bool getIsSet() const { return m_isSet; };

	glm::ivec2 getCoords() const { return m_chunkCoords; };

	void notifyBlockUpdate(const BlockRemeshEvent& data) { m_blockModificationQueue.push(data); };

	friend class TaskLookupTable;
};

