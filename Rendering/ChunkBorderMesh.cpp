#include "ChunkBorderMesh.h"

ChunkBorderMesh::ChunkBorderMesh()
{
	m_isSet = 0;
}

ChunkBorderMesh::~ChunkBorderMesh()
{

}

ChunkBorderMesh::ChunkBorderMesh(const ChunkBorderMesh& other)
{
	m_minChunkHandle = other.m_minChunkHandle;
	m_maxChunkHandle = other.m_maxChunkHandle;
	m_furtherSide = other.m_furtherSide;
	m_closerSide = other.m_closerSide;
	m_isSet = other.m_isSet;
}

ChunkBorderMesh& ChunkBorderMesh::operator=(const ChunkBorderMesh& other)
{
	m_minChunkHandle = other.m_minChunkHandle;
	m_maxChunkHandle = other.m_maxChunkHandle;
	m_furtherSide = other.m_furtherSide;
	m_closerSide = other.m_closerSide;
	m_isSet = other.m_isSet;
	return *this;
}

void ChunkBorderMesh::set(const Chunk& chunkA, const Chunk& chunkB)
{
	std::unique_lock<std::shared_mutex> lock(m_borderMutex);
	if (chunkA.m_chunkCoords.x == chunkB.m_chunkCoords.x)
	{
		if (chunkA.m_chunkCoords.y > chunkB.m_chunkCoords.y)
		{
			m_maxChunkHandle = &chunkA;
			m_minChunkHandle = &chunkB;
		}
		else
		{
			m_maxChunkHandle = &chunkB;
			m_minChunkHandle = &chunkA;
		}
		fillChunkHorizontal();
	}
	else if (chunkA.m_chunkCoords.y == chunkB.m_chunkCoords.y)
	{
		if (chunkA.m_chunkCoords.x > chunkB.m_chunkCoords.x)
		{
			m_maxChunkHandle = &chunkA;
			m_minChunkHandle = &chunkB;
		}
		else
		{
			m_maxChunkHandle = &chunkB;
			m_minChunkHandle = &chunkA;
		}
		fillChunkVertical();
	}

}

void ChunkBorderMesh::fillChunkHorizontal()
{
	m_furtherSide = { &DataRepository::getModel("cube").getMeshByName("back") };
	m_furtherSide.drawData = DataRepository::getModel("cube").getDrawToolKitByName("back");
	m_closerSide = { &DataRepository::getModel("cube").getMeshByName("front") };
	m_closerSide.drawData = DataRepository::getModel("cube").getDrawToolKitByName("front");
	bool maxChunkIsCube;
	bool minChunkIsCube;

	for(int y = 0; y < constWorldHeight; y++)
		for (int x = 0; x < constChunkSize; x++)
		{
			maxChunkIsCube = DataRepository::isCube(m_maxChunkHandle->m_blockId(y, x, 0));
			minChunkIsCube = DataRepository::isCube(m_minChunkHandle->m_blockId(y, x, constChunkSize - 1));

			if (maxChunkIsCube && minChunkIsCube || !maxChunkIsCube && !minChunkIsCube)
				continue;

			if (maxChunkIsCube && !minChunkIsCube)
			{
				m_furtherSide.addMesh(glm::ivec3(x, y, 0),
					glm::vec3(x + m_maxChunkHandle->m_chunkCoords.x * constChunkSize,
						y, m_maxChunkHandle->m_chunkCoords.y * constChunkSize),
					DataRepository::getItem(m_maxChunkHandle->m_blockId(y, x, 0)).meshMaterials[static_cast<unsigned int>(faceRotations::ROTATION_BACK)].atlasIndex);

			}
			else if(!maxChunkIsCube && minChunkIsCube)
			{
				m_closerSide.addMesh(glm::ivec3(x, y, constChunkSize - 1),
					glm::vec3(x + m_minChunkHandle->m_chunkCoords.x * constChunkSize,
						y, constChunkSize - 1 + m_minChunkHandle->m_chunkCoords.y * constChunkSize),
					DataRepository::getItem(m_minChunkHandle->m_blockId(y, x, constChunkSize - 1)).meshMaterials[static_cast<unsigned int>(faceRotations::ROTATION_FRONT)].atlasIndex);
			}
		}
}


void ChunkBorderMesh::fillChunkVertical()
{
	m_furtherSide = { &DataRepository::getModel("cube").getMeshByName("left") };
	m_furtherSide.drawData = DataRepository::getModel("cube").getDrawToolKitByName("left");
	m_closerSide = { &DataRepository::getModel("cube").getMeshByName("right") };
	m_closerSide.drawData = DataRepository::getModel("cube").getDrawToolKitByName("right");
	bool maxChunkIsCube;
	bool minChunkIsCube;

	for (int y = 0; y < constWorldHeight; y++)
		for (int z = 0; z < constChunkSize; z++)
		{
			maxChunkIsCube = DataRepository::isCube(m_maxChunkHandle->m_blockId(y, 0, z));
			minChunkIsCube = DataRepository::isCube(m_minChunkHandle->m_blockId(y, constChunkSize - 1, z));

			if (maxChunkIsCube && minChunkIsCube || !maxChunkIsCube && !minChunkIsCube)
				continue;

			if (maxChunkIsCube && !minChunkIsCube)
			{
				m_furtherSide.addMesh(glm::ivec3(0, y, z),
					glm::vec3(m_maxChunkHandle->m_chunkCoords.x * constChunkSize,
						y, z + m_maxChunkHandle->m_chunkCoords.y * constChunkSize),
					DataRepository::getItem(m_maxChunkHandle->m_blockId(y, 0, z)).meshMaterials[static_cast<unsigned int>(faceRotations::ROTATION_LEFT)].atlasIndex);

			}
			else if (!maxChunkIsCube && minChunkIsCube)
			{
				m_closerSide.addMesh(glm::ivec3(constChunkSize - 1, y, z),
					glm::vec3(constChunkSize - 1 + m_minChunkHandle->m_chunkCoords.x * constChunkSize,
						y, z + m_minChunkHandle->m_chunkCoords.y * constChunkSize),
					DataRepository::getItem(m_minChunkHandle->m_blockId(y, constChunkSize - 1, z)).meshMaterials[static_cast<unsigned int>(faceRotations::ROTATION_RIGHT)].atlasIndex);
			}
		}
}

void ChunkBorderMesh::loadBuffers()
{
	std::unique_lock<std::shared_mutex> lock(m_borderMutex);

	for (MeshData& uniqueMesh : std::initializer_list<std::reference_wrapper<MeshData>>{ m_furtherSide, m_closerSide })
	{
		if (!m_isSet)
		{
			glGenVertexArrays(1, &uniqueMesh.vertexArrayId);
			glGenBuffers((unsigned int)bufferTypesInstanced::BUFFER_NUM, uniqueMesh.buffers);
		}

		glBindVertexArray(uniqueMesh.vertexArrayId);

		//rebinding layout data
		glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.drawData.PositionsBufferId);
		glEnableVertexAttribArray(LOCATION_POSITION);
		glVertexAttribPointer(LOCATION_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.drawData.UVBufferId);
		glEnableVertexAttribArray(LOCATION_UV);
		glVertexAttribPointer(LOCATION_UV, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.drawData.NormalsBufferId);
		glEnableVertexAttribArray(LOCATION_NORMAL);
		glVertexAttribPointer(LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

		//populating position buffer
		glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.buffers[static_cast<unsigned int>(bufferTypesInstanced::BUFFER_TRANSPOSITIONS)]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uniqueMesh.transpositions[0]) * uniqueMesh.transpositions.size(), uniqueMesh.transpositions.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(LOCATION_TRANSPOSITION);
		glVertexAttribPointer(LOCATION_TRANSPOSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(LOCATION_TRANSPOSITION, 1);

		//populating texture index
		glBindBuffer(GL_ARRAY_BUFFER, uniqueMesh.buffers[static_cast<unsigned int>(bufferTypesInstanced::BUFFER_TEXTURE_INDEX)]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uniqueMesh.materialIndices[0]) * uniqueMesh.materialIndices.size(), uniqueMesh.materialIndices.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(LOCATION_INDEX);
		glVertexAttribIPointer(LOCATION_INDEX, 1, GL_UNSIGNED_INT, 0, 0);
		glVertexAttribDivisor(LOCATION_INDEX, 1);
	}

	glBindVertexArray(0);
	m_isSet = 1;


}

void ChunkBorderMesh::deleteBuffers()
{
	std::unique_lock<std::shared_mutex> lock(m_borderMutex);
	if (!m_isSet)
		return;
	m_isSet = 0;
	for (MeshData& uniqueMesh : std::initializer_list<std::reference_wrapper<MeshData>>{ m_furtherSide, m_closerSide })
	{
		glDeleteBuffers((unsigned int)bufferTypesInstanced::BUFFER_NUM, uniqueMesh.buffers);
		glDeleteVertexArrays(1, &uniqueMesh.vertexArrayId);
	}
	m_furtherSide.clear();
	m_closerSide.clear();
	m_minChunkHandle = nullptr;
	m_maxChunkHandle = nullptr;
}

void ChunkBorderMesh::draw()
{
	std::unique_lock<std::shared_mutex> lock(m_borderMutex);
	for (MeshData& uniqueMesh : std::initializer_list<std::reference_wrapper<MeshData>>{ m_furtherSide, m_closerSide })
	{
		glBindVertexArray(uniqueMesh.vertexArrayId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uniqueMesh.drawData.elementBufferId);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, uniqueMesh.drawData.numOfIndices, GL_UNSIGNED_INT,
			(void*)(sizeof(unsigned int) * uniqueMesh.drawData.indexOffset),
			uniqueMesh.numOfInstances, uniqueMesh.drawData.vertexOffset);
	}
}