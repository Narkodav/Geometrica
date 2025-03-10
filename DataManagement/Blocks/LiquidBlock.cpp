#include "LiquidBlock.h"
#include "dataManagement/DataRepository.h"

//Cuboid block file structure:
//#Block type --type-- //type matches class name
//#id --id--
//#material still --name--
//#material flowing --name--
//#viscosity
//#hitbox --hitbox name-- (.hbx file) //optional

bool LiquidBlock::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    setName(parseName(filepath));

    std::string line;
    std::vector<std::string> materials;
    float ticksToSpread = 0.0f;

    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty() || line[0] != '#') continue;

        std::istringstream iss(line.substr(1)); // Remove the #
        std::string tag;
        iss >> tag;

        if (tag == "Block") {
            std::string type;
            iss >> tag >> type; // Read "type" and the type value
            if (type != "LiquidBlock") {
                std::cerr << "Invalid block type in file: " << type << std::endl;
                return false;
            }
        }
        else if (tag == "id") {
            std::string id;
            iss >> id;
            setId(std::stoul(id));
        }
        else if (tag == "viscosity") {
            iss >> ticksToSpread;
        }
        else if (tag == "hitbox") {
            std::string hitboxName;
            iss >> hitboxName;
            if (!loadHitboxData(hitboxName)) {
                std::cerr << "Failed to load hitbox: " << hitboxName << std::endl;
                return false;
            }
        }
        else if (tag == "material") {
            std::string materialName;
            iss >> tag >> materialName; // Read "type" and the type value
            materials.push_back(materialName);
        }
    }

    // Verify we got all required materials
    if (materials.size() != static_cast<size_t>(Materials::MATERIAL_NUM)) {
        std::cerr << "Invalid number of materials: " << materials.size()
            << " (expected 6)" << std::endl;
        return false;
    }

    // Set materials for each face
    for (int i = 0; i < static_cast<size_t>(Materials::MATERIAL_NUM); ++i) {
        if (!setFaceMaterial(i, materials[i])) {
            std::cerr << "Failed to set material " << materials[i]
                << " for face " << i << std::endl;
            return false;
        }
    }

    // Set liquid specific properties
    m_ticksToSpread = ticksToSpread;

    file.close();
    return true;
}

bool LiquidBlock::setFaceMaterial(int faceIndex, const std::string& materialName) {
    m_atlasIndices[faceIndex] =
        DataRepository::getAtlas().getStorageBufferIndex(materialName);
    return true;
}

void LiquidBlockData::onAdjacentUpdate(const MapUpdateInterface& map, const GameClockInterface& clock) {
    m_nextUpdateScheduledTick = clock.getGlobalTime() + static_cast<const LiquidBlock*>(m_parentBlockType)->getTicksToSpread();
};

BlockModifiedBulkEvent LiquidBlockData::update(const MapUpdateInterface& map, const GameClockInterface& clock) {
    //if (m_isSource)
    //    return updateForSource(map, gameEvents);
    //else return updateForFlowing(map, gameEvents);
    if (m_currentLevel - levelDecline == 0)
        return BlockModifiedBulkEvent();

    BlockModifiedBulkEvent event;
    uint32_t blockId = map.getBlockId(m_position);
    m_lastUpdatedTick = clock.getGlobalTime();
    m_nextUpdateScheduledTick = m_lastUpdatedTick + 30;

    glm::ivec3 adjPos = m_position + constDirectionVectors3DHashed[
        static_cast<size_t>(Directions3DHashed::DIRECTION_DOWN)];
    if (m_id == map.getBlockId(adjPos) && !m_isSource)
        return BlockModifiedBulkEvent();

    adjPos = m_position + constDirectionVectors3DHashed[
        static_cast<size_t>(Directions3DHashed::DIRECTION_FORWARD)];

    if (DataRepository::airId == map.getBlockId(adjPos))
    {
        auto data = static_cast<const LiquidBlock*>(m_parentBlockType)->getBlockData(adjPos, m_id, clock);
        static_cast<LiquidBlockData*>(data.get())->m_currentLevel = m_currentLevel - levelDecline;
        static_cast<LiquidBlockData*>(data.get())->m_isSource = false;
        event.modifications.emplace_back(adjPos, m_id, std::move(data));
    }

    adjPos = m_position + constDirectionVectors3DHashed[
        static_cast<size_t>(Directions3DHashed::DIRECTION_BACKWARD)];
    if (DataRepository::airId == map.getBlockId(adjPos))
    {
        auto data = static_cast<const LiquidBlock*>(m_parentBlockType)->getBlockData(adjPos, m_id, clock);
        static_cast<LiquidBlockData*>(data.get())->m_currentLevel = m_currentLevel - levelDecline;
        static_cast<LiquidBlockData*>(data.get())->m_isSource = false;
        event.modifications.emplace_back(adjPos, m_id, std::move(data));
    }

    adjPos = m_position + constDirectionVectors3DHashed[
        static_cast<size_t>(Directions3DHashed::DIRECTION_LEFT)];
    if (DataRepository::airId == map.getBlockId(adjPos))
    {
        auto data = static_cast<const LiquidBlock*>(m_parentBlockType)->getBlockData(adjPos, m_id, clock);
        static_cast<LiquidBlockData*>(data.get())->m_currentLevel = m_currentLevel - levelDecline;
        static_cast<LiquidBlockData*>(data.get())->m_isSource = false;
        event.modifications.emplace_back(adjPos, m_id, std::move(data));
    }

    adjPos = m_position + constDirectionVectors3DHashed[
        static_cast<size_t>(Directions3DHashed::DIRECTION_RIGHT)];
    if (DataRepository::airId == map.getBlockId(adjPos))
    {
        auto data = static_cast<const LiquidBlock*>(m_parentBlockType)->getBlockData(adjPos, m_id, clock);
        static_cast<LiquidBlockData*>(data.get())->m_currentLevel = m_currentLevel - levelDecline;
        static_cast<LiquidBlockData*>(data.get())->m_isSource = false;
        event.modifications.emplace_back(adjPos, m_id, std::move(data));
    }

    adjPos = m_position + constDirectionVectors3DHashed[
        static_cast<size_t>(Directions3DHashed::DIRECTION_UP)];

    if (m_id == map.getBlockId(adjPos))
    {
        auto data = static_cast<const LiquidBlock*>(m_parentBlockType)->getBlockData(m_position, m_id, clock);
        static_cast<LiquidBlockData*>(data.get())->m_currentLevel = 16;
        static_cast<LiquidBlockData*>(data.get())->m_isSource = m_isSource;
        event.modifications.emplace_back(m_position, m_id, std::move(data));
    }

    return event;
}

std::vector<glm::ivec3> LiquidBlockData::getAvailableDirections(const MapUpdateInterface& map, glm::ivec3 blockCoord)
{
    //std::vector<glm::ivec3> result;
    //glm::ivec3 coord;

    //coord = blockCoord + constDirectionVectors3DHashed[
    //    static_cast<size_t>(Directions3DHashed::DIRECTION_DOWN)];
    //if (map.getBlockId(coord) == DataRepository::airId)
    //{
    //    result.push_back(coord);
    //    return result;
    //}
    //else
    //{
    //    for (int dir : directionsForFlowing)
    //    {
    //        coord = blockCoord + constDirectionVectors3DHashed[dir];
    //        if (map.getBlockId(coord) == DataRepository::airId)
    //            result.push_back(coord);
    //    }
    //}

    //return result;
    return std::vector<glm::ivec3>();
}

BlockModifiedBulkEvent LiquidBlockData::updateForSource(const MapUpdateInterface& map)
{
    //LiquidBlockPathFinderContext context([&map, this](const glm::ivec3& blockCoord) {
    //    return getAvailableDirections(map, blockCoord); });

    //Utils::PathFinder pathFinder(context);
    //auto paths = pathFinder.findAllNodesWhere(m_position, [this, &map](const glm::ivec3& coord) {
    //    if (map.getBlockId(coord + constDirectionVectors3DHashed[
    //        static_cast<size_t>(Directions3DHashed::DIRECTION_DOWN)]) 
    //        == DataRepository::airId)
    //        return true;
    //    return false;
    //}, directFlowDistanceThreshold);

    //size_t minCost = std::numeric_limits<size_t>::max();
    //std::vector<size_t> shortestPaths;

    //for (int i = 0; i < paths.size(); i++)
    //{
    //    if(paths[i].cost < minCost)
    //    {
    //        minCost = paths[i].cost;
    //        shortestPaths.clear();
    //        shortestPaths.push_back(i);
    //    }
    //    else if (paths[i].cost == minCost)
    //        shortestPaths.push_back(i);
    //}

    //if (shortestPaths.size() == 0)
    //{
    //    BlockModifiedBulkEvent updateData;
    //    uint32_t id = map.getBlockId(m_position);
    //    auto adjCoords = getAvailableDirections(map, m_position);
    //    for (auto& adj : adjCoords)
    //        updateData.modifications.push_back(BlockModifiedEvent{ adj, id, std::make_unique<LiquidBlockData>(
    //            m_position, id, false, m_currentLevel - levelDecline) });
    //    return updateData;
    //}
    //
    return BlockModifiedBulkEvent();
}

BlockModifiedBulkEvent LiquidBlockData::updateForFlowing(const MapUpdateInterface& map)
{
    return BlockModifiedBulkEvent();

}