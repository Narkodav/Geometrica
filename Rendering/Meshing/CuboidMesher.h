#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

#include <vector>
#include <queue>
#include <unordered_map>

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/compatibility.hpp>

#include "platformCommon.h"
#include "DataManagement/Cuboid.h"
#include "MesherTemplate.h"
#include "DataManagement/Blocks/CuboidBlock.h"
#include "DataManagement/Cuboid.h"


class CuboidMesher : public MesherTemplate
{
public:
	enum class Buffers
	{
		BUFFER_TRANSPOSITION,
		BUFFER_ATLAS_INDEX,
		BUFFER_FACE_INDEX,
		BUFFER_DIMENSIONS,
		NUM_BUFFERS
	};

	struct BlockData
	{
		//vector sizes must be the same
		std::vector<unsigned int> atlasIndices;
		std::vector<unsigned char> rotationIndices;
		std::vector<glm::vec3> dimensions;
		std::vector<glm::vec3> positions;
		glm::ivec3 blockCoord;
	};

	static inline const BlockMesherType mesherType = BlockMesherType::MESHING_CUBOID;

private:

	std::unordered_map<glm::ivec3, BlockData> m_coordToCuboidMap;
	unsigned int m_totalCapacity = 0;
	unsigned int m_buffers[static_cast<unsigned int>(Buffers::NUM_BUFFERS)];
	unsigned int m_vao;
	bool m_buffersBuilt = false;

	std::vector<unsigned int> m_bufferAtlasIndices;
	std::vector<unsigned char> m_bufferRotationIndices;
	std::vector<glm::vec3> m_bufferDimensions;
	std::vector<glm::vec3> m_bufferPositions;

public:
	unsigned int getTotalCapacity() const { return m_totalCapacity; };
	unsigned int getBuffer(Buffers buffer) const { return m_buffers[static_cast<unsigned int>(buffer)]; };
	unsigned int getVAO() const { return m_vao; };

	virtual ~CuboidMesher() { clearBuffers(); }; //in case someone decides to inherit from this

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
};

