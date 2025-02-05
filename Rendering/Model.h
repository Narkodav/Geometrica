#pragma once
//std
#include <string>
#include <fstream>
#include <vector>

//native
#include "Rendering/Texture.h"
#include "Rendering/Shader.h"

//vendor
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"

enum class bufferTypesPerModel : unsigned int
{
	BUFFER_POSITION = 0,
	BUFFER_UV,
	BUFFER_NORMALS,
	BUFFER_INDEX,
	BUFFER_NUM,
};

enum locations
{
	LOCATION_POSITION = 0,
	LOCATION_UV = 1,
	LOCATION_NORMAL = 2,
	LOCATION_TRANSPOSITION = 3,
	LOCATION_INDEX = 4,
};

//data for drawing
struct drawToolKit
{
	unsigned int PositionsBufferId;
	unsigned int UVBufferId;
	unsigned int NormalsBufferId;
	unsigned int elementBufferId;

	unsigned int numOfIndices;
	unsigned int indexOffset;
	unsigned int vertexOffset;
};

struct Mesh
{
	std::string name;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textureCoords;
	std::vector<glm::vec3> normals;
	std::vector<glm::ivec3> faces;
	unsigned int numOfIndices;
	unsigned int indexOffset;
	unsigned int vertexOffset;

	Mesh() = default;
	Mesh(const Mesh& other) = default;
	Mesh& operator=(const Mesh& other) = default;
	~Mesh() = default;

	Mesh(Mesh&& other) noexcept = default;
	Mesh& operator=(Mesh&& other) noexcept = default;
};

class Model
{
private:
	unsigned int m_numOfVertices;
	unsigned int m_numOfFaces;
	std::vector<Mesh> m_meshes;
	std::unordered_map<std::string, Mesh*> m_meshesByName;
	std::string m_name;

	unsigned int m_vertexArrayId;
	unsigned int m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_NUM];

	//temporary for buffer loading
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec2> m_textureCoords;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::ivec3> m_faces;
public:
	Model();
	Model(std::string filepath);
	Model(const Model& model);
	~Model();
	Model& operator=(const Model& model) noexcept;

	void load(std::string filepath);
	void clear();
	void copy(const Model& model);

	//getters
	const Mesh& getMesh(unsigned int meshIndex) const { return m_meshes[meshIndex]; };
	const Mesh& getMeshByName(const std::string& name) const { return *m_meshesByName.find(name)->second; };
	unsigned int getNumOfMeshes() const { return m_meshes.size(); };
	unsigned int getNumOfVertices() const { return m_numOfVertices; };
	unsigned int getNumOfFaces() const { return m_numOfFaces; };
	std::vector<std::string> getMeshNameVector() const
	{
		std::vector<std::string> result;
		for (auto& mesh : m_meshes)
			result.push_back(mesh.name);
		return result;
	};


	drawToolKit getDrawToolKit(unsigned int meshIndex) const {
		return {
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_POSITION],
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_UV],
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_NORMALS],
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_INDEX],
m_meshes[meshIndex].numOfIndices,
m_meshes[meshIndex].indexOffset,
m_meshes[meshIndex].vertexOffset };
	};

	drawToolKit getDrawToolKitByName(const std::string& name) const {
		return {
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_POSITION],
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_UV],
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_NORMALS],
m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_INDEX],
m_meshesByName.find(name)->second->numOfIndices,
m_meshesByName.find(name)->second->indexOffset,
m_meshesByName.find(name)->second->vertexOffset };
	};

	
private:
	std::string getName(std::string filepath);
	void extractData(const aiScene* scene);
	void populateMesh(const aiMesh* meshToExtract, Mesh& meshToPopulate);
	void loadBuffers();
};