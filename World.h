#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <unordered_map>
#include <vector>
#include <thread>

#include "Player.h"
#include "Generator.h"
#include "ChunkManagement/ChunkManager.h"

class World
{
private:
	//world data
	int m_loadDistance;
	unsigned int m_worldSeed;
	Player m_player;
	Generator m_generator;
	ChunkManager m_chunkManager;

	//entity data

public:
	World(int loadDistance, unsigned int worldSeed, const Player& player, const GameContext& gameContext);

	void handleInputs(Mouse& mouse, const Keyboard& keyboard, float deltaTime);
	void physicsUpdate(float deltaTime);
	void iterate();

	//const std::unordered_map<glm::ivec2, Chunk> getChunks() const { return m_loadedChunks; };

	const ChunkManager& getChunkManager() const { return m_chunkManager; };
	const Player& getPlayer() const { return m_player; };

};

#endif
