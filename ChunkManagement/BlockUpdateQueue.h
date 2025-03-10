#pragma once
#include <set>

#include "platformCommon.h"
#include "DataManagement/Blocks/DynamicBlockTemplate.h"
#include "GameEvents.h"

struct UpdateData
{
    glm::ivec3 coord;
    uint32_t tick;
    std::string typeName;
};

class BlockUpdateQueue
{
private:

    std::vector<UpdateData> m_scheduledUpdates;
    std::set<glm::ivec3, ComparatorIvec3> m_uniqueCoords;

public:

    bool push(BlockModifiedEvent& event)
    {
        auto it = m_uniqueCoords.find(event.blockCoord);
        if (it != m_uniqueCoords.end())
        {
            auto itVec = std::find_if(m_scheduledUpdates.begin(), m_scheduledUpdates.end(),
                [coord = event.blockCoord](const UpdateData& value) {
                    return value.coord == coord;
                });

            if (itVec == m_scheduledUpdates.end()) {
                // This is an error condition - coord exists in set but not in vector
                return false;
            }

            if (event.newDynamicData->canOverride(itVec->typeName))
            {
                *itVec = UpdateData{
                    event.blockCoord, event.newDynamicData->getNextUpdatedTick(), event.newDynamicData->getType() };
                return true;
            }
            return false;
        }
        else
        {
            UpdateData newUpdate{
                event.blockCoord,
                event.newDynamicData->getNextUpdatedTick(),
                event.newDynamicData->getType()
            };

            auto it = std::lower_bound(m_scheduledUpdates.begin(), m_scheduledUpdates.end(), newUpdate,
                [](const UpdateData& a, const UpdateData& b) { return a.tick > b.tick; });

            m_scheduledUpdates.insert(it, newUpdate);
            m_uniqueCoords.insert(event.blockCoord);
            return true;
        }
    }

    bool empty() const {
        return m_scheduledUpdates.empty();
    }

    const UpdateData& top() const {
        return m_scheduledUpdates.back();
    }

    void pop() {
        m_uniqueCoords.erase(m_scheduledUpdates.back().coord);
        m_scheduledUpdates.pop_back();
    }
};

