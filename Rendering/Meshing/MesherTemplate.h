#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

#include <vector>
#include <queue>
#include <unordered_map>

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/compatibility.hpp>

#include "ChunkManagement/Chunk.h"
#include "GameEvents.h"

class MesherTemplate {
public:
    virtual ~MesherTemplate() = 0; //define proper buffer cleanup in derived classes

    // Core meshing operations
    virtual void removeData(const glm::ivec3& blockCoord, Chunk::ReadAccess* access = nullptr) = 0;
    virtual void addData(const glm::ivec3& blockCoords, const unsigned int& id, Chunk::ReadAccess* access = nullptr) = 0;
    /*virtual void replaceData(const BlockRemeshEvent& data, Chunk::ReadAccess* access = nullptr) = 0;*/

    // Buffer management
    virtual void buildBuffers() = 0;
    virtual void clearBuffers() = 0;

    // Rendering
    virtual void draw() = 0;

    // Type and state queries
    virtual BlockMesherType getType() const = 0;
    virtual bool isDirty() const = 0;
    virtual bool isEmpty() const = 0;

protected:
    bool m_needsRebuild = false;  // This might be the only common state needed
};

