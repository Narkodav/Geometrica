#pragma once
#include <unordered_map>
#include <memory>

#include "GameEvents.h"
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

    //do not use this with dynamic blocks
    inline void setBlock(const int& y, const int& x, const int& z, const uint32_t& block) 
    { 
        m_blockData(y + 1, x + 1, z + 1) = block;
        const auto& iter = m_dynamicBlockData.find(glm::ivec3(x, y, z));
        if (iter != m_dynamicBlockData.end())
            m_dynamicBlockData.erase(iter);
    };

    inline void setBlock(const int& y, const int& x, const int& z, const BlockData& block)
    {
        m_blockData(y + 1, x + 1, z + 1) = block.blockId;
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

    
    BlockModifiedBulkEvent updateDynamicBlocks(
        const MapUpdateQueryInterface& map, const GameClockInterface& clock) const
    {
        if (m_dynamicBlockData.empty())
            return BlockModifiedBulkEvent();

        BlockModifiedBulkEvent bulkEvent;
        for (auto& blockData : m_dynamicBlockData)
        {
            if (!blockData.second->shouldUpdate(clock))
                continue;
            glm::ivec3 position;
            BlockModifiedBulkEvent bulkModificationBuffer;
            bulkModificationBuffer = blockData.second->update(map, clock);
            for (BlockModifiedEvent& ptr : bulkModificationBuffer.modifications) {
                bulkEvent.modifications.emplace_back(std::move(ptr));
            }
        }

        return bulkEvent;
    };

    //BlockModifiedBulkEvent updateDynamicBlock(const MapUpdateQueryInterface& map, const glm::ivec3& blockCoord) const
    //{
    //    if (m_dynamicBlockData.empty())
    //        return BlockModifiedBulkEvent();

    //    auto data = m_dynamicBlockData.find(blockCoord);
    //    if (data != m_dynamicBlockData.end())
    //        return data->second->update(map);
    //    return BlockModifiedBulkEvent();
    //};

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

