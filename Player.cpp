#include "Player.h"

Player::Player(GameContext gameContext) : m_gameContext(gameContext)
{

}

Player::Player(float yaw, float pitch, const glm::vec3& playerPosition, const glm::vec3& speed, GameContext gameContext)
	: m_gameContext(gameContext)
{
	set(yaw, pitch, playerPosition, speed);
}

void Player::set(float yaw, float pitch, const glm::vec3& playerPosition, const glm::vec3& speed)
{
	m_eye.set(glm::vec3(0, 1, 0), pitch, yaw);
	m_velocity = speed;
	m_playerCoords = playerPosition;
	m_playerChunkCoords.x = m_playerCoords.x / constChunkSize;
	m_playerChunkCoords.y = m_playerCoords.z / constChunkSize;
}

void Player::handleInputs(const Mouse& mouse, const Keyboard& keyboard)
{
	glm::vec3 buffer = glm::vec3(0.f);

	if (keyboard.m_keys[KEY_W].state)
	{
		buffer += m_eye.getCamForward();
	}
	if (keyboard.m_keys[KEY_S].state)
	{
		buffer -= m_eye.getCamForward();
	}

	if (keyboard.m_keys[KEY_D].state)
	{
		buffer += m_eye.getCamRight();
	}
	if (keyboard.m_keys[KEY_A].state)
	{
		buffer -= m_eye.getCamRight();
	}

	m_velocity.y = 0;
	if (keyboard.m_keys[KEY_SPACE].state)
	{
		m_velocity.y += m_upDownSpeed;
	}
	if (keyboard.m_keys[KEY_SHIFT].state)
	{
		m_velocity.y -= m_upDownSpeed;
	}

	if(buffer.x != 0 || buffer.y != 0 || buffer.z != 0)
		buffer = glm::normalize(buffer);

	m_acceleration = buffer * m_accelerationCoefficient;

	if (mouse.LMB.isChanged && mouse.LMB.state)
	{
		//hand animation...
		if (m_raycastResult.hit)
		{
			m_gameContext.gameEvents.emit<GameEventTypes::BLOCK_MODIFIED>(
				BlockModifiedEvent{ m_raycastResult.blockPos, DataRepository::airId });
		}
	}
	else if (mouse.RMB.isChanged && mouse.RMB.state)
	{
		//hand animation...
		if (m_raycastResult.hit)
		{
			m_gameContext.gameEvents.emit<GameEventTypes::BLOCK_MODIFIED>(
				BlockModifiedEvent{ m_raycastResult.blockPos + glm::ivec3(m_raycastResult.hitNormal),
				DataRepository::getBlock("stone_upper_half")->getId() });
		}
	}


}

void Player::handleMouseMove(Mouse& mouse, float delta)
{
	glm::vec2 mouseMovement = mouse.getAndResetMovement();
	if (mouseMovement.x != 0 || mouseMovement.y != 0)
		m_eye.rotate(-mouseMovement.y, mouseMovement.x);

}

void Player::update(float deltaTime, const ChunkMap& chunkMap)
{
	m_acceleration -= m_velocity * m_friction;
	glm::vec3 deltaPos = m_velocity * deltaTime + m_acceleration * deltaTime * deltaTime * 0.5f;

	/*deltaPos = PhysicsManager::handlePlayerCollisions(m_playerCoords, deltaPos, chunkMap);*/

	if (deltaPos.x == 0) m_velocity.x = 0;
	if (deltaPos.z == 0) m_velocity.z = 0;

	m_playerCoords += deltaPos;

	m_velocity += m_acceleration * deltaTime;

	m_velocity.y = 0;
	m_playerChunkCoords = Utils::tileToChunkCoord(m_playerCoords, constChunkSize);
	m_raycastResult = PhysicsManager::raycastBlock(
		{ m_playerCoords + m_eye.getPosition(), m_eye.getCamFront() }, m_handReach, chunkMap);
	if (m_raycastResult.hit)
	{
		m_selectedArea.corner = m_raycastResult.blockPos;
		m_selectedArea.sizes = glm::ivec3(1);
	}
	else m_selectedArea = { glm::ivec3(0), glm::ivec3(0) };
}