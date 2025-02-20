#include "ChunkRegion.h"

ChunkRegion::ChunkRegion(ChunkPtr& centerChunk)
{
	m_chunks.getCenter() = centerChunk;
};

ChunkRegion::ChunkPtr ChunkRegion::getAdj(Directions2DHashed direction) const
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	return m_chunks.getAdj(direction);
}

ChunkRegion::ChunkPtr ChunkRegion::getCenter() const
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	return m_chunks.getCenter();
}

ChunkRegionData ChunkRegion::getRegionData() const //makes a copy
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	return m_chunks;
}

void ChunkRegion::addAdjacentChunk(ChunkPtr chunk, Directions2DHashed direction)
{
	std::lock_guard<std::shared_mutex> lock(m_mutex);
	m_numOfAdjacentChunks += !m_chunks.hasAdj(direction);
	m_chunks.getAdj(direction) = chunk;
	auto adjAccess = m_chunks.getAdj(direction)->getReadAccess();
	auto centerAccess = m_chunks.getCenter()->getWriteAccess();
	switch (direction)
	{
	case Directions2DHashed::DIRECTION_FORWARD:
		for (int y = 0; y < constWorldHeight; y++)
			for (int z = 0; z < constChunkSize; z++)
			{
				centerAccess.data.setBlock(y, constChunkSize, z, adjAccess.data.getBlock(y, 0, z));
			}
		break;
	case Directions2DHashed::DIRECTION_BACKWARD:
		for (int y = 0; y < constWorldHeight; y++)
			for (int z = 0; z < constChunkSize; z++)
			{
				centerAccess.data.setBlock(y, -1, z, adjAccess.data.getBlock(y, constChunkSize - 1, z));
			}
		break;
	case Directions2DHashed::DIRECTION_RIGHT:
		for (int y = 0; y < constWorldHeight; y++)
			for (int x = 0; x < constChunkSize; x++)
			{
				centerAccess.data.setBlock(y, x, constChunkSize, adjAccess.data.getBlock(y, x, 0));
			}
		break;
	case Directions2DHashed::DIRECTION_LEFT:
		for (int y = 0; y < constWorldHeight; y++)
			for (int x = 0; x < constChunkSize; x++)
			{
				centerAccess.data.setBlock(y, x, -1, adjAccess.data.getBlock(y, x, constChunkSize - 1));
			}
		break;
	}
}

void ChunkRegion::removeAdjacentChunk(Directions2DHashed direction)
{
	std::lock_guard<std::shared_mutex> lock(m_mutex);
	if (!m_chunks.hasAdj(direction))
		return;
	m_chunks.getAdj(direction) = nullptr;
	m_numOfAdjacentChunks--;
}