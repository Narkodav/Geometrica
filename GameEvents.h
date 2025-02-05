#pragma once
#include "MultiThreading/EventPolicy.h"

#include <glm/glm.hpp>

//utility structs:
struct BlockModifiedEvent {
    glm::ivec3 blockCoord;
    unsigned int blockId;
};

struct BlockRemeshEvent {
    glm::ivec3 blockCoord;
    unsigned int currentId;
    unsigned int previousId;
};

//event enum:
enum class GameEventTypes {
    BLOCK_MODIFIED,
    BLOCK_REMESH,
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

//template<>
//template<>
//struct GameEventPolicy::Traits<GameEventTypes::BLOCK_BROKEN> {
//    using Signature = void(glm::ivec3); //block coord
//};