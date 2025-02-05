#include "Item.h"
#include "DataRepository.h"
//item file structure:
//#id --id--
//#cuboid --cuboid name-- (points to a cuboid) //optional
//--material name-- //six times for each cuboid face
//#model --model name-- (points to a model) //optional
//#material --mesh name--/--material name-- (points to materials for meshes, can be multiple per item) used for the model
//#hitbox --hitbox name-- (.hbx file) //optional
//#flags --flags-- (a series of ones and zeros)
//		flags: (in order)
//		is placeable
//		is block
//		is transparent
//		is solid
//		has gravity
//		is liquid
//		is breakable
//		needs support

std::string Item::getName(std::string filepath)
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

void Item::load(std::string itemPath)
{
	std::fstream itemFile(itemPath, std::ios::in);
#ifdef _DEBUG
	if (!itemFile.is_open())
		__debugbreak();
#endif //_DEBUG
	std::string line;
	std::string buffer;
	this->name = getName(itemPath);
	this->displayModelName = "NONE";
	this->cuboidModelName = "NONE";
	this->displayModelHandle = nullptr;
	this->cuboidModelHandle = nullptr;

	while (std::getline(itemFile, line))
	{
		if (line.find("#id") != std::string::npos)
		{
			//id
			line.erase(line.begin(), line.begin() + 4);
			this->id = std::stoi(line);
		}
		else if (line.find("#cuboid") != std::string::npos)
		{
			//cuboid
			line.erase(line.begin(), line.begin() + 8);
			this->cuboidModelName = line;
			cuboidModelHandle = &DataRepository::getCuboid(this->cuboidModelName);

			this->id |= DataRepository::hasCuboidModelIdMask;
			for (int i = 0; i < 6; i++)
			{
				std::getline(itemFile, line);
				cuboidMeshMaterials.atlasIndices[i] = DataRepository::getAtlas().getStorageBufferIndex(line);
			}
		}
		else if (line.find("#model") != std::string::npos)
		{
			//model
			line.erase(line.begin(), line.begin() + 7);
			this->displayModelName = line;
			displayModelHandle = &DataRepository::getModel(this->displayModelName);

			this->id |= DataRepository::hasDisplayModelIdMask;
			modelMeshMaterials.textureNames = displayModelHandle->getMeshNameVector();
			modelMeshMaterials.atlasIndices.resize(modelMeshMaterials.textureNames.size());
		}
		else if (line.find("#material") != std::string::npos)
		{
			//meshes
			line.erase(line.begin(), line.begin() + 10);
			int i = 0;
			while (line[i] != '/')
			{
				buffer += line[i];
				i++;
			}
			line.erase(line.begin(), line.begin() + i + 1);

			this->meshMaterialsMap[buffer] = modelMeshMaterials.atlasIndices.size();

			modelMeshMaterials.atlasIndices[std::find(modelMeshMaterials.textureNames.begin(),
				modelMeshMaterials.textureNames.end(), buffer) - modelMeshMaterials.textureNames.begin()] =
				DataRepository::getAtlas().getStorageBufferIndex(line);

			buffer.clear();
		}
		else if (line.find("#hitbox") != std::string::npos)
		{
			//hitbox
			line.erase(line.begin(), line.begin() + 8);
			this->hitboxHandle = DataRepository::getHitbox(line);
		}
		else if (line.find("#flags") != std::string::npos)
		{
			//flags
			for (int i = 0; i < 8; i++)
				this->properties.flags |= (line[8 + i] - '0') << i;
		}
	}
	DataRepository::m_itemNameMap[this->name] = this->id;
	itemFile.close();
}

