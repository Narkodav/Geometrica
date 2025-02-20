#pragma once
#include <unordered_map>
#include <memory>

#include "Utilities/vector3d.h"
#include "platformCommon.h"
#include "DataManagement/Blocks/DynamicBlockTemplate.h"
#include "DataManagement/Blocks/DynamicBlockDataFactory.h"
#include "DataManagement/DataRepository.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

struct ChunkData
{

private:
	Utils::Vector3d<uint32_t> m_blockData;
	glm::ivec2 m_coords;
    std::unordered_map<glm::ivec3, std::unique_ptr<DynamicBlockDataTemplate>> m_dynamicBlockData; //this will be used with physics affected or dynamic blocks
public:
    inline void setBlock(const int& y, const int& x, const int& z, const uint32_t& block) 
    { 
        m_blockData(y + 1, x + 1, z + 1) = block;
        if (block == DataRepository::airId)
            return;
        const auto& blockData = DataRepository::getBlock(block);
        if(blockData->isDynamic())
            m_dynamicBlockData[glm::ivec3(x, y, z)] = 
            std::move(DynamicBlockDataFactory::createBlockData(blockData->getType(), glm::ivec3(x, y, z)));
    };

    inline void setBlock(const int& y, const int& x, const int& z, const BlockData& block)
    {
        m_blockData(y + 1, x + 1, z + 1) = block.blockId;
        if(block.dynamicData != nullptr)
            m_dynamicBlockData[glm::ivec3(x, y, z)] = std::move(block.dynamicData->clone());
    };

    inline BlockData getBlock(const int& y, const int& x, const int& z) const
    { 
        auto dynamicData = m_dynamicBlockData.find(glm::ivec3(x, y, z));
        if (dynamicData != m_dynamicBlockData.end())
            return BlockData{ m_blockData(y + 1, x + 1, z + 1),
            std::move(dynamicData->second->clone()) };
        return BlockData{ m_blockData(y + 1, x + 1, z + 1),
        nullptr };
    };

    inline std::unique_ptr<DynamicBlockDataTemplate> getBlockData(const int& y, const int& x, const int& z) const
    {
        auto dynamicData = m_dynamicBlockData.find(glm::ivec3(x, y, z));
        if (dynamicData != m_dynamicBlockData.end())
            return std::move(dynamicData->second->clone());
        return nullptr;
    };

    inline uint32_t getBlockId(const int& y, const int& x, const int& z) const
    {
        return m_blockData(y + 1, x + 1, z + 1);
    };

    inline const glm::ivec2 getCoords() const { return m_coords; };
    inline void setCoords(glm::ivec2 newCoords) { m_coords = newCoords; };
    inline void clear() { m_blockData.clear(); m_coords = glm::ivec2(0, 0); };

    ChunkData() { m_blockData.resize(constWorldHeight + 2, constChunkSize + 2, constChunkSize + 2); };
    ~ChunkData() = default;

    ChunkData(ChunkData&& other) noexcept = default;
    ChunkData& operator=(ChunkData&& other) noexcept = default;

    ChunkData(const ChunkData& other) = delete;
    ChunkData& operator=(const ChunkData& other) = delete;

};

