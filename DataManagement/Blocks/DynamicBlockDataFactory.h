#pragma once
#include "DynamicBlockTemplate.h"
#include <memory.h>

class DynamicBlockDataFactory
{
public:
    // Define type for block creation function
    using BlockDataCreator = std::function<std::unique_ptr<DynamicBlockDataTemplate>(const glm::ivec3&, uint32_t, uint32_t, uint32_t)>;

    // Register a new block type with its creator function
    static void registerBlockDataType(const std::string& blockType, BlockDataCreator creator) {
        creators[blockType] = creator;
    }

    // Create block data based on block type
    static std::unique_ptr<DynamicBlockDataTemplate> createBlockData(const std::string& blockType, const glm::ivec3& blockCoord, 
        uint32_t id, uint32_t lastUpdatedTick, uint32_t nextUpdateScheduledTick) {
        auto it = creators.find(blockType);
        if (it != creators.end()) {
            return it->second(blockCoord, id, lastUpdatedTick, nextUpdateScheduledTick);
        }
        std::cerr << "Unknown block type: " << blockType << std::endl;
        __debugbreak();
    }

private:
    static inline std::unordered_map<std::string, BlockDataCreator> creators;
};

template<typename T>
class DynamicBlockDataRegistrar {
public:
    DynamicBlockDataRegistrar(const std::string& typeName) {
        DynamicBlockDataFactory::registerBlockDataType(typeName,
            [](const glm::ivec3& blockCoord, uint32_t id,
                uint32_t lastUpdatedTick, uint32_t nextUpdateScheduledTick) {
                    auto blockData = std::make_unique<T>(blockCoord, id, lastUpdatedTick, nextUpdateScheduledTick);
                return blockData;
            }
        );
    }
};