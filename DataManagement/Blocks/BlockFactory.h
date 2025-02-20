#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "BlockTemplate.h"

class BlockFactory {
public:
    // Define type for block creation function
    using BlockCreator = std::function<std::unique_ptr<BlockTemplate>(const std::string&)>;

    // Register a new block type with its creator function
    static void registerBlockType(const std::string& blockType, BlockCreator creator) {
        creators[blockType] = creator;
    }

    // Create block based on type
    static std::unique_ptr<BlockTemplate> createBlock(const std::string& blockType, const std::string& filePath) {
        auto it = creators.find(blockType);
        if (it != creators.end()) {
            return it->second(filePath);
        }
        return nullptr;
    }

private:
    static inline std::unordered_map<std::string, BlockCreator> creators;
};

template<typename T>
class BlockRegistrar {
public:
    BlockRegistrar(const std::string& typeName) {
        BlockFactory::registerBlockType(typeName,
            [](const std::string& filepath) {
                auto block = std::make_unique<T>();
                if (!block->loadFromFile(filepath)) {
                    return std::unique_ptr<T>(nullptr);
                }
                return block;
            }
        );
    }
};