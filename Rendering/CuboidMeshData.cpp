#include "CuboidMeshData.h"

void CuboidMeshData::addCuboidData(BlockData data)
{
	coordToCuboidMap.emplace(data.blockCoord, data);
	totalCapacity += data.atlasIndices.size();
}

void CuboidMeshData::removeCuboidData(const glm::ivec3& blockCoord)
{
	auto data = coordToCuboidMap.find(blockCoord);
	if (data == coordToCuboidMap.end())
		return;
	totalCapacity -= data->second.atlasIndices.size();
	coordToCuboidMap.erase(data);
}

void CuboidMeshData::buildBuffers()
{
	if (totalCapacity == 0)
		return;
	bufferAtlasIndices.reserve(totalCapacity);
	bufferRotationIndices.reserve(totalCapacity);
	bufferDimensions.reserve(totalCapacity);
	bufferPositions.reserve(totalCapacity);

	for (const auto& it : coordToCuboidMap)
	{
		bufferAtlasIndices.insert(bufferAtlasIndices.end(),
			it.second.atlasIndices.begin(),
			it.second.atlasIndices.end());
		bufferRotationIndices.insert(bufferRotationIndices.end(),
			it.second.rotationIndices.begin(),
			it.second.rotationIndices.end());
		bufferDimensions.insert(bufferDimensions.end(),
			it.second.dimensions.begin(),
			it.second.dimensions.end());
		bufferPositions.insert(bufferPositions.end(),
			it.second.positions.begin(),
			it.second.positions.end());
	}

	if (buffersBuilt == false)
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(static_cast<unsigned int>(CuboidMeshData::Buffers::NUM_BUFFERS), buffers);
	}

	glBindVertexArray(vao);

	//rebinding layout data
	glBindBuffer(GL_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_POSITION));
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION));
	glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION),
		3, GL_FLOAT, GL_FALSE, 0, 0);


	//populating atlas index buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffers[static_cast<unsigned int>(CuboidMeshData::Buffers::BUFFER_ATLAS_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bufferAtlasIndices[0]) * bufferAtlasIndices.size(), bufferAtlasIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX), 1, GL_UNSIGNED_INT, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX), 1);

	//populating rotation index buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffers[static_cast<unsigned int>(CuboidMeshData::Buffers::BUFFER_FACE_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bufferRotationIndices[0]) * bufferRotationIndices.size(), bufferRotationIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX), 1, GL_UNSIGNED_BYTE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX), 1);

	//populating dimensions buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffers[static_cast<unsigned int>(CuboidMeshData::Buffers::BUFFER_DIMENSIONS)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bufferDimensions[0]) * bufferDimensions.size(), bufferDimensions.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS));
	glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS), 1);

	//populating positions buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffers[static_cast<unsigned int>(CuboidMeshData::Buffers::BUFFER_TRANSPOSITION)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bufferPositions[0]) * bufferPositions.size(), bufferPositions.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION));
	glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION), 1);

	glBindVertexArray(0);

	buffersBuilt = true;
	bufferAtlasIndices.clear();
	bufferRotationIndices.clear();
	bufferDimensions.clear();
	bufferPositions.clear();
}

void CuboidMeshData::clearBuffers()
{
	glDeleteBuffers(static_cast<unsigned int>(CuboidMeshData::Buffers::NUM_BUFFERS), buffers);
	glDeleteVertexArrays(1, &vao);
	buffersBuilt = false;
}
