#include "Camera.h"

Camera::Camera()
{
	this->set({ 0,0,0 }, 0, 0);
}

Camera::Camera(glm::vec3 position, float pitch, float yaw)
{
	this->set(position, pitch, yaw);
}

Camera::Camera(const Camera& camera)
{
	this->m_view = camera.m_view;
	this->m_viewWithoutTranspos = camera.m_viewWithoutTranspos;
	this->m_position = camera.m_position;
	this->m_camFront = camera.m_camFront;
	this->m_camRight = camera.m_camRight;
	this->m_camUp = camera.m_camUp;
	this->m_yaw = camera.m_yaw;
	this->m_pitch = camera.m_pitch;
}

Camera& Camera::operator=(const Camera& camera)
{
	this->m_view = camera.m_view;
	this->m_viewWithoutTranspos = camera.m_viewWithoutTranspos;
	this->m_position = camera.m_position;
	this->m_camFront = camera.m_camFront;
	this->m_camRight = camera.m_camRight;
	this->m_camUp = camera.m_camUp;
	this->m_yaw = camera.m_yaw;
	this->m_pitch = camera.m_pitch;
	return *this;
}

void Camera::set(glm::vec3 position, float pitch, float yaw)
{
	m_pitch = pitch;
	m_yaw = yaw;

	if (m_pitch > glm::radians(89.f))
		m_pitch = glm::radians(89.f);
	if (m_pitch < -glm::radians(89.f))
		m_pitch = -glm::radians(89.f);

	m_position = position;
	m_camFront.x = glm::cos(m_yaw) * glm::cos(m_pitch);
	m_camFront.y = glm::sin(m_pitch);
	m_camFront.z = glm::sin(m_yaw) * glm::cos(m_pitch);
	m_camFront = glm::normalize(m_camFront);
	m_camRight = glm::normalize(glm::cross(m_camFront, 
	glm::vec3(constDirectionVectors3D[static_cast<unsigned int>(Directions::DIRECTION_UP)])));
	m_camUp = glm::normalize(glm::cross(m_camRight, m_camFront));
	m_view = glm::lookAt(m_position, m_position + m_camFront, 
	glm::vec3(constDirectionVectors3D[static_cast<unsigned int>(Directions::DIRECTION_UP)]));
	m_viewWithoutTranspos = glm::mat4(glm::mat3(m_view));
}

void Camera::move(glm::vec3 deltaPos)
{
	m_position += deltaPos;
	m_view = glm::translate(m_view, -deltaPos);
	m_viewWithoutTranspos = glm::mat4(glm::mat3(m_view));
}

void Camera::rotate(float pitch, float yaw)
{
	m_pitch += pitch;
	m_yaw += yaw;
	if (m_pitch > glm::radians(89.f))
		m_pitch = glm::radians(89.f);
	if (m_pitch < -glm::radians(89.f))
		m_pitch = -glm::radians(89.f);

	m_camFront.x = glm::cos(m_yaw) * glm::cos(m_pitch);
	m_camFront.y = glm::sin(m_pitch);
	m_camFront.z = glm::sin(m_yaw) * glm::cos(m_pitch);
	m_camFront = glm::normalize(m_camFront);
	m_camRight = glm::normalize(glm::cross(m_camFront, 
	glm::vec3(constDirectionVectors3D[static_cast<unsigned int>(Directions::DIRECTION_UP)])));
	m_camUp = glm::normalize(glm::cross(m_camRight, m_camFront));

	if (m_camRight.y != 0.0f)
		__debugbreak();
	m_view = glm::lookAt(m_position, m_position + m_camFront, 
	glm::vec3(constDirectionVectors3D[static_cast<unsigned int>(Directions::DIRECTION_UP)]));
	m_viewWithoutTranspos = glm::mat4(glm::mat3(m_view));
}



