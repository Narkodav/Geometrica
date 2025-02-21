#pragma once
#include <memory>

#include "GameEvents.h"
#include "DynamicBlockDataFactory.h"
#include "BlockFactory.h"
#include "Utilities/PathFinder.h"
#include "Utilities/PathFinderContext.h"
#include "Multithreading/EventSystem.h"


class LiquidBlockData : public DynamicBlockDataTemplate
{
public:
	class LiquidBlockPathFinderContext :
		public Utils::PathFinderContext<glm::ivec3, float, std::vector<glm::ivec3>, LiquidBlockPathFinderContext>
	{
	private:
		std::function<std::vector<glm::ivec3>(const glm::ivec3&)> m_getAvailableDirections;

	public:
		using PathFinderContextBase = PathFinderContext<glm::ivec3, float, std::vector<glm::ivec3>, LiquidBlockPathFinderContext>;
		using Node = typename PathFinderContextBase::Node;
		using Cost = typename PathFinderContextBase::Cost;
		using Path = typename PathFinderContextBase::Path;

		struct NodeHash {
			size_t operator()(const Node& node) const {
				// Custom hash for glm::ivec3
				return std::hash<int>()(node.x) ^
					std::hash<int>()(node.y) ^
					std::hash<int>()(node.z);
			}
		};

		LiquidBlockPathFinderContext(auto getAvailableDirections) :
			m_getAvailableDirections(getAvailableDirections) {};

		virtual bool isGoal(const Node& current, const Node& goal) const override {
			return current == goal;
		}

		virtual Cost estimateCost(const Node& from, const Node& to) const override {
			glm::ivec3 result = glm::abs(to - from);
			return result.x + result.y + result.z;
		}

		virtual std::vector<Node> getNeighbors(const Node& node) const override {
			return m_getAvailableDirections(node);
		}

		virtual Cost getCost(const Node& from, const Node& to) const override {
			return glm::length(glm::vec3(to - from));
		}

		virtual void addToPath(Path& path, const Node& node) const override {
			path.push_back(node);
		}
	};

	static inline const size_t directionsForFlowing[] = {
static_cast<size_t>(Directions3DHashed::DIRECTION_FORWARD),
static_cast<size_t>(Directions3DHashed::DIRECTION_RIGHT),
static_cast<size_t>(Directions3DHashed::DIRECTION_BACKWARD),
static_cast<size_t>(Directions3DHashed::DIRECTION_LEFT) };

protected:
	static inline DynamicBlockDataRegistrar<LiquidBlockData> registrar{ "LiquidBlock" };
	static inline const unsigned char sourceLevel = 14; // in pixels (16 max)
	static inline const unsigned char levelDecline = 2; // in pixels 7 levels total
	static inline const unsigned char directFlowDistanceThreshold = 4; // water will flow directly with accordance to the shortest path unless there are no paths shorter than this threshold
	unsigned char m_currentLevel;
	bool m_isSource;

public:
	//LiquidBlockData(glm::ivec3 coord) :
	//	DynamicBlockDataTemplate(coord), m_currentLevel(sourceLevel), m_isSource(true) {};

	LiquidBlockData(glm::ivec3 coord, uint32_t lastUpdatedTick, uint32_t nextUpdateScheduledTick) :
		DynamicBlockDataTemplate(coord, lastUpdatedTick, nextUpdateScheduledTick), m_currentLevel(sourceLevel), m_isSource(true) {};

	LiquidBlockData(glm::ivec3 coord, uint32_t lastUpdatedTick, uint32_t nextUpdateScheduledTick, bool isSource, unsigned char level) :
		DynamicBlockDataTemplate(coord, lastUpdatedTick, nextUpdateScheduledTick), m_currentLevel(level), m_isSource(isSource) {};

	virtual ~LiquidBlockData() override = default;
	virtual BlockModifiedBulkEvent update(const MapUpdateQueryInterface& map, const GameClockInterface& clock) override;
	BlockModifiedBulkEvent updateForSource(const MapUpdateQueryInterface& map);
	BlockModifiedBulkEvent updateForFlowing(const MapUpdateQueryInterface& map);
	std::vector<glm::ivec3> getAvailableDirections(const MapUpdateQueryInterface& map, glm::ivec3 blockCoord);
	unsigned char getCurrentLevel() const { return m_currentLevel; };
	virtual BlockMesherType getMesherType() const override { return BlockMesherType::MESHING_LIQUID; };
	bool isSource() const { return m_isSource; };

	virtual std::unique_ptr<DynamicBlockDataTemplate> clone() override {
		return std::make_unique<LiquidBlockData>(
			m_position, m_lastUpdatedTick, m_nextUpdateScheduledTick, m_isSource, m_currentLevel);
	};
};

class LiquidBlock : public DynamicBlockTemplate<LiquidBlockData>
{
public:
	enum class Materials
	{
		MATERIAL_STILL = 0,
		MATERIAL_FLOWING = 1,
		MATERIAL_NUM = 2
	};

protected:
	static inline BlockRegistrar<LiquidBlock> registrar{ "LiquidBlock" };

	std::array<uint32_t, static_cast<size_t>(Materials::MATERIAL_NUM)> m_atlasIndices;
	unsigned int m_ticksToSpread; //says how many ticks are needed to spread (randomly flactuates in a +- 5 ticks range)
public:
	virtual const Phys::Hitbox* getHitbox() const override { return nullptr; };
	virtual bool loadFromFile(const std::string& filepath) override;

	bool setFaceMaterial(int faceIndex, const std::string& materialName);
	virtual BlockMesherType getMesherType() const override { return BlockMesherType::MESHING_LIQUID; };
	virtual std::string getType() const { return "LiquidBlock"; };

	uint32_t getAtlasIndex(Materials material) const { return m_atlasIndices[static_cast<size_t>(material)]; };
};

