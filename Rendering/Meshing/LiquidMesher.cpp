#include "LiquidMesher.h"

void LiquidMesher::removeData(const glm::ivec3& blockCoord, Chunk::ReadAccess* access /*= nullptr*/)
{
	auto data = m_coordToBlockMap.find(blockCoord);
	if (data == m_coordToBlockMap.end())
		return;
	m_totalCapacity -= data->second.atlasIndices.size();
	m_coordToBlockMap.erase(data);
	m_needsRebuild = true;
}

void LiquidMesher::addData(const glm::ivec3& blockCoords, const unsigned int& id, Chunk::ReadAccess* access /*= nullptr*/)
{
	if (!access) throw std::runtime_error("Chunk access required");

	const std::unique_ptr<BlockTemplate>& baseBlock = DataRepository::getBlock(id);
	const LiquidBlock* block = static_cast<LiquidBlock*>(baseBlock.get());
	assert(block->getMesherType() == mesherType && "Block type mismatch in LiquidMesher!");
	const auto& data = std::move(access->data.getBlock(blockCoords.y, blockCoords.x, blockCoords.z));
	assert(data.dynamicData->getMesherType()
		== mesherType && "Block data type mismatch in LiquidMesher!");

	LiquidBlock::Materials material;
	if (static_cast<LiquidBlockData*>(data.dynamicData.get())->isSource())
		material = LiquidBlock::Materials::MATERIAL_STILL;
	else material = LiquidBlock::Materials::MATERIAL_FLOWING;

	LiquidBlockMeshData blockData;
	glm::ivec2 chunkCoords = access->data.getCoords();

	blockData.atlasIndices.reserve(6);
	blockData.positions.reserve(6);
	blockData.rotationIndices.reserve(6);
	blockData.levels.reserve(6);
	blockData.blockCoord = blockCoords;

	const glm::vec3 worldOffset(
		blockCoords.x + constChunkSize * chunkCoords.x,
		blockCoords.y,
		blockCoords.z + constChunkSize * chunkCoords.y
	);

	for (int i = 0; i < 6; i++)
	{
		glm::ivec3 adjCoords = blockCoords + constDirectionVectors3DHashed[i];
		auto adj = access->data.getBlock(adjCoords.y, adjCoords.x, adjCoords.z);

		if (adj.blockId == DataRepository::airId)
		{
			blockData.atlasIndices.push_back(block->getAtlasIndex(material));
			blockData.positions.push_back(worldOffset);
			blockData.rotationIndices.push_back(i);
		}
		//implement cuboid interaction later
		//else if (DataRepository::getBlock(upId)->getMesherType() == mesherType)
		//{

		//}
	}

	if (!blockData.positions.empty())
	{
		blockData.levels.resize(blockData.atlasIndices.size(), getEdgeData(blockCoords, access));
		m_coordToBlockMap.emplace(blockData.blockCoord, blockData);
		m_totalCapacity += blockData.atlasIndices.size();
		m_needsRebuild = true;
	}
}

void LiquidMesher::buildBuffers()
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
	m_bufferLevels.reserve(m_totalCapacity);
	m_bufferPositions.reserve(m_totalCapacity);

	for (const auto& it : m_coordToBlockMap)
	{
		m_bufferAtlasIndices.insert(m_bufferAtlasIndices.end(),
			it.second.atlasIndices.begin(),
			it.second.atlasIndices.end());
		m_bufferRotationIndices.insert(m_bufferRotationIndices.end(),
			it.second.rotationIndices.begin(),
			it.second.rotationIndices.end());
		m_bufferLevels.insert(m_bufferLevels.end(),
			it.second.levels.begin(),
			it.second.levels.end());
		m_bufferPositions.insert(m_bufferPositions.end(),
			it.second.positions.begin(),
			it.second.positions.end());
	}

	if (m_buffersBuilt == false)
	{
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(static_cast<unsigned int>(LiquidMesher::Buffers::NUM_BUFFERS), m_buffers);
	}

	glBindVertexArray(m_vao);

	//rebinding layout data
	glBindBuffer(GL_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_POSITION));
	glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION));
	glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION),
		3, GL_FLOAT, GL_FALSE, 0, 0);


	//populating atlas index buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(LiquidMesher::Buffers::BUFFER_ATLAS_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferAtlasIndices[0]) * m_bufferAtlasIndices.size(), m_bufferAtlasIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_ATLAS_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_ATLAS_INDEX), 1, GL_UNSIGNED_INT, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_ATLAS_INDEX), 1);

	//populating rotation index buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(LiquidMesher::Buffers::BUFFER_FACE_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferRotationIndices[0]) * m_bufferRotationIndices.size(), m_bufferRotationIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_FACE_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_FACE_INDEX), 1, GL_UNSIGNED_BYTE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_FACE_INDEX), 1);

	//populating levels buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(LiquidMesher::Buffers::BUFFER_LEVELS)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferLevels[0]) * m_bufferLevels.size(), m_bufferLevels.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_LEVELS));
	glVertexAttribIPointer(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_LEVELS), 4, GL_UNSIGNED_BYTE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_LEVELS), 1);

	//populating positions buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(LiquidMesher::Buffers::BUFFER_TRANSPOSITION)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_bufferPositions[0]) * m_bufferPositions.size(), m_bufferPositions.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_TRANSPOSITION));
	glVertexAttribPointer(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_TRANSPOSITION), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(LiquidMesher::Locations::LOCATION_TRANSPOSITION), 1);

	glBindVertexArray(0);

	m_buffersBuilt = true;
	m_needsRebuild = false;
	m_bufferAtlasIndices.clear();
	m_bufferRotationIndices.clear();
	m_bufferLevels.clear();
	m_bufferPositions.clear();
}

void LiquidMesher::clearBuffers()
{
	if (!m_buffersBuilt)
		return;
	glDeleteBuffers(static_cast<unsigned int>(LiquidMesher::Buffers::NUM_BUFFERS), m_buffers);
	glDeleteVertexArrays(1, &m_vao);
	m_coordToBlockMap.clear();
	m_totalCapacity = 0;
	m_buffersBuilt = false;
	m_needsRebuild = true;
}

void LiquidMesher::draw()
{
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_INDEX));
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, Cuboid::getNumberOfIndices(), GL_UNSIGNED_INT,
		0, m_totalCapacity, 0);
}

LiquidMesher::EdgeLevels LiquidMesher::getEdgeData(
	const glm::ivec3& blockCoords, Chunk::ReadAccess* access) const
{
	std::array<std::array<unsigned char, 3>, 3> region;
	for(int x = -1; x < 2; x++)
		for (int z = -1; z < 2; z++)
		{
			const auto& block = access->data.getBlock(blockCoords.y, blockCoords.x + x, blockCoords.z + z);
			if (block.blockId == DataRepository::airId)
			{
				region[x + 1][z + 1] = 0;
				continue;
			}
			const std::unique_ptr<BlockTemplate>& baseBlock = DataRepository::getBlock(block.blockId);

			if (baseBlock->getMesherType() == mesherType)
			{
				const LiquidBlockData* liquidData = static_cast<const LiquidBlockData*>(block.dynamicData.get());
				region[x + 1][z + 1] = liquidData->getCurrentLevel();
			}
			else region[x + 1][z + 1] = 0;
		}

	EdgeLevels levels;
	levels.setLevel(region);
	return levels;
}


//void LiquidMesher::meshTop(const glm::ivec3& blockCoords, LiquidBlockMeshData& blockData,
//	const LiquidBlock* block, LiquidBlock::Materials material,
//	Chunk::ReadAccess* access, Directions3DHashed direction)
//{
//	//up side:
//	glm::ivec3 adjCoords = blockCoords + constDirectionVectors3DHashed[static_cast<size_t>(direction)];
//	auto adjId = access->data.getBlockId(adjCoords.y, adjCoords.x, adjCoords.z);
//
//	if (adjId == DataRepository::airId)
//	{
//		blockData.atlasIndices.push_back(block->getAtlasIndex(material));
//		blockData.positions.push_back(blockCoords);
//		blockData.rotationIndices.push_back(static_cast<size_t>(direction));
//	}
//	//implement cuboid interaction later
//	//else if (DataRepository::getBlock(upId)->getMesherType() == mesherType)
//	//{
//
//	//}
//}
//
//void LiquidMesher::meshBottom(const glm::ivec3& blockCoords, LiquidBlockMeshData& blockData,
//	const LiquidBlock* block, LiquidBlock::Materials material,
//	Chunk::ReadAccess* access, Directions3DHashed direction)
//{
//	//down side:
//	glm::ivec3 adjCoords = blockCoords + constDirectionVectors3DHashed[static_cast<size_t>(direction)];
//	auto adjId = access->data.getBlockId(adjCoords.y, adjCoords.x, adjCoords.z);
//
//	if (adjId == DataRepository::airId)
//	{
//		blockData.atlasIndices.push_back(block->getAtlasIndex(material));
//		blockData.positions.push_back(blockCoords);
//		blockData.rotationIndices.push_back(static_cast<size_t>(direction));
//	}
//	//implement cuboid interaction later
//	//else if (DataRepository::getBlock(upId)->getMesherType() == mesherType)
//	//{
//
//	//}
//}
//
//void LiquidMesher::meshSide(const glm::ivec3& blockCoords,
//	LiquidBlockMeshData& blockData, const LiquidBlock* block, LiquidBlock::Materials material,
//	Chunk::ReadAccess* access, Directions3DHashed direction)
//{
//	//left side:
//	glm::ivec3 adjCoords = blockCoords + constDirectionVectors3DHashed[static_cast<size_t>(direction)];
//	auto adj = access->data.getBlock(adjCoords.y, adjCoords.x, adjCoords.z);
//
//	if (adj.blockId == DataRepository::airId)
//	{
//		blockData.atlasIndices.push_back(block->getAtlasIndex(material));
//		blockData.positions.push_back(blockCoords);
//		blockData.rotationIndices.push_back(static_cast<size_t>(direction));
//	}
//	//implement cuboid interaction later
//	//else if (DataRepository::getBlock(upId)->getMesherType() == mesherType)
//	//{
//
//	//}
//}