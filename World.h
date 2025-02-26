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
#include "GameContext.h"

class World
{
private:
	//world data
	int m_loadDistance;
	unsigned int m_worldSeed;
	Player m_player;
	Generator m_generator;
	ChunkManager m_chunkManager;
	GameServicesInterface<GameEventPolicy> m_interface;
	//entity data

public:
	World(int loadDistance, unsigned int worldSeed,
		const Player& player, const GameServicesInterface<GameEventPolicy>& gameServicesInterface);

	void handleInputs(Mouse& mouse, const Keyboard& keyboard, float deltaTime);
	void physicsUpdate(float deltaTime);
	void iterate();

	//const std::unordered_map<glm::ivec2, Chunk> getChunks() const { return m_loadedChunks; };

	ChunkManager& getChunkManager() { return m_chunkManager; };
	const ChunkManager& getChunkManager() const { return m_chunkManager; };
	const Player& getPlayer() const { return m_player; };

};

#endif
