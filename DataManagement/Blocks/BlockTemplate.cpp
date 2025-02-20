#include "BlockTemplate.h"
#include "dataManagement/DataRepository.h"

std::string BlockTemplate::parseName(std::string filepath)
{
	int dot = filepath.size() - 1;
	int slash;

	for (slash = filepath.size() - 1; slash > -1; slash--)
		if (filepath[slash] == '.')
		{
			dot = slash;
			slash--;
			break;
		}

	for (slash; slash > -1; slash--)
		if (filepath[slash] == '/')
			break;

	filepath = filepath.substr(slash + 1, dot - slash - 1);
	return filepath;
}

std::string BlockTemplate::parseBlockType(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return "error";
    }

    std::string line;
    std::string type;

    while (std::getline(file, line)) {
        // Skip empty lines and comments that don't start with #
        if (line.empty() || line[0] != '#') continue;
        else if (line[0] == '#') {
            std::istringstream iss(line.substr(1)); // Remove the #
            std::string tag;
            iss >> tag;

            if (tag == "Block") {
                iss >> tag >> type; // Read "type" and the type value
                return type;
            }
        }
    }
}

bool BlockTemplate::loadHitboxData(const std::string& hitboxName) {
    setHitboxName(hitboxName);
    setHitbox(&DataRepository::getHitbox(hitboxName));
    return true;
}