#pragma once
#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/compatibility.hpp>

#include "Rendering/Model.h"
#include "DataManagement/dataRepository.h"

enum class bufferTypesInstanced : unsigned int
{
	BUFFER_TRANSPOSITIONS = 0,
	BUFFER_TEXTURE_INDEX,
	BUFFER_NUM,
};

struct MeshData
{
	//per mesh data
	const Mesh* data;
	drawToolKit drawData;
	unsigned int numOfInstances = 0;
	unsigned int vertexArrayId;
	unsigned int buffers[static_cast<unsigned int>(bufferTypesInstanced::BUFFER_NUM)];

	//per instance data
	std::unordered_map<glm::vec3, unsigned int> meshLookupTable;

	std::vector<glm::ivec3> meshCoords = {};
	std::vector<glm::vec3> transpositions = {};
	std::vector<unsigned int> materialIndices = {};

	void addMesh(glm::ivec3 meshCoord, glm::vec3 transposition, unsigned int materialIndex)
	{
		numOfInstances++;
		meshLookupTable[meshCoord] = meshCoords.size();
		meshCoords.emplace_back(meshCoord);
		transpositions.emplace_back(transposition);
		materialIndices.emplace_back(materialIndex);
	}

	void removeMesh(glm::ivec3 coord) //needs to be a renderer thread
	{
		if (meshLookupTable.find(coord) == meshLookupTable.end())
			return;
		auto iterator = meshLookupTable.find(coord);

		meshCoords[iterator->second] = meshCoords.back();
		transpositions[iterator->second] = transpositions.back();
		materialIndices[iterator->second] = materialIndices.back();

		meshLookupTable.find(meshCoords[iterator->second])->second = iterator->second;
		meshLookupTable.erase(iterator);

		meshCoords.pop_back();
		transpositions.pop_back();
		materialIndices.pop_back();

		numOfInstances--;

		//populating position buffer
		glBindBuffer(GL_ARRAY_BUFFER, buffers[static_cast<unsigned int>(bufferTypesInstanced::BUFFER_TRANSPOSITIONS)]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(transpositions[0]) * transpositions.size(), transpositions.data(), GL_DYNAMIC_DRAW);

		//populating texture index
		glBindBuffer(GL_ARRAY_BUFFER, buffers[static_cast<unsigned int>(bufferTypesInstanced::BUFFER_TEXTURE_INDEX)]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(materialIndices[0]) * materialIndices.size(), materialIndices.data(), GL_DYNAMIC_DRAW);
	}

	void clear()
	{
		meshLookupTable.clear();
		meshCoords.clear();
		transpositions.clear();
		materialIndices.clear();
	}

	MeshData() {
		//meshCoords.reserve(constChunkCapacity);
		//transpositions.reserve(constChunkCapacity);
		//materialIndices.reserve(constChunkCapacity);
	};

	MeshData(const Mesh* data) {
		data = data;
		//meshCoords.reserve(constChunkCapacity);
		//transpositions.reserve(constChunkCapacity);
		//materialIndices.reserve(constChunkCapacity);
	};
};
