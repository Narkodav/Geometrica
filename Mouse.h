#pragma once
#include "platformCommon.h"

struct Mouse
{
	int coordX, coordY;
	keyState LMB, RMB, scrollWheel;

	//fixed cursor mode
    double accumulatedTime, accumulatedMovementX, accumulatedMovementY, mouseSensitivity;
    float updateInterval = 1.0f / 60.0f;

	//scroll
	double offsetX, offsetY, scrollSensitivity;

    void updateMouseMovement(float deltaX, float deltaY, float deltaTime) {

        accumulatedMovementX += deltaX * mouseSensitivity;
        accumulatedMovementY += deltaY * mouseSensitivity;
        accumulatedTime += deltaTime;
    }

    glm::vec2 getAndResetMovement() {
        glm::vec2 movement(accumulatedMovementX, accumulatedMovementY);
        accumulatedMovementX = 0.0f;
        accumulatedMovementY = 0.0f;
        accumulatedTime = 0.0f;
        return movement;
        return glm::vec2(0.0f, 0.0f);
    }
};

