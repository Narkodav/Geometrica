#pragma once
#include <functional>
#include <array>
#include "utilities/array3d.h"
#include "Rendering/MeshData.h"
#include "ChunkManagement/ChunkRegionData.h"

class ChunkMeskTaskTable
{
	enum class MeshFunctionType
	{
		CORE = 0,

		SIDE_RIGHT,
		SIDE_LEFT,
		SIDE_FRONT,
		SIDE_BACK,

		CORNER_RIGHT_FRONT,
		CORNER_RIGHT_BACK,
		CORNER_LEFT_FRONT,
		CORNER_LEFT_BACK,

		TOP_CORE,

		TOP_SIDE_RIGHT,
		TOP_SIDE_LEFT,
		TOP_SIDE_FRONT,
		TOP_SIDE_BACK,

		TOP_CORNER_RIGHT_FRONT,
		TOP_CORNER_RIGHT_BACK,
		TOP_CORNER_LEFT_FRONT,
		TOP_CORNER_LEFT_BACK,

		BOTTOM_CORE,

		BOTTOM_SIDE_RIGHT,
		BOTTOM_SIDE_LEFT,
		BOTTOM_SIDE_FRONT,
		BOTTOM_SIDE_BACK,

		BOTTOM_CORNER_RIGHT_FRONT,
		BOTTOM_CORNER_RIGHT_BACK,
		BOTTOM_CORNER_LEFT_FRONT,
		BOTTOM_CORNER_LEFT_BACK,

		MESH_FUNCTION_NUM
	};



	struct MeshDataTable
	{
		std::vector<MeshData>& uniqueMeshesCache;
		std::map<std::string, unsigned int>& uniqueMeshesMap;
		Chunk::ReadAccess* access[ChunkRegionData::center + 1];
	};

	using TaskTable = Array3d<std::function<void(MeshDataTable&)>,
	constWorldHeight, constChunkSize, constChunkSize>;

	static TaskTable initialMeshTaskTable;

	static TaskTable getInitTaskTable();

	static inline void checkAdj(MeshDataTable& data, FaceRotations rotation,
		Directions2DHashed dir2d, glm::ivec3 coordCurrent, glm::ivec3 coordAdj, unsigned int id);

	static inline void checkCurrent(MeshDataTable& data, Directions3DHashed dir2, 
	glm::ivec3 coord, unsigned int id);


	static void meshCore(MeshDataTable& data, glm::ivec3 coord);

	static void meshSide(MeshDataTable& data, glm::ivec3 coord, Directions2DHashed dir);

	static void meshCorner(MeshDataTable& data, glm::ivec3 coord,
		Directions2DHashed dir1, Directions2DHashed dir2);

	static void meshTopCore(MeshDataTable& data, glm::ivec3 coord);

	static void meshTopSide(MeshDataTable& data, glm::ivec3 coord,
		Directions2DHashed dir);

	static void meshTopCorner(MeshDataTable& data, glm::ivec3 coord,
		Directions2DHashed dir1, Directions2DHashed dir2);

	static void meshBottomCore(MeshDataTable& data, glm::ivec3 coord);

	static void meshBottomSide(MeshDataTable& data, glm::ivec3 coord,
		Directions2DHashed dir);

	static void meshBottomCorner(MeshDataTable& data, glm::ivec3 coord,
		Directions2DHashed dir1, Directions2DHashed dir2);

	template<size_t size>
	static void addCube(MeshDataTable& data,
		const glm::ivec3& blockCoords,
		const std::array<unsigned int, size>& sidesToCheck)
	{
		auto access = *data.access[ChunkRegionData::center];
		auto& item = DataRepository::getItem(access.data.m_blockData(
			blockCoords.y, blockCoords.x, blockCoords.z));

		for (int i : sidesToCheck)
		{
			if ((access.data.m_blockData(blockCoords.y + constDirectionVectors3DHashed[i].y,
				blockCoords.x + constDirectionVectors3DHashed[i].x,
				blockCoords.z + constDirectionVectors3DHashed[i].z)
				& DataRepository::modelTypeMask) != DataRepository::cubeModelIdMask)
				addCubeSide(data, blockCoords, item, static_cast<FaceRotations>(i));
		}
	}

	static void addCubeSide(MeshDataTable& data, 
	const glm::ivec3& meshCoords,
		const Item& item,
		FaceRotations rotation);

	static void addBlock(MeshDataTable& data, const glm::ivec3& coords,
		unsigned int id);

	static void addMesh(MeshDataTable& data,
		const glm::ivec3& meshCoords, 
		const Mesh& mesh, unsigned int id, 
		unsigned int meshNum);

};

