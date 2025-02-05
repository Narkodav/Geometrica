#pragma once
#include <vector>
#include <unordered_map>
#include <shared_mutex>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/hash.hpp"

#include "ChunkManagement/Chunk.h"
#include "MeshData.h"

class ChunkBorderMesh
{
public:
	struct ChunkBorderIdentifier
	{
	private:
		glm::ivec2 coordMin;
		glm::ivec2 coordMax;
	public:
		ChunkBorderIdentifier() = default;
		~ChunkBorderIdentifier() = default;

		ChunkBorderIdentifier(const ChunkBorderIdentifier& other) = default;
		ChunkBorderIdentifier(ChunkBorderIdentifier&& other) = default;

		ChunkBorderIdentifier& operator=(const ChunkBorderIdentifier& other) = default;
		ChunkBorderIdentifier& operator=(ChunkBorderIdentifier&& other) = default;

		ChunkBorderIdentifier(glm::ivec2 a, glm::ivec2 b) {
			if (a.x == b.x)
			{
				coordMin.x = a.x;
				coordMax.x = a.x;
				coordMin.y = std::min(a.y, b.y);
				coordMax.y = std::max(a.y, b.y);
			}
			else if (a.y == b.y)
			{
				coordMin.y = a.y;
				coordMax.y = a.y;
				coordMin.x = std::min(a.x, b.x);
				coordMax.x = std::max(a.x, b.x);
			}
			else __debugbreak();
		};

		bool operator==(const ChunkBorderIdentifier& other) const {
			return coordMin == other.coordMin && coordMax == other.coordMax;
		}

		size_t hash() const
		{
			return std::hash<glm::ivec2>()(coordMin) ^
				(std::hash<glm::ivec2>()(coordMax) << 1);
		}
	};
private:
	const Chunk* m_minChunkHandle;
	const Chunk* m_maxChunkHandle;

	MeshData m_furtherSide; //for faces from the bigger coordinate chunk m_maxChunkHandle
	MeshData m_closerSide; //for faces from the smaller coordinate chunk m_minChunkHandle

	bool m_isSet = 0;
	std::shared_mutex m_borderMutex;

	void fillChunkHorizontal(); //when x are equal
	void fillChunkVertical(); //when y are equal


public:
	bool m_isBeingUsed = 0;

	void set(const Chunk& chunkA, const Chunk& chunkB);
	void loadBuffers();
	void deleteBuffers();
	void draw();

	bool getIsSet() const { return m_isSet; };

	ChunkBorderMesh();
	~ChunkBorderMesh();

	ChunkBorderMesh(const ChunkBorderMesh& other);

	ChunkBorderMesh& operator=(const ChunkBorderMesh& other);
};

namespace std
{
	template<>
	struct hash<ChunkBorderMesh::ChunkBorderIdentifier>
	{
		size_t operator()(const ChunkBorderMesh::ChunkBorderIdentifier& id) const
		{
			return id.hash();
		}
	};
}