#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <variant>

#include "Rendering/Model.h"
#include "Rendering/TextureAtlas.h"
#include "Physics/Hitboxes.h"
#include "platformCommon.h"
#include "Cuboid.h"
#include "BlockTemplate.h"
#include "BlockFactory.h"

class CuboidBlock : public BlockTemplate //block that uses cuboids for models, use this as a template for cuboid blocks types
{
public:
	struct CuboidMaterialData
	{
		std::array<unsigned int, 6> atlasIndices;
	};

private:
	std::string m_cuboidName;
	std::string m_hitboxName;

	const Cuboid* m_cuboidHandle = nullptr;
	const Phys::Hitbox* m_hitboxHandle = nullptr; //no hitbox if nullptr

	CuboidMaterialData m_cuboidMeshMaterials;

public:
	bool loadFromFile(const std::string& filepath) override;

	virtual const Cuboid& getCuboid() const { return *m_cuboidHandle; };
	virtual const Phys::Hitbox* getHitbox() const override { return m_hitboxHandle; };
	virtual const unsigned int& getMaterialAtlasIndex(unsigned int rotation) const
	{
		return m_cuboidMeshMaterials.atlasIndices[rotation];
	};

private:
	bool loadCuboidData(const std::string& cuboidName);
	bool loadHitboxData(const std::string& hitboxName);
	bool setFaceMaterial(int faceIndex, const std::string& materialName);
};

