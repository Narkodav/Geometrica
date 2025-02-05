#pragma once
#include "Rendering/Model.h"
#include "Rendering/Camera.h"

struct lightSource
{
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};

struct Scene
{
	std::list<Model> m_models;
	std::vector<lightSource> m_lightSources;
	Camera m_camera;
	glm::vec3 m_ambientLightColor;
	float m_ambientLightIntensity;



};