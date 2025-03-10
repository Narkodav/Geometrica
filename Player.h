#ifndef PLAYER_H
#define PLAYER_H

#pragma once
#include <iostream>
#include <shared_mutex>

#include "Rendering/Camera.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "PhysicsManager.h"
#include "Utilities/Functions.h"
#include "GameContext.h"
#include "GameEvents.h"
#include "Multithreading/EventSystem.h"
#include "DataManagement/Blocks/DynamicBlockTemplate.h"
#include "DataManagement/Blocks/DynamicBlockDataFactory.h"

class Player
{
private:
	Camera m_eye;
	glm::ivec2 m_playerChunkCoords;
	glm::vec3 m_playerCoords; // at the bottom, 0.5 meters from the ground
	glm::vec3 m_acceleration;
	glm::vec3 m_velocity;
	float m_walkingSpeed = 0.5; //meters per sec
	float m_runningSpeed = 1; //meters per sec
	float m_flyingSpeed = 2; //meters per sec
	float m_upDownSpeed = 8.f; //meters per sec
	float m_accelerationCoefficient = 8.f; //meters per sec squared
	float m_friction = 0.6f; // 1/sec
	float m_handReach = 5.f;
	bool m_flightMode;
	BlockRaycastResult m_raycastResult;
	Area m_selectedArea;
	
public:
	Player() = default;

	Player(float yaw, float pitch, const glm::vec3& playerPosition, const glm::vec3& speed);
	void set(float yaw, float pitch, const glm::vec3& playerPosition, const glm::vec3& speed);

	Player(const Player&) = default;
	Player& operator=(const Player&) = default;

	Player(Player&&) = default;
	Player& operator=(Player&&) = default;

	void handleInputs(const Mouse& mouse, const Keyboard& keyboard,
		GameServicesInterface<GameEventPolicy> interface);
	void handleMouseMove(Mouse& mouse, float delta);

	void update(float deltaTime, const PhysicsManager::MapPhysicsInterface& chunkMap);

	//getters

	const glm::ivec2& getChunkCoords() const { return m_playerChunkCoords; };
	const glm::vec3& getPosition() const { return m_playerCoords; };
	const glm::vec3& getVelocity() const { return m_velocity; };
	const glm::vec3& getAcceleration() const { return m_acceleration; };
	const glm::mat4& getView() const { return glm::translate(m_eye.getView(), -m_playerCoords); };
	const glm::mat4& getViewWithoutTranspos() const { return m_eye.getViewWithoutTranspos(); };
	const BlockRaycastResult& getRaycastResult() const { return m_raycastResult; };
	const Area& getSelectedArea() const { return m_selectedArea; };
	/*friend class PhysicsManager;*/
};

#endif

