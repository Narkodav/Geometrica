#pragma once
#include <vector>
#include <queue>
#include <atomic>
#include <unordered_map>
#include <map>
#include <shared_mutex>
#include <functional>

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/compatibility.hpp>

#include "ChunkManagement/ChunkRegion.h"
#include "MeshData.h"
#include "platformCommon.h"
#include "GameEvents.h"
#include "CuboidMeshData.h"

class ChunkMesh //coords are YXZ for transferring layer by layer
{
private:

	ChunkRegionData m_regionData;

	std::vector<MeshData> m_uniqueMeshesCache; //for non cuboids
	std::map<std::string, unsigned int> m_uniqueMeshesMap; //for easy search

	CuboidMeshData m_cuboidMeshData;

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
	void applyModifications();

	void addBlock(const glm::ivec3& coords, const unsigned int& id);

	void addCuboid(const glm::ivec3& blockCoords, const unsigned int& id, Chunk::ReadAccess& access);

	void replaceBlock(const BlockRemeshEvent& data, Chunk::ReadAccess& access);

	void removeMesh(const glm::ivec3& coord, std::string meshName);

	void addMesh(const glm::ivec3& meshCoords,
		const Mesh& mesh, unsigned int id, unsigned int meshNum);

	void drawCuboids() const; //assumes the shader, atlas and uniforms are bound 
	void drawComplicated() const; //assumes the shader, atlas and uniforms are bound 

	void deleteBuffers();
	void release();

	bool getIsSet() const { return m_isSet; };

	glm::ivec2 getCoords() const { return m_chunkCoords; };

	void notifyBlockModification(const BlockRemeshEvent& data) { m_blockModificationQueue.push(data); };

	friend class TaskLookupTable;
};

