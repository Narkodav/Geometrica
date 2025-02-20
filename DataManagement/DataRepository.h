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
#include "Blocks/BlockFactory.h"
#include "Cuboid.h"
//#include "Item.h"


class DataRepository
{
	friend class Block;
	//friend class Item;
public:
	//first byte of an item id defines the mesh type (cube, flat plane, custom, etc)
	//this is the only mesh items with that first byte in id can have
	//this is needed for optimization during mesh building
	//those masks are generated automatically based on the mesh, so the id is technically only 3 bytes (or 2^24 items per model type)
	static inline const unsigned int modelPropertiesTypeMask =  0b11111111'00000000'00000000'00000000;
	static inline const unsigned int airId = 0; //air has a reserved id of 0
	static inline const unsigned int hasCuboidModelIdMask =		0b00000001'00000000'00000000'00000000;
	static inline const unsigned int hasDisplayModelIdMask =	0b00000010'00000000'00000000'00000000;
	static inline const unsigned int hasPhysicsMask =			0b00000100'00000000'00000000'00000000;

private:
	static inline std::map<uint32_t, std::unique_ptr<BlockTemplate>> m_blocks;
	static inline std::map<std::string, unsigned int> m_blockNameMap;
	static inline std::map<std::string, Model> m_models;
	static inline std::map<std::string, Cuboid> m_cuboids;
	static inline std::map<std::string, Texture> m_textures;
	static inline std::map<std::string, std::unique_ptr<Phys::Hitbox>> m_hitboxes;
	static inline TextureAtlas m_textureAtlas;
	static inline std::string m_resourcesFilepath;

	static void readPaths(std::vector<std::string>& container, const std::string& path);
	
	static void setBlocks();
	static void setModels();
	static void setCuboids();
	static void setTextures();
	static void setHitboxes();

public:
	static void set(std::string resourcesFilepath);

	static inline bool hasCuboidModel(unsigned int id) { return (id & hasCuboidModelIdMask) != 0; };
	static inline bool hasDisplayModel(unsigned int id) { return (id & hasDisplayModelIdMask) != 0; };

	static const std::unique_ptr<BlockTemplate>& getBlock(unsigned int id) { return m_blocks.find(id)->second; };
	static const std::unique_ptr<BlockTemplate>& getBlock(std::string name) { return m_blocks.find(m_blockNameMap.find(name)->second)->second; };
	static const std::map<unsigned int, std::unique_ptr<BlockTemplate>>& getBlocks() { return m_blocks; };
	static const Model& getModel(const std::string& modelName) { return m_models.find(modelName)->second; };
	static const Cuboid& getCuboid(const std::string& cuboidName) { return m_cuboids.find(cuboidName)->second; };
	static const Texture& getTexture(const std::string& textureName) { return m_textures.find(textureName)->second; };
	static const TextureAtlas& getAtlas() { return m_textureAtlas; };
	static const Phys::Hitbox& getHitbox(const std::string& hitboxName) { return *m_hitboxes.find(hitboxName)->second.get(); };
	/*const Texture* getTextureRef(const std::string& textureName) const { return &m_textures[textureName]; };*/
};

