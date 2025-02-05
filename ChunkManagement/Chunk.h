#pragma once
#include <shared_mutex>
#include <functional>
#include <array>
#include <atomic>

#include "ChunkData.h"
#include "Generator.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

class Chunk
{
public:
	struct WriteAccess
	{
		WriteAccess(ChunkData& data, std::shared_mutex& mutex) : data(data), lock(mutex) {};
		ChunkData& data;
	private:
		std::lock_guard<std::shared_mutex> lock;
	};

	struct ReadAccess
	{
		ReadAccess(const ChunkData& data, std::shared_mutex& mutex) : data(data), lock(mutex) {};
		const ChunkData& data;
	private:
		std::shared_lock<std::shared_mutex> lock;
	};

private:
	mutable std::shared_mutex m_mutex;
	ChunkData m_data;

public:
	
	Chunk() = default;
	~Chunk() = default;

	Chunk(const Chunk& other) = delete;
	Chunk& operator=(const Chunk& other) = delete;

	Chunk(Chunk&& other) = delete;
	Chunk& operator=(Chunk&& other) = delete;

	void generateChunk(const glm::ivec2& coords, Generator& generator);
	void storeChunk();

	inline WriteAccess getWriteAccess()
	{
		return WriteAccess(m_data, m_mutex);
	}

	inline ReadAccess getReadAccess() const
	{
		return ReadAccess(m_data, m_mutex);
	}

	inline glm::ivec2 getPosition() const
	{
		std::shared_lock<std::shared_mutex> lock(m_mutex);
		return m_data.getCoords();
	}

};

