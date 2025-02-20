#pragma once
#include <memory>
#include <vector>

#include "MultiThreading/EventPolicy.h"
//#include "DataManagement/Blocks/DynamicBlockTemplate.h"

#include <glm/glm.hpp>

class DynamicBlockDataTemplate;

//utility structs:
struct BlockModifiedEvent {
    glm::ivec3 blockCoord;
    unsigned int blockId;
    std::unique_ptr<DynamicBlockDataTemplate> newDynamicData = nullptr; //not necessary
};

struct BlockRemeshEvent {
    glm::ivec3 blockCoord;
    unsigned int currentId;
    unsigned int previousId;
};

struct BlockModifiedBulkEvent {
    std::vector<BlockModifiedEvent> modifications;
};

struct BlockRemeshBulkEvent {
    std::vector<BlockRemeshEvent> remeshes;
};

//event enum:
enum class GameEventTypes {
    BLOCK_MODIFIED,
    BLOCK_REMESH,
    BLOCK_MODIFIED_BULK,
    BLOCK_REMESH_BULK,
    EVENT_NUM
};

struct GameEventPolicy : MT::EventPolicy<GameEventTypes, static_cast<size_t>(GameEventTypes::EVENT_NUM)> {};

template<>
template<>
struct GameEventPolicy::Traits<GameEventTypes::BLOCK_MODIFIED> {
    using Signature = void(BlockModifiedEvent); //block coord and id
};

template<>
template<>
struct GameEventPolicy::Traits<GameEventTypes::BLOCK_REMESH> {
    using Signature = void(BlockRemeshEvent); //block coord and id
};

template<>
template<>
struct GameEventPolicy::Traits<GameEventTypes::BLOCK_MODIFIED_BULK> {
    using Signature = void(BlockModifiedBulkEvent); //vector of modifications
};

template<>
template<>
struct GameEventPolicy::Traits<GameEventTypes::BLOCK_REMESH_BULK> {
    using Signature = void(BlockRemeshBulkEvent); //vector of remeshes
};

//template<>
//template<>
//struct GameEventPolicy::Traits<GameEventTypes::BLOCK_BROKEN> {
//    using Signature = void(glm::ivec3); //block coord
//};