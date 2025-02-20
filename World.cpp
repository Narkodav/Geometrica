#include "World.h"

World::World(int loadDistance, unsigned int worldSeed, 
	const Player& player, const GameContext& gameContext) :
	m_loadDistance(loadDistance), m_worldSeed(worldSeed), m_player(player), 
	m_chunkManager(gameContext, m_generator, m_loadDistance)
{
	m_generator.set(m_worldSeed);
};

void World::handleInputs(Mouse& mouse, const Keyboard& keyboard, float deltaTime)
{
	m_player.handleInputs(mouse, keyboard);
	m_player.handleMouseMove(mouse, deltaTime);
	
}

void World::physicsUpdate(float deltaTime)
{
	m_player.update(deltaTime, m_chunkManager.getMapQuery());
	/*m_chunkManager.update(m_player.getChunkCoords());*/
}

void World::iterate()
{
	m_chunkManager.updateChunkMap(m_player.getChunkCoords());
}