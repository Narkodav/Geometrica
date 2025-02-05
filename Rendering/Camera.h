#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>

#include "platformCommon.h"

class Camera
{
private:
	glm::mat4 m_view;
	glm::mat4 m_viewWithoutTranspos;
	glm::vec3 m_position;
	glm::vec3 m_camFront, m_camRight, m_camUp;
	float m_yaw, m_pitch;
public:

	Camera();
	Camera(glm::vec3 position, float pitch, float yaw);
	Camera(const Camera& camera);
	Camera& operator=(const Camera& camera);

	void set(glm::vec3 position, float pitch, float yaw);
	void move(glm::vec3 deltaPos);
	void rotate(float pitch, float yaw);

	//getters
	const glm::mat4& getView() const { return m_view; };
	const glm::mat4& getViewWithoutTranspos() const { return m_viewWithoutTranspos; };
	const glm::mat3& getRotation() const { return glm::mat3(m_view); };
	const glm::vec3& getPosition() const { return m_position; };
	const glm::vec3& getCamFront() const { return m_camFront; };
	const glm::vec3& getCamRight() const { return m_camRight; };
	const glm::vec3& getCamUp() const { return m_camUp; };
	glm::vec3 getCamForward() const { return glm::vec3(glm::cos(m_yaw), 0, glm::sin(m_yaw)); };
};

