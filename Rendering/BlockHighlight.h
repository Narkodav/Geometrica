#pragma once
#include <array>
#include "platformCommon.h"
#include "Rendering/Shader.h"
#include "DataManagement/DataRepository.h"

class BlockHighlight
{
public:

    struct BufferData
    {
        std::vector<glm::vec3> transpositions;
        std::vector<unsigned int> atlasIndices;
        std::vector<unsigned char> faceIndices;
    };

    // Positions front (X+) face
    static inline const std::array<glm::vec3, 4> positions = {
        glm::vec3(0.5f, -0.5f,  0.5f),      // vertex 0: bottom-left
        glm::vec3(0.5f, -0.5f, -0.5f),      // vertex 1: bottom-right
        glm::vec3(0.5f,  0.5f, -0.5f),      // vertex 2: top-right
        glm::vec3(0.5f,  0.5f,  0.5f)       // vertex 3: top-left
    };

    // Two triangles (faces), each with 3 indices
    static inline const std::array<glm::uvec3, 2> indices = {
        glm::uvec3(0, 1, 2),    // first triangle
        glm::uvec3(2, 3, 0)     // second triangle
    };

    enum class Buffers
    {
        POSITIONS = 0,
        TRANSPOSITIONS,
        ATLAS_INDEX,
        FACE_INDEX,
        INDEX,
        NUM,
    };

    enum class Locations
    {
        POSITIONS = 0,
        TRANSPOSITIONS,
        ATLAS_INDEX,
        FACE_INDEX,
        NUM,
    };

private:
    unsigned int m_VAO;
    std::array<unsigned int, static_cast<size_t>(Buffers::NUM)> m_buffers;
    unsigned int m_atlasIndex;
    float m_delta;
    
    BufferData makeBufferData(Area selectedArea);

public:
    BlockHighlight(unsigned int atlasIndex, float delta = 0.002f);
    ~BlockHighlight();

    //cornerPosition is left-bottom-back corner, sizes is depth, width and height
    void draw(Shader& shader, Area selectedArea,
    glm::mat4 view, glm::mat4 projection);

};

