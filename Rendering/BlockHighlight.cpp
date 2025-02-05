#include "BlockHighlight.h"

BlockHighlight::BlockHighlight(unsigned int atlasIndex, float delta /*= 0.002f*/)
	: m_atlasIndex(atlasIndex), m_delta(delta)
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(static_cast<size_t>(Buffers::NUM), m_buffers.data());

	glBindVertexArray(m_VAO);

	//populating position buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<size_t>(Buffers::POSITIONS)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(static_cast<size_t>(Locations::POSITIONS));
	glVertexAttribPointer(static_cast<size_t>(Locations::POSITIONS), 3, GL_FLOAT, GL_FALSE, 0, 0);

	//populating index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[static_cast<size_t>(Buffers::INDEX)]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
};

BlockHighlight::~BlockHighlight()
{
	glDeleteBuffers(static_cast<size_t>(Buffers::NUM), m_buffers.data());
	glDeleteVertexArrays(1, &m_VAO);
}

BlockHighlight::BufferData BlockHighlight::makeBufferData(
	Area selectedArea)
{
	BufferData data;
	
	//front face and back face
	for(int z = 0; z < selectedArea.sizes.z; z++)
	{
		for(int y = 0; y < selectedArea.sizes.y; y++)
		{
			data.transpositions.push_back(glm::vec3(selectedArea.corner.x + selectedArea.sizes.x - 1, selectedArea.corner.y + y, selectedArea.corner.z + z));
			data.atlasIndices.push_back(m_atlasIndex);
			data.faceIndices.push_back(static_cast<unsigned int>(Directions3DHashed::DIRECTION_FORWARD));
		
			data.transpositions.push_back(glm::vec3(selectedArea.corner.x, selectedArea.corner.y + y, selectedArea.corner.z + z));
			data.atlasIndices.push_back(m_atlasIndex);
			data.faceIndices.push_back(static_cast<unsigned int>(Directions3DHashed::DIRECTION_BACKWARD));
		}
	}

	//right face and left face
	for (int x = 0; x < selectedArea.sizes.x; x++)
	{
		for (int y = 0; y < selectedArea.sizes.y; y++)
		{
			data.transpositions.push_back(glm::vec3(selectedArea.corner.x + x, selectedArea.corner.y + y, selectedArea.corner.z + selectedArea.sizes.z - 1));
			data.atlasIndices.push_back(m_atlasIndex);
			data.faceIndices.push_back(static_cast<unsigned int>(Directions3DHashed::DIRECTION_RIGHT));

			data.transpositions.push_back(glm::vec3(selectedArea.corner.x + x, selectedArea.corner.y + y, selectedArea.corner.z));
			data.atlasIndices.push_back(m_atlasIndex);
			data.faceIndices.push_back(static_cast<unsigned int>(Directions3DHashed::DIRECTION_LEFT));
		}
	}

	//top face and bottom face
	for (int x = 0; x < selectedArea.sizes.x; x++)
	{
		for (int z = 0; z < selectedArea.sizes.y; z++)
		{
			data.transpositions.push_back(glm::vec3(selectedArea.corner.x + x, selectedArea.corner.y + selectedArea.sizes.y - 1, selectedArea.corner.z + z));
			data.atlasIndices.push_back(m_atlasIndex);
			data.faceIndices.push_back(static_cast<unsigned int>(Directions3DHashed::DIRECTION_UP));

			data.transpositions.push_back(glm::vec3(selectedArea.corner.x + x, selectedArea.corner.y, selectedArea.corner.z + z));
			data.atlasIndices.push_back(m_atlasIndex);
			data.faceIndices.push_back(static_cast<unsigned int>(Directions3DHashed::DIRECTION_DOWN));
		}
	}

	return data;
}

void BlockHighlight::draw(Shader& shader, 
	Area selectedArea,
	glm::mat4 view, glm::mat4 projection)
{
	size_t instanceCount = selectedArea.sizes.x * selectedArea.sizes.y * 2 + 
	selectedArea.sizes.x * selectedArea.sizes.z * 2 + selectedArea.sizes.z * selectedArea.sizes.y * 2;
	if (instanceCount == 0) {
		// Handle error - no instances to draw
		return;
	}

	shader.Bind();
	glUniformMatrix4fv(shader.GetUniformLocation("u_viewTransform"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(shader.GetUniformLocation("u_projectionTransform"), 1, GL_FALSE, &projection[0][0]);
	glUniform1f(shader.GetUniformLocation("u_delta"), m_delta);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DataRepository::getAtlas().getId());
	glUniform1i(shader.GetUniformLocation("u_Texture"), 0);
	DataRepository::getAtlas().bindStorageBuffer();

	glBindVertexArray(m_VAO);

	BufferData data = makeBufferData(selectedArea);

	//populating atlas index buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(Buffers::ATLAS_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data.atlasIndices[0]) * data.atlasIndices.size(), data.atlasIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Locations::ATLAS_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(Locations::ATLAS_INDEX), 1, GL_UNSIGNED_INT, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Locations::ATLAS_INDEX), 1);

	//populating rotation index buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(Buffers::FACE_INDEX)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data.faceIndices[0]) * data.faceIndices.size(), data.faceIndices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Locations::FACE_INDEX));
	glVertexAttribIPointer(static_cast<unsigned int>(Locations::FACE_INDEX), 1, GL_UNSIGNED_BYTE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Locations::FACE_INDEX), 1);

	//populating transpositions buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(Buffers::TRANSPOSITIONS)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data.transpositions[0]) * data.transpositions.size(), data.transpositions.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(Locations::TRANSPOSITIONS));
	glVertexAttribPointer(static_cast<unsigned int>(Locations::TRANSPOSITIONS), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(static_cast<unsigned int>(Locations::TRANSPOSITIONS), 1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[static_cast<unsigned int>(Buffers::INDEX)]);
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT,
		0, instanceCount, 0);

	glBindVertexArray(0);
}