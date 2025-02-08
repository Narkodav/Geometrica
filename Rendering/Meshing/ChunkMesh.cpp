#include "ChunkMesh.h"


ChunkMesh::ChunkMesh()
{
	m_isSet = 0;

	m_meshers[static_cast<size_t>(BlockMesherType::MESHING_CUBOID)] = std::make_unique<CuboidMesher>();

	
    //setCacheSize();
}

ChunkMesh::~ChunkMesh()
{
	deleteBuffers();
}

void ChunkMesh::release()
{
	m_isSet = 0;
}

void ChunkMesh::set(ChunkRegionData& regionData)
{
	m_regionData = std::move(regionData);
	auto access = m_regionData.getCenter()->getReadAccess();

	unsigned int id;
	m_chunkCoords = access.data.getCoords();

	for (int y = 0; y < constWorldHeight; y++)
		for (int x = 0; x < constChunkSize; x++)
			for (int z = 0; z < constChunkSize; z++)
			{
				id = access.data.getBlock(y, x, z);
				if (!id)
					continue;
				const std::unique_ptr<BlockTemplate>& block = DataRepository::getBlock(id);
				m_meshers[static_cast<size_t>(block->getMesherType())]->addData(
					glm::ivec3(x, y, z), id, &access);
			}

}

void ChunkMesh::applyUpdates()
{
	if (m_blockModificationQueue.empty())
		return;
	auto access = m_regionData.getCenter()->getReadAccess();
	while (!m_blockModificationQueue.empty())
	{
		replaceBlock(m_blockModificationQueue.front(), access);
		m_blockModificationQueue.pop();
	}
}

//void ChunkMesh::addBlock(const glm::ivec3& coords, const unsigned int& id)
//{
//	const Model& model = *DataRepository::getBlock(id).displayModelHandle;
//	//m_blockModelsCache[coords.y][coords.x][coords.z] = &model;
//
//	for (int i = 0; i < model.getNumOfMeshes(); i++)
//		addMesh(coords, model.getMesh(i), id, i);
//
//}
//
//void ChunkMesh::addCuboid(const glm::ivec3& blockCoords, const unsigned int& id, Chunk::ReadAccess& access) //Cube and cubeFace meshes are essential, changing their files breaks the game
//{
//	auto& item = DataRepository::getBlock(id);
//	CuboidMeshData::BlockData blockData;
//
//	blockData.atlasIndices.reserve(6);
//	blockData.positions.reserve(6);
//	blockData.dimensions.reserve(6);
//	blockData.rotationIndices.reserve(6);
//	blockData.blockCoord = blockCoords;
//
//	const glm::vec3 worldOffset(
//		blockCoords.x + constChunkSize * m_chunkCoords.x,
//		blockCoords.y,
//		blockCoords.z + constChunkSize * m_chunkCoords.y
//	);
//	for (int i = 0; i < 6; i++)
//	{
//		const glm::vec3 adjPosition = blockCoords + constDirectionVectors3DHashed[i];
//		const auto& adjId = access.data.getBlock(adjPosition.y, adjPosition.x, adjPosition.z);
//
//		const std::vector<FaceDrawData>& data = DataRepository::hasCuboidModel(adjId)
//			? item.cuboidModelHandle->getFaceDrawData(
//				*DataRepository::getBlock(adjId).cuboidModelHandle,
//				constDirectionVectors3DHashed[i],
//				static_cast<FaceRotations>(i),
//				item.cuboidMeshMaterials.atlasIndices[i])
//			: item.cuboidModelHandle->getFaceDrawData(
//				static_cast<FaceRotations>(i),
//				item.cuboidMeshMaterials.atlasIndices[i]);
//
//		if (data.empty()) continue;
//
//		for (int j = 0; j < data.size(); j++)
//		{
//			blockData.atlasIndices.push_back(data[j].atlasIndex);
//			blockData.rotationIndices.push_back(data[j].rotationIndex);
//			blockData.dimensions.push_back(data[j].dimensions);
//			blockData.positions.push_back(data[j].position + worldOffset);
//		}
//	}
//	if (!blockData.positions.empty())
//		m_cuboidMeshData.addCuboidData(std::move(blockData));
//}

void ChunkMesh::replaceBlock(const BlockRemeshEvent& data, Chunk::ReadAccess& access)
{
	if (data.currentId == data.previousId)
	{
		if (data.currentId != DataRepository::airId) {
			const std::unique_ptr<BlockTemplate>& block = DataRepository::getBlock(data.currentId);
			auto mesherIndex = static_cast<size_t>(block->getMesherType());
			m_meshers[mesherIndex]->removeData(data.blockCoord, &access);
			m_meshers[mesherIndex]->addData(data.blockCoord, data.currentId, &access);
		}
		else return;
	}
	else if (data.previousId == DataRepository::airId)
	{
		const std::unique_ptr<BlockTemplate>& block = DataRepository::getBlock(data.currentId);
		m_meshers[static_cast<size_t>(block->getMesherType())]->addData(
			data.blockCoord, data.currentId, &access);
	}
	else if (data.currentId == DataRepository::airId)
	{
		const std::unique_ptr<BlockTemplate>& block = DataRepository::getBlock(data.previousId);
		m_meshers[static_cast<size_t>(block->getMesherType())]->removeData(
			data.blockCoord, &access);
	}
}

//void ChunkMesh::removeMesh(const glm::ivec3 & coord, std::string meshName)
//{
//	m_uniqueMeshesCache[m_uniqueMeshesMap.find(meshName)->second].removeMesh(coord);
//}
//
//void ChunkMesh::addMesh(const glm::ivec3& meshCoords,
//	const Mesh& mesh, unsigned int id, unsigned int meshNum)
//{
//	auto iterator = m_uniqueMeshesMap.find(mesh.name);
//	if (iterator == m_uniqueMeshesMap.end())
//	{
//		m_uniqueMeshesMap[mesh.name] = m_uniqueMeshesCache.size();
//		m_uniqueMeshesCache.push_back({ &mesh });
//		m_uniqueMeshesCache.back().drawData = DataRepository::getBlock(id).displayModelHandle->getDrawToolKitByName(mesh.name);
//		m_uniqueMeshesCache.back().addMesh(meshCoords,
//			glm::vec3(meshCoords.x + m_chunkCoords.x * constChunkSize,
//				meshCoords.y, meshCoords.z + m_chunkCoords.y * constChunkSize),
//			DataRepository::getBlock(id).modelMeshMaterials.atlasIndices[meshNum]);
//		return;
//	}
//
//	m_uniqueMeshesCache[iterator->second].addMesh(meshCoords,
//		glm::vec3(meshCoords.x + m_chunkCoords.x * constChunkSize,
//			meshCoords.y, meshCoords.z + m_chunkCoords.y * constChunkSize),
//		DataRepository::getBlock(id).modelMeshMaterials.atlasIndices[meshNum]);
//}

void ChunkMesh::loadBuffers(unsigned int offset /*= 0*/)
{
	applyUpdates();
	//for (MeshData& uniqueMesh : m_uniqueMeshesCache)
	//{
	//	if (!m_isSet)
	//	{
	//		glGenVertexArrays(1, &uniqueMesh.vertexArrayId);
	//		glGenBuffers((unsigned int)bufferTypesInstanced::BUFFER_NUM, uniqueMesh.buffers);
	//	}
	//	
	//	glBindVertexArray(uniqueMesh.vertexArrayId);

	//	//rebinding layout data
	//	glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.drawData.PositionsBufferId);
	//	glEnableVertexAttribArray(LOCATION_POSITION);
	//	glVertexAttribPointer(LOCATION_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//	glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.drawData.UVBufferId);
	//	glEnableVertexAttribArray(LOCATION_UV);
	//	glVertexAttribPointer(LOCATION_UV, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//	glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.drawData.NormalsBufferId);
	//	glEnableVertexAttribArray(LOCATION_NORMAL);
	//	glVertexAttribPointer(LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//	//populating position buffer
	//	glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.buffers[static_cast<unsigned int>(bufferTypesInstanced::BUFFER_TRANSPOSITIONS)]);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(uniqueMesh.transpositions[0]) * uniqueMesh.transpositions.size(), uniqueMesh.transpositions.data(), GL_DYNAMIC_DRAW);
	//	glEnableVertexAttribArray(LOCATION_TRANSPOSITION);
	//	glVertexAttribPointer(LOCATION_TRANSPOSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//	glVertexAttribDivisor(LOCATION_TRANSPOSITION, 1);

	//	//populating texture index
	//	glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.buffers[static_cast<unsigned int>(bufferTypesInstanced::BUFFER_TEXTURE_INDEX)]);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(uniqueMesh.materialIndices[0]) * uniqueMesh.materialIndices.size(), uniqueMesh.materialIndices.data(), GL_DYNAMIC_DRAW);
	//	glEnableVertexAttribArray(LOCATION_INDEX);
	//	glVertexAttribIPointer(LOCATION_INDEX, 1, GL_UNSIGNED_INT, 0, 0);
	//	glVertexAttribDivisor(LOCATION_INDEX, 1);
	//}

	//cuboid data
	/*m_cuboidMeshData.buildBuffers();*/

	for (const auto& mesher : m_meshers) {
		mesher->buildBuffers();
	}

	glBindVertexArray(0);

	m_isSet = 1;
}

void ChunkMesh::deleteBuffers()
{
	if (!m_isSet)
		return;
	m_isSet = 0;
	for (const auto& mesher : m_meshers) {
		mesher->clearBuffers();
	}

	//for (MeshData& uniqueMesh : m_uniqueMeshesCache)
	//{
	//	glDeleteBuffers((unsigned int)bufferTypesInstanced::BUFFER_NUM, uniqueMesh.buffers);
	//	glDeleteVertexArrays(1, &uniqueMesh.vertexArrayId);
	//}

	//m_cuboidMeshData.clear();
	//m_uniqueMeshesMap.clear();
	//m_uniqueMeshesCache.clear();
	//m_regionData.reset();
}

void ChunkMesh::draw(BlockMesherType mesherType) const
{
	m_meshers[static_cast<size_t>(mesherType)]->draw();
}

//void ChunkMesh::drawComplicated() const
//{
//	for (const MeshData& uniqueMesh : m_uniqueMeshesCache)
//	{
//		glBindVertexArray(uniqueMesh.vertexArrayId);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uniqueMesh.drawData.elementBufferId);
//		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, uniqueMesh.drawData.numOfIndices, GL_UNSIGNED_INT,
//			(void*)(sizeof(unsigned int) * uniqueMesh.drawData.indexOffset),
//			uniqueMesh.numOfInstances, uniqueMesh.drawData.vertexOffset);
//	}
//}
//
//void ChunkMesh::drawCuboids() const
//{
//	glBindVertexArray(m_cuboidMeshData.getVAO());
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_INDEX));
//	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, Cuboid::getNumberOfIndices(), GL_UNSIGNED_INT,
//		0, m_cuboidMeshData.getTotalCapacity(), 0);
//}