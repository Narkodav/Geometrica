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
#include "Cuboid.h"

class CuboidMeshData
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

private:
	//std::vector<unsigned int> atlasIndices;
	//std::vector<unsigned char> rotationIndices;
	//std::vector<glm::vec3> dimensions;
	//std::vector<glm::vec3> positions;
	//std::vector<glm::ivec3> blockCoords;

	std::unordered_map<glm::ivec3, BlockData> coordToCuboidMap;
	unsigned int totalCapacity = 0;
	unsigned int buffers[static_cast<unsigned int>(Buffers::NUM_BUFFERS)];
	unsigned int vao;
	bool buffersBuilt = false;
public:
	unsigned int getTotalCapacity() const { return totalCapacity; };
	unsigned int getBuffer(Buffers buffer) const { return buffers[static_cast<unsigned int>(buffer)]; };
	unsigned int getVAO() const { return vao; };

	void removeCuboidData(const glm::ivec3& blockCoord);
	void addCuboidData(BlockData data);

	void buildBuffers();

	void clearBuffers();

	std::vector<unsigned int> bufferAtlasIndices;
	std::vector<unsigned char> bufferRotationIndices;
	std::vector<glm::vec3> bufferDimensions;
	std::vector<glm::vec3> bufferPositions;

	//std::unordered_multimap<glm::ivec3, unsigned int> coordToCuboidMap;
	//unsigned int buffers[static_cast<unsigned int>(Buffers::NUM_BUFFERS)];
	//unsigned int vao;

	void clear()
	{
		if (buffersBuilt == true)
			clearBuffers();
		coordToCuboidMap.clear();
		totalCapacity = 0;
	}

	//void addBufferData(unsigned int atlasIndex, unsigned char rotationIndex,
	//	glm::vec3 dims, glm::vec3 pos, glm::ivec3 blockCoord)
	//{
	//	coordToCuboidMap.emplace(blockCoord, blockCoords.size());
	//	atlasIndices.push_back(atlasIndex);
	//	dimensions.push_back(dims);
	//	positions.push_back(pos);
	//	rotationIndices.push_back(rotationIndex);
	//	blockCoords.push_back(blockCoord);
	//}

	//void removeBufferData(glm::ivec3 blockCoord)
	//{
	//	auto range = coordToCuboidMap.equal_range(blockCoord);
	//	for (auto it = range.first; it != range.second; ++it) {
	//		size_t idxToRemove = it->second;
	//		size_t lastIdx = blockCoords.size() - 1;

	//		if (idxToRemove != lastIdx) {
	//			// Swap with last element
	//			atlasIndices[idxToRemove] = atlasIndices[lastIdx];
	//			dimensions[idxToRemove] = dimensions[lastIdx];
	//			positions[idxToRemove] = positions[lastIdx];
	//			rotationIndices[idxToRemove] = rotationIndices[lastIdx];
	//			blockCoords[idxToRemove] = blockCoords[lastIdx];

	//			// Update map for swapped element
	//			auto lastRange = coordToCuboidMap.equal_range(blockCoords[lastIdx]);
	//			for (auto mapIt = lastRange.first; mapIt != lastRange.second; ++mapIt) {
	//				if (mapIt->second == lastIdx) {
	//					mapIt->second = idxToRemove;
	//					break;
	//				}
	//			}
	//		}

	//		// Remove last element
	//		atlasIndices.pop_back();
	//		dimensions.pop_back();
	//		positions.pop_back();
	//		rotationIndices.pop_back();
	//		blockCoords.pop_back();
	//	}
	//	coordToCuboidMap.erase(range.first, range.second);
	//}
};

