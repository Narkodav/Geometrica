#pragma once
#include "GameContext.h"
#include "BlockTemplate.h"
#include "Multithreading/EventSystem.h"
//#include "ChunkManagement/ChunkMap.h"

class BlockModifiedBulkEvent;
class DynamicBlockDataTemplate;

struct BlockData
{
	uint32_t blockId;
	std::unique_ptr<DynamicBlockDataTemplate> dynamicData;
};

// this is a template for the map query that contains the chunk map reference 
// and provides a clear interface of what block updates can do
struct MapUpdateInterface {
	virtual uint32_t getBlockId(glm::ivec3 blockCoords) const = 0;
	virtual std::unique_ptr<DynamicBlockDataTemplate> getBlockData(glm::ivec3 blockCoords) const = 0;
	virtual BlockData getBlock(glm::ivec3 blockCoords) const = 0;
	virtual ~MapUpdateInterface() = default;
};

//every dynamic block must have corresponding dynamic data
// 
//used in the chunk
class DynamicBlockDataTemplate
{
protected:
	//put anything here
	glm::ivec3 m_position;  // Position within chunk
	uint32_t m_id;
	// time is measured in game ticks
	// this parameter is updated manually in update method
	uint32_t m_lastUpdatedTick = 0;
	uint32_t m_nextUpdateScheduledTick = std::numeric_limits<uint32_t>::max();

	const BlockTemplate* m_parentBlockType;

public:
	//DynamicBlockDataTemplate(glm::ivec3 position, uint32_t id) :
	//	m_position(position), m_lastUpdatedTick(0), m_nextUpdateScheduledTick(0) {};
	DynamicBlockDataTemplate(glm::ivec3 position, uint32_t id,
		uint32_t lastUpdatedTick, uint32_t nextUpdateScheduledTick,
		const BlockTemplate* parentBlockType) :
		m_position(position), m_id(id),
		m_lastUpdatedTick(lastUpdatedTick),
		m_nextUpdateScheduledTick(nextUpdateScheduledTick),
		m_parentBlockType(parentBlockType) {};

	virtual std::unique_ptr<DynamicBlockDataTemplate> clone() = 0;

	virtual ~DynamicBlockDataTemplate() = default;

	DynamicBlockDataTemplate(const DynamicBlockDataTemplate&) = default;
	DynamicBlockDataTemplate(DynamicBlockDataTemplate&&) = default;
	DynamicBlockDataTemplate& operator=(DynamicBlockDataTemplate&&) = default;

	// implement any physics updates here
	// if your block state depends on surrounding blocks use the map to check them, 
	// do not use it to actually modify blocks, the event system is used for that

	virtual BlockModifiedBulkEvent update(const MapUpdateInterface& map, const GameClockInterface& clock) = 0;
	virtual void onAdjacentUpdate(const MapUpdateInterface& map, const GameClockInterface& clock) = 0; //sets new update time
	virtual bool canOverride(const std::string& type) { return false; }; //place which updates this block can override, default returns false
	virtual inline bool shouldUpdate(const GameClockInterface& clock) {
		if (clock.getGlobalTime() >= m_nextUpdateScheduledTick)
			return true;
		return false;
	};

	inline uint32_t getLastUpdatedTick() const { return m_lastUpdatedTick; };
	inline uint32_t getNextUpdatedTick() const { return m_nextUpdateScheduledTick; };
	inline const glm::vec3& getPosition() const { return m_position; };
	inline const uint32_t& getId() const { return m_id; };
	virtual std::string getType() const = 0;
	virtual BlockMesherType getMesherType() const = 0; //must correspond to DynamicBlock type, used only for debugging
};

//template<typename DataType>
class DynamicBlockTemplate : public BlockTemplate
{
//protected:
//	static_assert(std::is_base_of<DynamicBlockDataTemplate, DataType>::value,
//		"DataType must inherit from DynamicBlockDataTemplate"); //assert children have DynamicBlockData defined

public:
	virtual ~DynamicBlockTemplate() = default;
	virtual const Phys::Hitbox* getHitbox() const = 0; //make it return nullptr if no collisions
	virtual bool loadFromFile(const std::string& filepath) = 0;
	virtual bool isDynamic() { return true; };
	virtual BlockMesherType getMesherType() const = 0;
	virtual std::string getType() const = 0;

	virtual bool onPlayerPlace(const BlockRaycastResult& result,
		const GameServicesInterface<GameEventPolicy>& interface) const = 0;

	// this is used when getting blocks data for placement
	// you can also use this in the update method to place a block
	// impelemnt tick calculation for block updates (when to update)
	virtual std::unique_ptr<DynamicBlockDataTemplate> getBlockData(const glm::ivec3& blockCoord, uint32_t id,
		const GameClockInterface& clock) const = 0;
};