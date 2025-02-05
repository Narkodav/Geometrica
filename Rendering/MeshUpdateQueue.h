#pragma once
#include <variant>
#include <functional>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

#include "Utilities/SortedVector.h"
#include "MultiThreading/MemoryPool.h"
#include "ChunkManagement/Chunk.h"

class MeshUpdateQueue
{
public:
    struct MeshComparator {
        glm::ivec2 playerPos;
        MeshComparator(const glm::ivec2& pos) : playerPos(pos) {};
        bool operator()(const MT::MemoryPool<Chunk>::SharedPointer& a, 
        const MT::MemoryPool<Chunk>::SharedPointer& b) const {
            auto accessA = a->getReadAccess();
            auto accessB = a->getReadAccess();
            auto distA = glm::distance(glm::vec2(accessA.data.getCoords()), glm::vec2(playerPos));
            auto distB = glm::distance(glm::vec2(accessB.data.getCoords()), glm::vec2(playerPos));
            return distA < distB;
        }
    };

    struct UpdateComparator {
        glm::ivec2 playerPos;
        UpdateComparator(const glm::ivec2& pos) : playerPos(pos) {};
        bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
            auto distA = glm::distance(glm::vec2(a), glm::vec2(playerPos));
            auto distB = glm::distance(glm::vec2(b), glm::vec2(playerPos));
            return distA < distB;
        }
    };

    struct DeleteComparator {
        glm::ivec2 playerPos;
        DeleteComparator(const glm::ivec2& pos) : playerPos(pos) {};
        bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
            auto distA = glm::distance(glm::vec2(a), glm::vec2(playerPos));
            auto distB = glm::distance(glm::vec2(b), glm::vec2(playerPos));
            return distA > distB;
        }
    };

    enum class QueueType {
        MESHING = 0,
        DELETION = 1,
        UPDATE = 2,
        NONE = 3
    };

    struct QueueElement
    {
        QueueType type;
        std::variant<MT::MemoryPool<Chunk>::SharedPointer, glm::ivec2> data;

        QueueElement() : type(QueueType::NONE) {};

        template<typename T>
        QueueElement(QueueType type, T data) : type(type), data(data) {};
    };

private:

    using SortedVectorMesh = Utils::SortedVector<MT::MemoryPool<Chunk>::SharedPointer, MeshComparator>;
    using SortedVectorDelete = Utils::SortedVector<glm::ivec2, DeleteComparator>;
    using SortedVectorUpdate = Utils::SortedVector<glm::ivec2, UpdateComparator>;

    SortedVectorMesh m_meshVector{ MeshComparator(m_lastRecordedPlayerCoords) };
    SortedVectorDelete m_deleteVector{ DeleteComparator(m_lastRecordedPlayerCoords) };
    SortedVectorUpdate m_updateVector{ UpdateComparator(m_lastRecordedPlayerCoords) };

    glm::ivec2 m_lastRecordedPlayerCoords = glm::ivec2(0, 0);

    std::function<void()> m_cachedPop;
    QueueElement m_cachedElem;
    bool m_cachedToggle;

    char router = 0;

    void addToMeshQueue(const MT::MemoryPool<Chunk>::SharedPointer& chunk) {


        if (std::find(m_meshVector.begin(), m_meshVector.end(), chunk) != m_meshVector.end())
            return;
        {
            auto access = chunk->getReadAccess();
            m_deleteVector.remove(access.data.getCoords());
            m_updateVector.remove(access.data.getCoords());
        }
        m_meshVector.insert(chunk);
    }

    void addToDeleteQueue(const glm::ivec2& pos) {

        m_meshVector.remove(std::find_if(m_meshVector.begin(), m_meshVector.end(),
            [pos](const MT::MemoryPool<Chunk>::SharedPointer& chunk) {
                auto access = chunk->getReadAccess();
                return access.data.getCoords() == pos;
            }));
        m_updateVector.remove(pos);
        m_deleteVector.insert(pos);
    }

    void addToUpdateQueue(const glm::ivec2& pos) {

        if (std::find(m_updateVector.begin(), m_updateVector.end(), pos) != m_updateVector.end())
            return;
        if (std::find_if(m_meshVector.begin(), m_meshVector.end(),
            [pos](const MT::MemoryPool<Chunk>::SharedPointer& chunk) {
                auto access = chunk->getReadAccess();
                return access.data.getCoords() == pos;
            }) != m_meshVector.end())
            return;
        if (std::find(m_deleteVector.begin(), m_deleteVector.end(), pos) != m_deleteVector.end())
            return;

        m_updateVector.insert(pos);
    }

public:

    void resort(const glm::ivec2& playerPos)
    {
        m_lastRecordedPlayerCoords = playerPos;
        m_meshVector.resort(MeshComparator(m_lastRecordedPlayerCoords));
        m_deleteVector.resort(DeleteComparator(m_lastRecordedPlayerCoords));
        m_updateVector.resort(UpdateComparator(m_lastRecordedPlayerCoords));
    };

    void push(QueueElement element)
    {
        switch (element.type) {
        case QueueType::MESHING:
            addToMeshQueue(std::get<MT::MemoryPool<Chunk>::SharedPointer>(element.data));
            break;
        case QueueType::DELETION:
            addToDeleteQueue(std::get<glm::ivec2>(element.data));
            break;
        case QueueType::UPDATE:
            addToUpdateQueue(std::get<glm::ivec2>(element.data));
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
                router = (router + 1) % 3;
            return m_cachedElem;
        }

        for (int attempt = 0; attempt < 3; attempt++) {
            switch (router) {
            case 0:
                if (!m_meshVector.empty()) {
                    router = (router + 1) % 3;
                    return QueueElement(QueueType::MESHING, m_meshVector.front());
                }
                break;

            case 1:
                if (!m_deleteVector.empty()) {
                    router = (router + 1) % 3;
                    return QueueElement(QueueType::DELETION, m_deleteVector.front());
                }
                break;

            case 2:
                if (!m_updateVector.empty()) {
                    router = (router + 1) % 3;
                    return QueueElement(QueueType::UPDATE, m_updateVector.front());
                }
                break;
            }
            router = (router + 1) % 3;
        }

        return QueueElement(QueueType::NONE, glm::ivec2(0, 0));
            
    }

    QueueElement front()
    {
        if (m_cachedPop && m_cachedElem.type != QueueType::NONE)
            return m_cachedElem;

        m_cachedToggle = 0;
        for (int attempt = 0; attempt < 3; attempt++) {
            switch (router) {
            case 0:
                if (!m_meshVector.empty()) {
                    m_cachedPop = [&]() {m_meshVector.pop_front(); };
                    m_cachedToggle = 1;
                    m_cachedElem = QueueElement(QueueType::MESHING, m_meshVector.front());
                    return m_cachedElem;
                }
                break;

            case 1:
                if (!m_deleteVector.empty()) {
                    m_cachedPop = [&]() {m_deleteVector.pop_front(); };
                    m_cachedToggle = 1;
                    m_cachedElem = QueueElement(QueueType::DELETION, m_deleteVector.front());
                    return m_cachedElem;
                }
                break;

            case 2:
                if (!m_updateVector.empty()) {
                    m_cachedPop = [&]() {m_updateVector.pop_front(); };
                    m_cachedToggle = 1;
                    m_cachedElem = QueueElement(QueueType::UPDATE, m_updateVector.front());
                    return m_cachedElem;
                }
                break;
            }
            router = (router + 1) % 3;
        }
        m_cachedPop = []() {};
        m_cachedToggle = 0;
        m_cachedElem = QueueElement(QueueType::NONE, glm::ivec2(0, 0));
        return m_cachedElem;
    }
};

