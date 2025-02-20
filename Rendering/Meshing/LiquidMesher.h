#pragma once
#include <array>

#include "MesherTemplate.h"
#include "DataManagement/Blocks/LiquidBlock.h"
#include "DataManagement/Cuboid.h"

class LiquidMesher : public MesherTemplate
{
public:

	enum class Buffers
	{
		BUFFER_TRANSPOSITION,
		BUFFER_ATLAS_INDEX,
		BUFFER_FACE_INDEX,
		BUFFER_LEVELS,
		NUM_BUFFERS
	};

	enum class Locations {
		LOCATION_POSITION = 0, //face vertex positions
		LOCATION_TRANSPOSITION = 1, //global world position, added to the vertex positions
		LOCATION_ATLAS_INDEX = 2,
		LOCATION_FACE_INDEX = 3,
		LOCATION_LEVELS = 4,
		NUM_LOCATIONS
	};

	struct EdgeLevels // hight of edges in pixels (from 0 to 16)
	{
		unsigned char edgeXposZpos;
		unsigned char edgeXposZneg;
		unsigned char edgeXnegZpos;
		unsigned char edgeXnegZneg;

		//static inline const std::array<std::function<void(Levels&, unsigned char)>, 6> directionLookups = {
		//	[](Levels& levels, unsigned char level) { levels.adjForwardLevel = level; },
		//	[](Levels& levels, unsigned char level) { levels.adjRightLevel = level; },
		//	[](Levels& levels, unsigned char level) { return; },
		//	[](Levels& levels, unsigned char level) { levels.adjBackwardLevel = level; },
		//	[](Levels& levels, unsigned char level) { levels.adjLeftLevel = level; },
		//	[](Levels& levels, unsigned char level) { return; }
		//};

		void setLevel(const std::array<std::array<unsigned char, 3>, 3>& waterLevels) {
			// Use local variables to work with registers instead of memory
			const unsigned char* row0 = waterLevels[0].data();
			const unsigned char* row1 = waterLevels[1].data();
			const unsigned char* row2 = waterLevels[2].data();

			// Cache center column comparisons (most frequently used)
			const unsigned char max11_12 = std::max(row1[1], row1[2]);
			const unsigned char max10_11 = std::max(row1[0], row1[1]);

			// Calculate all edges at once
			edgeXposZpos = std::max(std::max(row2[2], row2[1]), max11_12);
			edgeXposZneg = std::max(std::max(row2[0], row2[1]), max10_11);
			edgeXnegZpos = std::max(std::max(row0[2], row0[1]), max11_12);
			edgeXnegZneg = std::max(std::max(row0[0], row0[1]), max10_11);
		}
	};

	struct LiquidBlockMeshData
	{
		//vector sizes must be the same
		std::vector<unsigned int> atlasIndices;
		std::vector<unsigned char> rotationIndices;
		std::vector<EdgeLevels> levels;
		std::vector<glm::vec3> positions;
		glm::ivec3 blockCoord;
	};

	static inline const BlockMesherType mesherType = BlockMesherType::MESHING_LIQUID;

private:

	std::unordered_map<glm::ivec3, LiquidBlockMeshData> m_coordToBlockMap;
	unsigned int m_totalCapacity = 0;
	unsigned int m_buffers[static_cast<unsigned int>(Buffers::NUM_BUFFERS)];
	unsigned int m_vao;
	bool m_buffersBuilt = false;

	std::vector<unsigned int> m_bufferAtlasIndices;
	std::vector<unsigned char> m_bufferRotationIndices;
	std::vector<EdgeLevels> m_bufferLevels;
	std::vector<glm::vec3> m_bufferPositions;

public:
	unsigned int getTotalCapacity() const { return m_totalCapacity; };
	unsigned int getBuffer(Buffers buffer) const { return m_buffers[static_cast<unsigned int>(buffer)]; };
	unsigned int getVAO() const { return m_vao; };

	virtual ~LiquidMesher() { clearBuffers(); }; //in case someone decides to inherit from this

	// Core meshing operations
	void removeData(const glm::ivec3& blockCoord, Chunk::ReadAccess* access = nullptr) override;
	void addData(const glm::ivec3& blockCoords, const unsigned int& id, Chunk::ReadAccess* access = nullptr) override;
	/*void replaceData(const BlockRemeshEvent& data, Chunk::ReadAccess* access = nullptr);*/

	// Buffer management
	void buildBuffers() override;
	void clearBuffers() override;

	// Rendering
	void draw() override;

	// Type and state queries
	BlockMesherType getType() const override { return mesherType; };
	bool isDirty() const override { return m_needsRebuild; };
	bool isEmpty() const override { return m_totalCapacity == 0; };

private:

	EdgeLevels getEdgeData(const glm::ivec3& blockCoords, Chunk::ReadAccess* access) const;

	//void meshTop(const glm::ivec3& blockCoords, LiquidBlockMeshData& blockData,
	//	const LiquidBlock* block, LiquidBlock::Materials material,
	//	Chunk::ReadAccess* access, Directions3DHashed direction);
	//void meshBottom(const glm::ivec3& blockCoords, LiquidBlockMeshData& blockData,
	//	const LiquidBlock* block, LiquidBlock::Materials material,
	//	Chunk::ReadAccess* access, Directions3DHashed direction);
	//void meshSide(const glm::ivec3& blockCoords, EdgeLevels& levels,
	//	LiquidBlockMeshData& blockData, const LiquidBlock* block, LiquidBlock::Materials material,
	//	Chunk::ReadAccess* access, Directions3DHashed direction);
};

