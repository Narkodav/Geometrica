#include "ChunkMeskTaskTable.h"

ChunkMeskTaskTable::TaskTable ChunkMeskTaskTable::initialMeshTaskTable = getInitTaskTable();


ChunkMeskTaskTable::TaskTable ChunkMeskTaskTable::getInitTaskTable()
{
	TaskTable result;

//core
	for (int y = 1; y < constWorldHeight - 1; y++)
		for (int x = 1; x < constWorldHeight - 1; x++)
			for (int z = 1; z < constWorldHeight - 1; z++)
			{
				result(y, x, z) = [y, x, z](MeshDataTable& data) {
					meshCore(data, glm::ivec3(x, y, z));
					};
			}

//side right

	for (int y = 1; y < constWorldHeight - 1; y++)
		for (int x = 1; x < constWorldHeight - 1; x++)
			for (int z = 1; z < constWorldHeight - 1; z++)
			{
				//initialMeshTaskTable(y, x, z) = [y, x, z](MeshDataTable& data) {
				//	unsigned int id = data.access[ChunkRegionData::center]->
				//		data.m_blockData(y, x, z);

				//	if (!id)
				//		return;
				//	if (DataRepository::isCube(id))
				//	{
				//		checkCurrent(data,
				//			dir, glm::ivec3(x, y, z), id)

				//		checkAdj(data, rotation,
				//				dir2d, coordCurrent, coordAdj, id);
				//	};

				//	}
				//	else addBlock(data, glm::ivec3(x, y, z), id);

					
			}

	return result;
}

inline void ChunkMeskTaskTable::checkAdj(MeshDataTable& data, FaceRotations rotation,
	Directions2DHashed dir2d, glm::ivec3 coordCurrent, glm::ivec3 coordAdj, unsigned int id)
{
	unsigned int idAdj = data.access[static_cast<unsigned int>(dir2d)]->
	data.getBlock(coordAdj.y, coordAdj.x, coordAdj.z);

	if (!DataRepository::isCube(idAdj))
		addCubeSide(data, coordCurrent,
			DataRepository::getItem(id),
			rotation);
}

inline void ChunkMeskTaskTable::checkCurrent(MeshDataTable& data, 
	Directions3DHashed dir, glm::ivec3 coord, unsigned int id)
{
	glm::ivec3 coordAdj = coord + constDirectionVectors3DHashed[static_cast<unsigned int>(dir)];
	unsigned int idAdj = data.access[ChunkRegionData::center]->
		data.getBlock(coordAdj.y, coordAdj.x, coordAdj.z);

	if (!DataRepository::isCube(idAdj))
		addCubeSide(data, coord,
			DataRepository::getItem(id),
			static_cast<FaceRotations>(dir));
}

void ChunkMeskTaskTable::meshCore(MeshDataTable& data, glm::ivec3 coord)
{
	unsigned int id = data.access[ChunkRegionData::center]->
	data.getBlock(coord.y, coord.x, coord.z);

	if (!id)
		return;
	if (DataRepository::isCube(id))
		for (int i = 0; i < 6; i++)
			checkCurrent(data, static_cast<Directions3DHashed>(i), coord, id);
	else addBlock(data, coord, id);
}

void ChunkMeskTaskTable::meshSide(MeshDataTable& data, glm::ivec3 coord, Directions2DHashed dir)
{

}

void ChunkMeskTaskTable::meshCorner(MeshDataTable& data, glm::ivec3 coord,
	Directions2DHashed dir1, Directions2DHashed dir2)
{

}

void ChunkMeskTaskTable::meshTopCore(MeshDataTable& data, glm::ivec3 coord)
{

}

void ChunkMeskTaskTable::meshTopSide(MeshDataTable& data, glm::ivec3 coord,
	Directions2DHashed dir)
{

}

void ChunkMeskTaskTable::meshTopCorner(MeshDataTable& data, glm::ivec3 coord,
	Directions2DHashed dir1, Directions2DHashed dir2)
{

}

void ChunkMeskTaskTable::meshBottomCore(MeshDataTable& data, glm::ivec3 coord)
{

}

void ChunkMeskTaskTable::meshBottomSide(MeshDataTable& data, glm::ivec3 coord,
	Directions2DHashed dir)
{

}

void ChunkMeskTaskTable::meshBottomCorner(MeshDataTable& data, glm::ivec3 coord,
	Directions2DHashed dir1, Directions2DHashed dir2)
{

}

void ChunkMeskTaskTable::addCubeSide(MeshDataTable& data,
	const glm::ivec3& meshCoords,
	const Item& item,
	FaceRotations rotation)
{
	const glm::ivec2& chunkCoords = data.access[ChunkRegionData::center]->data.getCoords();
	data.uniqueMeshesCache[static_cast<unsigned int>(rotation)].addMesh(meshCoords,
		glm::vec3(meshCoords.x + chunkCoords.x * constChunkSize,
			meshCoords.y, meshCoords.z + chunkCoords.y * constChunkSize),
		item.meshMaterials.atlasIndices[static_cast<unsigned int>(rotation)]);
}

void ChunkMeskTaskTable::addBlock(MeshDataTable& data,
	const glm::ivec3& coords, unsigned int id)
{
	const Model& model = *DataRepository::getItem(id).modelHandle;
	//m_blockModelsCache[coords.y][coords.x][coords.z] = &model;

	for (int i = 0; i < model.getNumOfMeshes(); i++)
		addMesh(data, coords, model.getMesh(i), id, i);

}

void ChunkMeskTaskTable::addMesh(MeshDataTable& data, const glm::ivec3& meshCoords,
	const Mesh& mesh, unsigned int id, unsigned int meshNum)
{
	const glm::ivec2& chunkCoords = data.access[ChunkRegionData::center]->data.getCoords();
	auto iterator = data.uniqueMeshesMap.find(mesh.name);
	if (iterator == data.uniqueMeshesMap.end())
	{
		data.uniqueMeshesMap[mesh.name] = data.uniqueMeshesCache.size();
		data.uniqueMeshesCache.push_back({ &mesh });
		data.uniqueMeshesCache.back().drawData = DataRepository::getItem(id).modelHandle->getDrawToolKitByName(mesh.name);
		data.uniqueMeshesCache.back().addMesh(meshCoords,
			glm::vec3(meshCoords.x + chunkCoords.x * constChunkSize,
				meshCoords.y, meshCoords.z + chunkCoords.y * constChunkSize),
			DataRepository::getItem(id).meshMaterials.atlasIndices[meshNum]);
		return;
	}

	data.uniqueMeshesCache[iterator->second].addMesh(meshCoords,
		glm::vec3(meshCoords.x + chunkCoords.x * constChunkSize,
			meshCoords.y, meshCoords.z + chunkCoords.y * constChunkSize),
		DataRepository::getItem(id).meshMaterials.atlasIndices[meshNum]);
}
