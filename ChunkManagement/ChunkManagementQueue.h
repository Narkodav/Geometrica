#pragma once
#include <queue>
#include <unordered_map>
#include <optional>
#include <array>
#include <memory>
#include <variant>
#include <bitset>
#include <iostream>
#include <set>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

#include "Utilities/SortedVector.h"
#include "platformCommon.h"

class ChunkManagementQueue
{
public:
    struct GenerateComparator {
        glm::ivec2 playerPos;
        GenerateComparator(const glm::ivec2& pos) : playerPos(pos) {};
        bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
            auto distA = glm::distance(glm::vec2(a), glm::vec2(playerPos));
            auto distB = glm::distance(glm::vec2(b), glm::vec2(playerPos));
            return distA < distB;
        }
    };

    struct StoreComparator {
        glm::ivec2 playerPos;
        StoreComparator(const glm::ivec2& pos) : playerPos(pos) {};
        bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
            auto distA = glm::distance(glm::vec2(a), glm::vec2(playerPos));
            auto distB = glm::distance(glm::vec2(b), glm::vec2(playerPos));
            return distA > distB;
        }
    };

    enum class QueueType {
        GENERATION = 0,      // Normal generation to chunk
        STORAGE = 1,         // Normal storage
        DIRECT_STORAGE = 2,   // Direct generation to storage
        NONE = 3   // For return values
    };

    struct QueueElement
    {
        QueueType type;
        glm::ivec2 pos;
    };

private:

    using SortedVectorLoad = Utils::SortedVector<glm::ivec2, GenerateComparator>;
    using SortedVectorStore = Utils::SortedVector<glm::ivec2, StoreComparator>;
    using RegularVector = std::vector<glm::ivec2>;

    SortedVectorLoad m_generateQueue{ GenerateComparator(m_lastRecordedPlayerCoords) };
    SortedVectorStore m_storeQueue{ StoreComparator(m_lastRecordedPlayerCoords) };
    RegularVector m_directStoreQueue;

    glm::ivec2 m_lastRecordedPlayerCoords = glm::ivec2(0, 0);

    std::function<void()> m_cachedPop;
    QueueElement m_cachedElem;
    bool m_cachedToggle;

    bool router = 0;

    void removeFromQueue(const glm::ivec2& pos, QueueType queueType) {
        switch (queueType) {
        case QueueType::GENERATION:
            m_generateQueue.remove(pos);
            break;
        case QueueType::STORAGE:
            m_storeQueue.remove(pos);
            break;
        case QueueType::DIRECT_STORAGE:
            m_directStoreQueue.erase(std::remove(m_directStoreQueue.begin(),
                m_directStoreQueue.end(), pos), m_directStoreQueue.end());
            break;
        }
    }

    void addToGenerateQueue(const glm::ivec2& pos) {
        if (std::find(m_generateQueue.begin(), m_generateQueue.end(), pos)
            != m_generateQueue.end())
        {
            return;
        }

        auto storeIt = std::find(m_storeQueue.begin(), m_storeQueue.end(), pos);
        if (storeIt != m_storeQueue.end())
        {
            m_storeQueue.remove(storeIt);
        }
           

        auto directIt = std::find(m_directStoreQueue.begin(), m_directStoreQueue.end(), pos);
        if (directIt != m_directStoreQueue.end())
        {
            m_directStoreQueue.erase(directIt);
        }
            

        m_generateQueue.insert(pos);
    }

    void addToStoreQueue(const glm::ivec2& pos) {

        auto generateIt = std::find(m_generateQueue.begin(), m_generateQueue.end(), pos);
        if (generateIt != m_generateQueue.end())
        {
            m_generateQueue.remove(generateIt);
            m_directStoreQueue.push_back(pos);
            return;
        }

        if (std::find(m_storeQueue.begin(), m_storeQueue.end(), pos) != m_storeQueue.end() ||
            std::find(m_directStoreQueue.begin(), m_directStoreQueue.end(), pos) != m_directStoreQueue.end())
        {
            return;
        }

        m_storeQueue.insert(pos);
    }

    void addToDirectQueue(const glm::ivec2& pos) {

        auto generateIt = std::find(m_generateQueue.begin(), m_generateQueue.end(), pos);
        if (generateIt != m_generateQueue.end())
        {
            m_generateQueue.remove(generateIt);
        }

        auto storeIt = std::find(m_storeQueue.begin(), m_storeQueue.end(), pos);
        if (storeIt != m_storeQueue.end())
        {
            m_storeQueue.remove(storeIt);
        }

        if (std::find(m_directStoreQueue.begin(), m_directStoreQueue.end(), pos)
            != m_directStoreQueue.end())
        {
            return;
        }

        m_directStoreQueue.push_back(pos);
    }

public:

    void resort(const glm::ivec2& playerPos)
    {
        m_lastRecordedPlayerCoords = playerPos;
        m_generateQueue.resort(GenerateComparator(m_lastRecordedPlayerCoords));
        m_storeQueue.resort(StoreComparator(m_lastRecordedPlayerCoords));
    };

    void push(QueueElement element)
    {
        switch (element.type) {
        case QueueType::GENERATION:
            addToGenerateQueue(element.pos);
            break;
        case QueueType::STORAGE:
            addToStoreQueue(element.pos);
            break;
        case QueueType::DIRECT_STORAGE:
            addToDirectQueue(element.pos);
            break;
        }
    };

    QueueElement pop()
    {
        if (m_cachedPop)
        {
            m_cachedPop();
            m_cachedPop = nullptr;
            if (m_cachedToggle)
                router = !router;
            return m_cachedElem;
        }

        char byte = 0;
        byte = ((((byte | m_generateQueue.empty()) << 1) | m_storeQueue.empty()) << 1) | m_directStoreQueue.empty();
        QueueElement elem;

        switch (byte)
        {
        case 0b111:
            return { QueueType::NONE, glm::ivec2(0, 0) };
            break;
        case 0b110:
            elem = { QueueType::DIRECT_STORAGE, m_directStoreQueue.back() };
            m_directStoreQueue.pop_back();
            return elem;
            break;
        case 0b100:
        case 0b101:
            elem = { QueueType::STORAGE, m_storeQueue.front() };
            m_storeQueue.pop_front();
            router = 0;
            return elem;
            break;
        case 0b010:
        case 0b011:
            elem = { QueueType::GENERATION, m_generateQueue.front() };
            m_generateQueue.pop_front();
            router = 1;
            return elem;
            break;
        case 0b001:
        case 0b000:
            if (router)
            {
                elem = { QueueType::GENERATION, m_generateQueue.front() };
                m_generateQueue.pop_front();
                router = !router;
                return elem;
            }
            else
            {
                elem = { QueueType::STORAGE, m_storeQueue.front() };
                m_storeQueue.pop_front();
                router = !router;
                return elem;
            }
            break;
        default:
            return { QueueType::NONE, glm::ivec2(0, 0) };
            break;
        }
    }

    QueueElement front()
    {
        if (m_cachedPop && m_cachedElem.type != QueueType::NONE)
            return m_cachedElem;
        char byte = 0;
        byte = ((((byte | m_generateQueue.empty()) << 1) | m_storeQueue.empty()) << 1) | m_directStoreQueue.empty();

        m_cachedToggle = 0;

        switch (byte)
        {
        case 0b111:
            m_cachedPop = []() {};
            m_cachedElem = { QueueType::NONE, glm::ivec2(0, 0) };
            return m_cachedElem;
            break;
        case 0b110:
            m_cachedPop = [&]() {m_directStoreQueue.pop_back(); };
            m_cachedElem = { QueueType::DIRECT_STORAGE, m_directStoreQueue.back() };
            return m_cachedElem;
            break;
        case 0b100:
        case 0b101:
            m_cachedPop = [&]() {m_storeQueue.pop_front(); };
            m_cachedElem = { QueueType::STORAGE, m_storeQueue.front() };
            router = 0;
            return m_cachedElem;
            break;
        case 0b010:
        case 0b011:
            m_cachedPop = [&]() {m_generateQueue.pop_front(); };
            m_cachedElem = { QueueType::GENERATION, m_generateQueue.front() };
            router = 1;
            return m_cachedElem;
            break;
        case 0b001:
        case 0b000:
            if (router)
            {
                m_cachedToggle = 1;
                m_cachedPop = [&]() {m_generateQueue.pop_front(); };
                m_cachedElem = { QueueType::GENERATION, m_generateQueue.front() };
                return m_cachedElem;
            }
            else
            {
                m_cachedToggle = 1;
                m_cachedPop = [&]() {m_storeQueue.pop_front(); };
                m_cachedElem = { QueueType::STORAGE, m_storeQueue.front() };
                return m_cachedElem;
            }
            break;
        default:
            m_cachedPop = []() {};
            m_cachedElem = { QueueType::NONE, glm::ivec2(0, 0) };
            return m_cachedElem;
            break;
        }
    }

};

