#pragma once
#include <unordered_map>

#include "Utilities/vector3d.h"
#include "platformCommon.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

struct ChunkData
{
private:
	Utils::Vector3d<uint32_t> m_blockData;
	glm::ivec2 m_coords;
    std::unordered_map<glm::ivec3, uint32_t> m_additionalBlockData; //this will be used with physics affected or dynamic blocks
public:
    inline unsigned int& getBlock(const int& y, const int& x, const int& z) { return m_blockData(y + 1, x + 1, z + 1); };
    inline const unsigned int& getBlock(const int& y, const int& x, const int& z) const { return m_blockData(y + 1, x + 1, z + 1); };
    inline glm::ivec2& getCoords() { return m_coords; };
    inline const glm::ivec2& getCoords() const { return m_coords; };
    inline void clear() { m_blockData.clear(); m_coords = glm::ivec2(0, 0); };

    ChunkData() { m_blockData.resize(constWorldHeight + 2, constChunkSize + 2, constChunkSize + 2); };
    ~ChunkData() = default;

    ChunkData(ChunkData&& other) noexcept = default;
    ChunkData& operator=(ChunkData&& other) noexcept = default;

    ChunkData(const ChunkData& other) = delete;
    ChunkData& operator=(const ChunkData& other) = delete;

};

