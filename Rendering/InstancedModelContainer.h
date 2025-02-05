#pragma once
#include "model.h"

struct DrawElementsIndirectCommand {
    GLuint count;         // Number of indices to draw
    GLuint instanceCount; // Number of instances to draw
    GLuint firstIndex;    // Starting index in the index buffer
    GLint  baseVertex;    // Value added to each index
    GLuint baseInstance;  // First instance ID
};

struct InstancedData
{
    glm::ivec3 modelCoords = {};
    glm::vec3 transpositions = {};
    unsigned int materialIndex = {};
};

class InstancedModelContainer
{
    enum class BufferTypesPerContainer : unsigned int
    {
        BUFFER_POSITION = 0,
        BUFFER_UV,
        BUFFER_NORMALS,
        BUFFER_INDEX,
        BUFFER_INSTANCES,
        BUFFER_COMMANDS,
        BUFFER_NUM,
    };

    enum Locations
    {
        LOCATION_POSITION = 0,
        LOCATION_UV = 1,
        LOCATION_NORMAL = 2,
        LOCATION_TRANSPOSITION = 3,
        LOCATION_INDEX = 4,
    };

    struct ModelInfo {
        uint32_t indexCount;  // Number of indices
        uint32_t firstIndex;  // Starting index in buffer
        int32_t baseVertex;   // Base vertex offset
    };

private:
    const Model& m_modelHandle;
    /*GLuint m_buffers[BufferTypesPerContainer::BUFFER_NUM];*/
    std::vector<DrawElementsIndirectCommand> m_drawCommands;

public:
    InstancedModelContainer(const Model& model) : m_modelHandle(model) {};

    void addMeshInstances(std::vector<InstancedData> data, std::string meshName);

};

