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

class Item
{
public:
	struct PropertyFlags
	{
		unsigned char flags = 0;
		static inline const unsigned char PLACEABLE = 1 << 0;		// 00000001
		static inline const unsigned char BLOCK = 1 << 1;			// 00000010
		static inline const unsigned char TRANSPARENT = 1 << 2;		// 00000100
		static inline const unsigned char SOLID = 1 << 3;			// 00001000
		static inline const unsigned char HAS_GRAVITY = 1 << 4;		// 00010000
		static inline const unsigned char LIQUID = 1 << 5;			// 00100000
		static inline const unsigned char BREAKABLE = 1 << 6;		// 01000000
		static inline const unsigned char NEEDS_SUPPORT = 1 << 7;	// 10000000
	};

	//might expand material data with surface properties
	struct ModelMaterialData
	{
		std::vector<std::string> textureNames;
		std::vector<unsigned int> atlasIndices;
	};

	struct CuboidMaterialData
	{
		std::array<unsigned int, 6> atlasIndices;
	};

	uint32_t id = 0; //precisely 32 bits, uses bit masking
	std::string name;
	std::string displayModelName;
	std::string cuboidModelName;
	std::string hitboxName;

	const Cuboid* cuboidModelHandle = nullptr; //used if exists (model is a cuboid), if not - nullptr
	const Model* displayModelHandle = nullptr; //displayed in the inventory (will be implemented) and used in the world if no cuboidModel
	const Phys::Hitbox* hitboxHandle = nullptr; //no hitbox if nullptr

	ModelMaterialData modelMeshMaterials;
	CuboidMaterialData cuboidMeshMaterials;
	std::unordered_map<std::string, unsigned int> meshMaterialsMap;
	PropertyFlags properties;

	void load(std::string itemPath);

	Item() = default;
	Item(const Item& other) = default;
	Item& operator=(const Item& other) = default;
	~Item() = default;

	Item(Item&& other) noexcept = default;
	Item& operator=(Item&& other) noexcept = default;
private:
	std::string getName(std::string filepath);
};

