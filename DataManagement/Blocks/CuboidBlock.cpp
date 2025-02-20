#include "CuboidBlock.h"
#include "dataManagement/DataRepository.h"

//Cuboid block file structure:
//#Block type --type-- //type matches class name
//#id --id--
//#cuboid --cuboid name-- (points to a cuboid)
//--material name-- //six times for each cuboid face
//#hitbox --hitbox name-- (.hbx file) //optional

bool CuboidBlock::loadFromFile(const std::string & filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }
    setName(parseName(filepath));

    std::string line;
    std::string type, id, cuboidName, hitboxName;
    std::vector<std::string> materials;

    while (std::getline(file, line)) {
        // Skip empty lines and comments that don't start with #
        if (line.empty() || (line[0] != '#' && materials.size() >= 6)) continue;

        if (line[0] == '#') {
            std::istringstream iss(line.substr(1)); // Remove the #
            std::string tag;
            iss >> tag;

            if (tag == "Block") {
                iss >> tag >> type; // Read "type" and the type value
                if (type != "CuboidBlock") {
                    std::cerr << "Invalid block type in file: " << type << std::endl;
                    return false;
                }
            }
            else if (tag == "id") {
                iss >> id;
                setId(std::stoul(id));
            }
            else if (tag == "cuboid") {
                iss >> cuboidName;
                // Load cuboid data from cuboid name
                if (!loadCuboidData(cuboidName)) {
                    std::cerr << "Failed to load cuboid: " << cuboidName << std::endl;
                    return false;
                }
            }
            else if (tag == "hitbox") {
                iss >> hitboxName;
                // Load hitbox data from .hbx file
                if (!loadHitboxData(hitboxName)) {
                    std::cerr << "Failed to load hitbox: " << hitboxName << std::endl;
                    return false;
                }
            }
        }
        else {
            // Must be a material name
            if (materials.size() < 6) {
                std::string materialName = line;
                // Trim whitespace
                materialName.erase(0, materialName.find_first_not_of(" \t\r\n"));
                materialName.erase(materialName.find_last_not_of(" \t\r\n") + 1);
                materials.push_back(materialName);
            }
        }
    }

    // Verify we got all 6 materials
    if (materials.size() != 6) {
        std::cerr << "Invalid number of materials: " << materials.size()
            << " (expected 6)" << std::endl;
        return false;
    }

    // Set materials for each face
    for (int i = 0; i < 6; ++i) {
        if (!setFaceMaterial(i, materials[i])) {
            std::cerr << "Failed to set material " << materials[i]
                << " for face " << i << std::endl;
            return false;
        }
    }

    file.close();
    return true;
}

// Helper functions that would need to be implemented:
bool CuboidBlock::loadCuboidData(const std::string& cuboidName) {
    m_cuboidName = cuboidName;
    m_cuboidHandle = &DataRepository::getCuboid(m_cuboidName);
    return true;
}

bool CuboidBlock::setFaceMaterial(int faceIndex, const std::string& materialName) {
    m_cuboidMeshMaterials.atlasIndices[faceIndex] =
        DataRepository::getAtlas().getStorageBufferIndex(materialName);

    return true;
}