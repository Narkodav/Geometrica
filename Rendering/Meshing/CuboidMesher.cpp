#include "CuboidMesher.h"

void CuboidMesher::removeData(const glm::ivec3& blockCoord, Chunk::ReadAccess* access /*= nullptr*/)
{
	auto data = m_coordToCuboidMap.find(blockCoord);
	if (data == m_coordToCuboidMap.end())
		return;
	m_totalCapacity -= data->second.atlasIndices.size();
	m_coordToCuboidMap.erase(data);
	m_needsRebuild = true;
}

void CuboidMesher::addData(const glm::ivec3& blockCoords, const unsigned int& id, Chunk::ReadAccess* access /*= nullptr*/)
{
	if (!access) throw std::runtime_error("Chunk access required");

	const std::unique_ptr<BlockTemplate>& baseBlock = DataRepository::getBlock(id);
	const CuboidBlock* block = static_cast<CuboidBlock*>(baseBlock.get());
	assert(block->getMesherType() == mesherType && "Block type mismatch in CuboidMesher!");

	BlockData blockData;
	glm::ivec2 chunkCoords = access->data.getCoords();

	blockData.atlasIndices.reserve(6);
	blockData.positions.reserve(6);
	blockData.dimensions.reserve(6);
	blockData.rotationIndices.reserve(6);
	blockData.blockCoord = blockCoords;

	const glm::vec3 worldOffset(
		blockCoords.x + constChunkSize * chunkCoords.x,
		blockCoords.y,
		blockCoords.z + constChunkSize * chunkCoords.y
	);
	for (int i = 0; i < 6; i++)
	{
		const glm::ivec3 adjPosition = blockCoords + constDirectionVectors3DHashed[i];
		const auto& adjId = access->data.getBlockId(adjPosition.y, adjPosition.x, adjPosition.z);
		std::vector<FaceDrawData> data;

		if (adjId == DataRepository::airId) {
			// If adjacent block is air, get face data for this block only
			data = block->getCuboid().getFaceDrawData(
				static_cast<FaceRotations>(i),
				block->getMaterialAtlasIndex(i)
			);

			for (int j = 0; j < data.size(); j++)
			{
				blockData.atlasIndices.push_back(data[j].atlasIndex);
				blockData.rotationIndices.push_back(data[j].rotationIndex);
				blockData.dimensions.push_back(data[j].dimensions);
				blockData.positions.push_back(data[j].position + worldOffset);
			}
			continue;
		}

		const auto& adjBlock = DataRepository::getBlock(adjId);

		if (adjBlock->getMesherType() == mesherType) {
			// If adjacent block exists and has same mesher type, get face data considering both blocks
			data = block->getCuboid().getFaceDrawData(
				static_cast<CuboidBlock*>(adjBlock.get())->getCuboid(),
				constDirectionVectors3DHashed[i],
				static_cast<FaceRotations>(i),
				block->getMaterialAtlasIndex(i)
			);
		}
		else {
			// If adjacent block exists but has different mesher type, get face data for this block only
			data = block->getCuboid().getFaceDrawData(
				static_cast<FaceRotations>(i),
				block->getMaterialAtlasIndex(i)
			);
		}

		if (data.empty()) continue;

		for (int j = 0; j < data.size(); j++)
		{
			blockData.atlasIndices.push_back(data[j].atlasIndex);
			blockData.rotationIndices.push_back(data[j].rotationIndex);
			blockData.dimensions.push_back(data[j].dimensions);
			blockData.positions.push_back(data[j].position + worldOffset);
		}
	}

	if (!blockData.positions.empty())
	{
		m_coordToCuboidMap.emplace(blockData.blockCoord, blockData);
		m_totalCapacity += blockData.atlasIndices.size();
		m_needsRebuild = true;
	}
}

void CuboidMesher::buildBuffers()
{
	if (!m_needsRebuild)
		return;

	if (m_totalCapacity == 0)
	{
		clearBuffers();
		return;
	}
		
	m_bufferAtlasIndices.reserve(m_totalCapacity);
	m_bufferRotationIndices.reserve(m_totalCapacity);
	m_bufferDimensions.reserve(m_totalCapacity);
	m_bufferPositions.reserve(m_totalCapacity);

	for (const auto& it : m_coordToCuboidMap)
	{
		m_bufferAtlasIndices.insert(m_bufferAtlasIndices.end(),
			it.second.atlasIndices.begin(),
			it.second.atlasIndices.end());
		m_bufferRotationIndices.insert(m_bufferRotationIndices.end(),
			it.second.rotationIndices.begin(),
			it.second.rotationIndices.end());
		m_bufferDimensions.insert(m_bufferDimensions.end(),
			it.second.dimensions.begin(),
			it.second.dimensions.end());
		m_bufferPositions.insert(m_bufferPositions.end(),
			it.second.positions.begin(),
			it.second.positions.end());
	}

	if (m_buffersBuilt == false)
	{
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(static_cast<unsigned int>(CuboidMesher::Buffers::NUM_BUFFERS), m_buffers);
	}

	glBindVertexArray(m_vao);

	//rebinding layout data
	glBindBuffer(GL_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_POSITION));
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION));
	glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION),
		3, GL_FLOAT, GL_FALSE, 0, 0);


	//populating atlas index buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(CuboidMesher::Buffers::BUFFER_ATLAS_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferAtlasIndices[0]) * m_bufferAtlasIndices.size(), m_bufferAtlasIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX), 1, GL_UNSIGNED_INT, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX), 1);

	//populating rotation index buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(CuboidMesher::Buffers::BUFFER_FACE_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferRotationIndices[0]) * m_bufferRotationIndices.size(), m_bufferRotationIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX), 1, GL_UNSIGNED_BYTE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX), 1);

	//populating dimensions buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(CuboidMesher::Buffers::BUFFER_DIMENSIONS)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferDimensions[0]) * m_bufferDimensions.size(), m_bufferDimensions.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS));
	glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS), 1);

	//populating positions buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(CuboidMesher::Buffers::BUFFER_TRANSPOSITION)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferPositions[0]) * m_bufferPositions.size(), m_bufferPositions.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION));
	glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION), 1);

	glBindVertexArray(0);

	m_buffersBuilt = true;
	m_needsRebuild = false;
	m_bufferAtlasIndices.clear();
	m_bufferRotationIndices.clear();
	m_bufferDimensions.clear();
	m_bufferPositions.clear();
}

void CuboidMesher::clearBuffers()
{
	if (!m_buffersBuilt)
		return;
	glDeleteBuffers(static_cast<unsigned int>(CuboidMesher::Buffers::NUM_BUFFERS), m_buffers);
	glDeleteVertexArrays(1, &m_vao);
	m_coordToCuboidMap.clear();
	m_totalCapacity = 0;
	m_buffersBuilt = false;
	m_needsRebuild = true;
}

void CuboidMesher::draw() 
{
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_INDEX));
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, Cuboid::getNumberOfIndices(), GL_UNSIGNED_INT,
		0, m_totalCapacity, 0);
}