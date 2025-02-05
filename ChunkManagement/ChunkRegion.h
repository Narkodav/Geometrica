#pragma once
#include <atomic>
#include <array>
#include <shared_mutex>

#include "MultiThreading/MemoryPool.h"
#include "ChunkRegionData.h"


class ChunkRegion
{
public:
	using ChunkPtr = ChunkRegionData::ChunkPtr;

private:
	ChunkRegionData m_chunks;
	std::atomic<unsigned int> m_numOfAdjacentChunks = 0;
	mutable std::shared_mutex m_mutex;
public:
	ChunkRegion(ChunkPtr& centerChunk);

	ChunkRegion(const ChunkRegion& other) = delete;
	ChunkRegion& operator=(const ChunkRegion& other) = delete;

	ChunkRegion(ChunkRegion&& other) = default;
	ChunkRegion& operator=(ChunkRegion&& other) = default;

	inline unsigned int getAdjAmount() const
	{
		return m_numOfAdjacentChunks.load();
	}

	ChunkPtr getAdj(Directions2DHashed direction) const;

	ChunkPtr getCenter() const;

	ChunkRegionData getRegionData() const;

	void addAdjacentChunk(ChunkPtr chunk, Directions2DHashed direction);

	void removeAdjacentChunk(Directions2DHashed direction);
};

