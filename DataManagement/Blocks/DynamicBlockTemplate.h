#pragma once
#include "BlockTemplate.h"
#include "Multithreading/EventSystem.h"
#include "GameEvents.h"
//#include "ChunkManagement/ChunkMap.h"

struct BlockData
{
	uint32_t blockId;
	std::unique_ptr<DynamicBlockDataTemplate> dynamicData;
};

// this is a template for the map query that contains the chunk map reference 
// and provides a clear interface of what block updates can do
struct MapUpdateQueryInterface {
	virtual uint32_t getBlockId(glm::ivec3 blockCoords) const = 0;
	virtual std::unique_ptr<DynamicBlockDataTemplate> getBlockData(glm::ivec3 blockCoords) const = 0;
	virtual BlockData getBlock(glm::ivec3 blockCoords) const = 0;
	virtual ~MapUpdateQueryInterface() = default;
};

//every dynamic block must have corresponding dynamic data
// 
//used in the chunk
class DynamicBlockDataTemplate
{
protected:
	//put anything here
	glm::ivec3 m_position;  // Position within chunk
public:
	DynamicBlockDataTemplate(glm::ivec3 position) : m_position(position) {};

	virtual std::unique_ptr<DynamicBlockDataTemplate> clone() = 0;

	virtual ~DynamicBlockDataTemplate() = default;

	//implement any physics updates here
	//if your block state depends on surrounding blocks use the map to check them, do not use it to actually modify blocks, the event system is used for that
	virtual void update(const MapUpdateQueryInterface& map, MT::EventSystem<GameEventPolicy>& gameEvents) = 0;
	virtual BlockMesherType getMesherType() const = 0; //must correspond to DynamicBlock type, used only for debugging
};

template<typename DataType>
class DynamicBlockTemplate : public BlockTemplate
{
	static_assert(std::is_base_of<DynamicBlockDataTemplate, DataType>::value,
		"DataType must inherit from DynamicBlockData"); //assert children have DynamicBlockData defined

public:
	virtual ~DynamicBlockTemplate() = default;
	virtual const Phys::Hitbox* getHitbox() const = 0; //make it return nullptr if no collisions
	virtual bool loadFromFile(const std::string& filepath) = 0;
	virtual bool isDynamic() { return true; };
	virtual BlockMesherType getMesherType() const = 0;
	virtual std::string getType() const = 0;
};