#include "Cuboid.h"

Cuboid::Cuboid()
{
	instanceCount++;
	if (instanceCount == 1)
		makeBuffers();
}

//Cuboid::Cuboid(std::vector<CuboidData> data) : m_cuboidData(data)
//{
//	instanceCount++;
//	if (instanceCount == 1)
//		makeBuffers();
//}

Cuboid::~Cuboid()
{
	instanceCount--;
	if (instanceCount == 0)
		deleteBuffers();
}

void Cuboid::makeBuffers()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(static_cast<size_t>(Buffers::NUM_BUFFERS), buffers.data());

	glBindVertexArray(vao);

	//populating position buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffers[static_cast<size_t>(Buffers::BUFFER_POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(static_cast<size_t>(Locations::LOCATION_POSITION));
	glVertexAttribPointer(static_cast<size_t>(Locations::LOCATION_POSITION), 3, GL_FLOAT, GL_FALSE, 0, 0);

	//populating index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[static_cast<size_t>(Buffers::BUFFER_INDEX)]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Cuboid::deleteBuffers()
{
	glDeleteBuffers(static_cast<size_t>(Buffers::NUM_BUFFERS), buffers.data());
	glDeleteVertexArrays(1, &vao);
}

bool CuboidData::cullFace(const Cuboid& other, const FaceRotations& face,
    const glm::vec3& otherPosition) const
{
    const std::vector<CuboidData>& cuboidDataOther = other.getCuboidData();

    for (auto& data : cuboidDataOther)
    {
        // First check if the cuboids are adjacent on the face's axis
        switch (face)
        {
        case FaceRotations::ROTATION_FRONT: // +X face
            if ((otherPosition.x + data.position.x) - (this->position.x) != (dimensions.x + data.dimensions.x) / 2) return false;
            break;
        case FaceRotations::ROTATION_BACK: // -X face
            if ((otherPosition.x + data.position.x) - (this->position.x) != -(dimensions.x + data.dimensions.x) / 2) return false;
            break;
        case FaceRotations::ROTATION_RIGHT: // +Z face
            if ((otherPosition.z + data.position.z) - (this->position.z) != (dimensions.z + data.dimensions.z) / 2) return false;
            break;
        case FaceRotations::ROTATION_LEFT: // -Z face
            if ((otherPosition.z + data.position.z) - (this->position.z) != -(dimensions.z + data.dimensions.z) / 2) return false;
            break;
        case FaceRotations::ROTATION_TOP: // +Y face
            if ((otherPosition.y + data.position.y) - (this->position.y) != (dimensions.y + data.dimensions.y) / 2) return false;
            break;
        case FaceRotations::ROTATION_BOTTOM: // -Y face
            if ((otherPosition.y + data.position.y) - (this->position.y) != -(dimensions.y + data.dimensions.y) / 2) return false;
            break;
        }

        // Then check if the face is completely covered by projecting onto the plane
        switch (face)
        {
        case FaceRotations::ROTATION_FRONT:
        case FaceRotations::ROTATION_BACK:
            // Check if other cuboid covers this face in Y and Z dimensions
            return abs(otherPosition.y + data.position.y - this->position.y) <= (data.dimensions.y - dimensions.y) / 2 &&
                abs(otherPosition.z + data.position.z - this->position.z) <= (data.dimensions.z - dimensions.z) / 2;

        case FaceRotations::ROTATION_RIGHT:
        case FaceRotations::ROTATION_LEFT:
            // Check if other cuboid covers this face in X and Y dimensions
            return abs(otherPosition.x + data.position.x - this->position.x) <= (data.dimensions.x - dimensions.x) / 2 &&
                abs(otherPosition.y + data.position.y - this->position.y) <= (data.dimensions.y - dimensions.y) / 2;

        case FaceRotations::ROTATION_TOP:
        case FaceRotations::ROTATION_BOTTOM:
            // Check if other cuboid covers this face in X and Z dimensions
            return abs(otherPosition.x + data.position.x - this->position.x) <= (data.dimensions.x - dimensions.x) / 2 &&
                abs(otherPosition.z + data.position.z - this->position.z) <= (data.dimensions.z - dimensions.z) / 2;
        }
    }
    return false;
}

std::vector<FaceDrawData> Cuboid::getFaceDrawData(const Cuboid& adjacentCuboid,
    const glm::vec3& adjacentPosition, 
    FaceRotations rotation, unsigned int atlasIndex) const
{
    FaceCombinationsKey key(m_dataId, adjacentCuboid.m_dataId, rotation);
    {
        auto access = faceCombinationsCache.getReadAccess();
        auto data = access->find(key);
        if (data != access->end())
            return data->second;
    }
    //if (this->m_name == "slabHorizontalUp" /*|| adjacentCuboid.m_name == "slabHorizontalUp"*/)
    //    __debugbreak();

    std::vector<FaceDrawData> faceData;
	for (int i = 0; i < m_cuboidData.size(); i++)
	{
        if (!m_cuboidData[i].cullFace(adjacentCuboid, rotation, adjacentPosition))
            faceData.push_back({ atlasIndex, static_cast<unsigned char>(rotation),
            m_cuboidData[i].dimensions, m_cuboidData[i].position });
	}
    
    {
        auto writeAccess = faceCombinationsCache.getWriteAccess();
        // Use try_emplace to handle potential race condition where another thread
        // might have inserted the same key while we were computing
        auto [it, inserted] = writeAccess->try_emplace(key, faceData);
        return inserted ? faceData : it->second;
    }
}

std::vector<FaceDrawData> Cuboid::getFaceDrawData(FaceRotations rotation, unsigned int atlasIndex) const
{
    FaceCombinationsKey key(m_dataId, -1, rotation); //adj is air
    {
        auto access = faceCombinationsCache.getReadAccess();
        auto data = access->find(key);
        if (data != access->end())
            return data->second;
    }

    std::vector<FaceDrawData> faceData;

    for (const auto& data : m_cuboidData) {
        faceData.push_back({
            atlasIndex,
            static_cast<unsigned char>(rotation),
            data.dimensions,
            data.position
            });
    }

    {
        auto writeAccess = faceCombinationsCache.getWriteAccess();
        // Use try_emplace to handle potential race condition where another thread
        // might have inserted the same key while we were computing
        auto [it, inserted] = writeAccess->try_emplace(key, faceData);
        return inserted ? faceData : it->second;
    }
}

std::string getName(std::string filepath)
{
    int dot = filepath.size() - 1;
    int slash;

    for (slash = filepath.size() - 1; slash > -1; slash--)
        if (filepath[slash] == '.')
        {
            dot = slash;
            slash--;
            break;
        }

    for (slash; slash > -1; slash--)
        if (filepath[slash] == '/')
            break;

    filepath = filepath.substr(slash + 1, dot - slash - 1);
    return filepath;
}

void Cuboid::load(const std::string& filepath)
{
    std::ifstream file(filepath);
    m_name = getName(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open cuboid file: " << filepath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);

        // Read dimensions (x:y:z)
        float dimX, dimY, dimZ;
        char delimiter; // for the ':' characters

        iss >> dimX >> delimiter >> dimY >> delimiter >> dimZ;

        // Read position offset (x:y:z)
        float posX, posY, posZ;
        iss >> posX >> delimiter >> posY >> delimiter >> posZ;

        if (iss.fail()) {
            std::cerr << "Error parsing line in cuboid file: " << line << std::endl;
            continue;
        }
        m_cuboidData.push_back({ glm::vec3(dimX, dimY, dimZ), glm::vec3(posX, posY, posZ) });
    }
    m_dataId = loadedCount;
    loadedCount++;
}

void Cuboid::drawTest(std::vector<glm::vec3> positions, std::vector<unsigned int> atlasIndices) const
{
    std::vector<FaceDrawData> faceData;
    std::vector<unsigned char> rotationIndices;
    std::vector<glm::vec3> dimensions;

    for (int i = 0; i < positions.size(); i++)
    {
        for (int rotation = 0; rotation < 6; rotation++)
        {
            std::vector<FaceDrawData> buff = getFaceDrawData(static_cast<FaceRotations>(rotation), atlasIndices[i * 6 + rotation]);
            for(int j = 0; j < buff.size(); j++)
                buff[j].position += positions[i];

            faceData.insert(faceData.end(), buff.begin(), buff.end());
        }
    }

    atlasIndices.clear();
    positions.clear();

    for (int i = 0; i < faceData.size(); i++)
    {
        atlasIndices.push_back(faceData[i].atlasIndex);
        rotationIndices.push_back(faceData[i].rotationIndex);
        dimensions.push_back(faceData[i].dimensions);
        positions.push_back(faceData[i].position);
    }

    unsigned int vao;
    unsigned int bufferAtlas, bufferRotation, bufferDimensions, bufferPosition;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &bufferAtlas);
    glGenBuffers(1, &bufferRotation);
    glGenBuffers(1, &bufferDimensions);
    glGenBuffers(1, &bufferPosition);

    glBindVertexArray(vao);

    //rebinding layout data
    glBindBuffer(GL_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_POSITION));
    glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION));
    glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_POSITION),
        3, GL_FLOAT, GL_FALSE, 0, 0);

    //populating atlas index buffer
    glBindBuffer(GL_ARRAY_BUFFER, bufferAtlas);
    glBufferData(GL_ARRAY_BUFFER, sizeof(atlasIndices[0]) * atlasIndices.size(), atlasIndices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX));
    glVertexAttribIPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX), 1, GL_UNSIGNED_INT, 0, 0);
    glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_ATLAS_INDEX), 1);

    //populating rotation index buffer
    glBindBuffer(GL_ARRAY_BUFFER, bufferRotation);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rotationIndices[0]) * rotationIndices.size(), rotationIndices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX));
    glVertexAttribIPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX), 1, GL_UNSIGNED_BYTE, 0, 0);
    glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_FACE_INDEX), 1);

    //populating dimensions buffer
    glBindBuffer(GL_ARRAY_BUFFER, bufferDimensions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dimensions[0]) * dimensions.size(), dimensions.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS));
    glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_DIMENSIONS), 1);

    //populating positions buffer
    glBindBuffer(GL_ARRAY_BUFFER, bufferPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION));
    glVertexAttribPointer(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(static_cast<unsigned int>(Cuboid::Locations::LOCATION_TRANSPOSITION), 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cuboid::getBuffer(Cuboid::Buffers::BUFFER_INDEX));
    glDrawElementsInstancedBaseVertex(GL_TRIANGLES, Cuboid::getNumberOfIndices(), GL_UNSIGNED_INT,
        0, faceData.size(), 0);

    glBindVertexArray(0);

    glDeleteBuffers(1, &bufferAtlas);
    glDeleteBuffers(1, &bufferRotation);
    glDeleteBuffers(1, &bufferDimensions);
    glDeleteBuffers(1, &bufferPosition);
    glDeleteVertexArrays(1, &vao);
}