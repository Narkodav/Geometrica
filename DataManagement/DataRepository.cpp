#include "DataRepository.h"

void DataRepository::readPaths(std::vector<std::string>& container, const std::string& path)
{
	std::fstream file(path, std::ios::in);
	if (!file.is_open())
		__debugbreak();
	std::string itemName;

	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	unsigned int estimatedCount = fileSize / 10;
	container.reserve(estimatedCount);

	while (std::getline(file, itemName))
		container.push_back(itemName);

	container.shrink_to_fit();
	file.close();
}

void DataRepository::setItems()
{
	std::vector<std::string> itemPaths;
	readPaths(itemPaths, m_resourcesFilepath + "/items/itemNames.txt");

	std::ostringstream path;
	Item buffer;

	for (int i = 0; i < itemPaths.size(); i++)
	{
		path << m_resourcesFilepath << "/items/" << itemPaths[i] << ".idata";
		buffer.load(path.str());
		m_items[buffer.id] = buffer;
		path.str("");
		path.clear();
	}
}

void DataRepository::setModels()
{
	std::vector<std::string> modelPaths;
	readPaths(modelPaths, m_resourcesFilepath + "/models/modelNames.txt");

	std::ostringstream path;

	for (int i = 0; i < modelPaths.size(); i++)
	{
		path << m_resourcesFilepath << "/models/" << modelPaths[i] << ".obj";
		m_models[modelPaths[i]].load(path.str());
		path.str("");
		path.clear();
	}
	m_models["NONE"] = Model();
}

void DataRepository::setCuboids()
{
	std::vector<std::string> cuboidPaths;
	readPaths(cuboidPaths, m_resourcesFilepath + "/cuboids/cuboidNames.txt");

	std::ostringstream path;

	for (int i = 0; i < cuboidPaths.size(); i++)
	{
		path << m_resourcesFilepath << "/cuboids/" << cuboidPaths[i] << ".cuboid";
		m_cuboids[cuboidPaths[i]].load(path.str());
		path.str("");
		path.clear();
	}
	m_cuboids["NONE"] = Cuboid();

};

void DataRepository::setTextures()
{
	std::vector<std::string> texturePaths;
	readPaths(texturePaths, m_resourcesFilepath + "/textures/textureNames.txt");

	std::ostringstream path;
	int i = 0;
	for (int i = 0; i < texturePaths.size(); i++)
	{
		path << m_resourcesFilepath << "/textures/" << texturePaths[i] << ".png";
		m_textures[texturePaths[i]].load(path.str());
		texturePaths[i] = path.str();
		path.str("");
		path.clear();
	}

	m_itemTexturesAtlas.set(texturePaths);
	m_itemTexturesAtlas.save(m_resourcesFilepath + "/textureAtlas.png");
	m_itemTexturesAtlas.makeStorageBuffer();
}

void DataRepository::setHitboxes()
{
	std::vector<std::string> hitboxPaths;
	readPaths(hitboxPaths, m_resourcesFilepath + "/hitboxes/hitboxNames.txt");

	std::ostringstream path;
	int i = 0;
	for (int i = 0; i < hitboxPaths.size(); i++)
	{
		path << m_resourcesFilepath << "/hitboxes/" << hitboxPaths[i] << ".hbx";
		m_hitboxes[hitboxPaths[i]] = std::move(Phys::HitboxFactory::loadHitbox(path.str()));
		path.str("");
		path.clear();
	}
}

void DataRepository::set(std::string resourcesFilepath)
{
	m_resourcesFilepath = resourcesFilepath;
#ifdef _DEBUG
	std::cout << m_resourcesFilepath << std::endl;
#endif //_DEBUG
	setTextures();
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "textures:" << std::endl;
	for (const auto& texture : m_textures)
	{
		std::cout << "id:   " << texture.second.GetId() << std::endl;
		std::cout << "name: " << texture.first << std::endl;
	}
#endif //_DEBUG
	setModels();
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "models:" << std::endl;
	for (const auto& model : m_models)
	{
		std::cout << "name:               " << model.first << std::endl;
		std::cout << "number of meshes:   " << model.second.getNumOfMeshes() << std::endl;
	}
#endif //_DEBUG
	setCuboids();
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "cuboids:" << std::endl;
	for (const auto& cuboid : m_cuboids)
	{
		std::cout << "name:               " << cuboid.first << std::endl;
	}
#endif //_DEBUG
	setHitboxes();
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "hitboxes:" << std::endl;
	for (const auto& hitbox : m_hitboxes)
	{
		std::cout << "name: " << hitbox.first << std::endl;
	}
#endif //_DEBUG
	setItems();
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "items:" << std::endl;
	for (const auto& item : m_items)
	{
		std::cout << "id:   " << item.first << std::endl;
		std::cout << "name: " << item.second.name << std::endl;
	}
#endif //_DEBUG
}