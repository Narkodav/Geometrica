#ifndef GENERATOR_H
#define GENERATOR_H

#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "platformCommon.h"
#include "ChunkManagement/ChunkData.h"
#include "DataManagement/dataRepository.h"
#include "Mathematics/PerlinNoise2d.h"
#include "Mathematics/PerlinNoise3d.h"

#include <random>

//generators are custom made for a specific item set, completely new item set requires a new generator
class Generator
{
	Math::PerlinNoise2d m_perlinNoise2d;
	Math::PerlinNoise3d m_perlinNoise3d1;
	Math::PerlinNoise3d m_perlinNoise3d2;

	enum class blockTypes :unsigned int
	{
		BLOCK_AIR = 0,
		BLOCK_DIRT = 1,
		BLOCK_GRASS = 2,
		BLOCK_STONE = 3,
		BLOCK_NUM,
	};

	unsigned int m_relevantBlockIds[static_cast<unsigned int>(blockTypes::BLOCK_NUM)];

	static const int m_groundLevel = 200;
	unsigned int m_seed;
public:
	Generator();
	Generator(const Generator& other) = default;
	Generator& operator=(const Generator& other) = default;

	void set(unsigned int seed);

	bool shouldBeCave(int x, int y, int z);

	ChunkData getChunkData(glm::ivec2 coords);
};

#endif

