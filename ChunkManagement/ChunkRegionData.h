#pragma once
#include <array>
#include "MultiThreading/MemoryPool.h"
#include "Chunk.h"

struct ChunkRegionData
{
public:
	using ChunkPtr = typename MT::MemoryPool<Chunk>::SharedPointer;
	static inline constexpr unsigned int center = constDirectionVectors2DHashed.size();
	std::array<ChunkPtr, center + 1> chunks;

	ChunkRegionData() = default;

	ChunkRegionData(const ChunkRegionData& other) = default;
	ChunkRegionData& operator=(const ChunkRegionData& other) = default;

	ChunkRegionData(ChunkRegionData&& other) = default;
	ChunkRegionData& operator=(ChunkRegionData&& other) = default;

	inline void reset()
	{
		chunks = { nullptr };
	};

	inline ChunkPtr& getAdj(Directions2DHashed direction)
	{
		return chunks[static_cast<unsigned int>(direction)];
	}

	inline ChunkPtr& getCenter()
	{
		return chunks[center];
	}

	inline const ChunkPtr& getAdj(Directions2DHashed direction) const
	{
		return chunks[static_cast<unsigned int>(direction)];
	}

	inline const ChunkPtr& getCenter() const
	{
		return chunks[center];
	}

	inline bool hasAdj(Directions2DHashed direction)
	{
		return chunks[static_cast<unsigned int>(direction)] != nullptr;
	}
};

