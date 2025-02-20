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
#include "DataManagement/Cuboid.h"
#include "BlockTemplate.h"
#include "BlockFactory.h"

class CuboidBlock : public BlockTemplate //block that uses cuboids for models, use this as a template for cuboid blocks types
{
	static inline BlockRegistrar<CuboidBlock> registrar{ "CuboidBlock" };
public:
	struct CuboidMaterialData
	{
		std::array<unsigned int, 6> atlasIndices;
	};

private:
	std::string m_cuboidName;
	const Cuboid* m_cuboidHandle = nullptr;

	CuboidMaterialData m_cuboidMeshMaterials;

public:
	bool loadFromFile(const std::string& filepath) override;

	virtual const Cuboid& getCuboid() const { return *m_cuboidHandle; };
	
	virtual const unsigned int& getMaterialAtlasIndex(unsigned int rotation) const
	{
		return m_cuboidMeshMaterials.atlasIndices[rotation];
	};

	virtual BlockMesherType getMesherType() const { return BlockMesherType::MESHING_CUBOID; };
	virtual std::string getType() const { return "CuboidBlock"; };

private:
	bool loadCuboidData(const std::string& cuboidName);
	bool setFaceMaterial(int faceIndex, const std::string& materialName);
};

