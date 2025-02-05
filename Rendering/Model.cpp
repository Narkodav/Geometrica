#include "Model.h"

Model::Model()
{
	unsigned int m_numOfVertices = 0;
	unsigned int m_numOfFaces = 0;
}

Model::Model(std::string filepath)
{
	this->load(filepath);
}

Model::Model(const Model& model)
{
	copy(model);
}

Model::~Model()
{
	if (m_name.size())
		clear();

}

Model& Model::operator=(const Model& model) noexcept
{
	if (this != &model)
	{
		clear();
		copy(model);
	}
	return *this;
}

void Model::clear()
{
	if (m_vertexArrayId)
	{
		glDeleteBuffers((unsigned int)bufferTypesPerModel::BUFFER_NUM, m_buffers);
		glDeleteVertexArrays(1, &m_vertexArrayId);
	}

	unsigned int m_numOfVertices = 0;
	unsigned int m_numOfFaces = 0;
	if (!m_name.size())
		return;
	m_name.clear();
	m_meshes.clear();
}

void Model::copy(const Model& model)
{
	this->m_meshes = model.m_meshes;
}

void Model::load(std::string filepath)
{
	clear();
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filepath, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals);

#ifdef _DEBUG
	if (!pScene)
		__debugbreak();
#endif
	m_name = getName(filepath);
	m_meshes.resize(pScene->mNumMeshes);
	extractData(pScene);
	importer.FreeScene();
	loadBuffers();
}

std::string Model::getName(std::string filepath)
{
	int dot = 0;
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

//void Model::getId(const std::string& filepath)
//{
//	std::fstream file(filepath, std::ios::in);
//	std::string line;
//	do
//	{
//		std::getline(file, line);
//	} while (line.find("#id") == std::string::npos);
//	file.close();
//	int i = 4;
//	while (line[i] >= '0' && line[i] <= '9');
//	{
//		m_id *= 10;
//		m_id += line[i] - '0';
//		i++;
//	}
//}

void Model::extractData(const aiScene* scene)
{
	m_numOfVertices = 0;
	m_numOfFaces = 0;
	for (unsigned int i = 0; i < m_meshes.size(); i++)
	{
		populateMesh(scene->mMeshes[i], m_meshes[i]);
		m_meshesByName[m_meshes[i].name] = &m_meshes[i];
	}
}

void Model::populateMesh(const aiMesh* meshToExtract, Mesh& meshToPopulate) //no need to optimize yet, loaded only once
{
	meshToPopulate.numOfIndices = meshToExtract->mNumFaces * 3;
	meshToPopulate.indexOffset = m_numOfFaces * 3;
	meshToPopulate.vertexOffset = m_numOfVertices;
	meshToPopulate.name = meshToExtract->mName.C_Str();
	meshToPopulate.positions.resize(meshToExtract->mNumVertices);
	meshToPopulate.normals.resize(meshToExtract->mNumVertices);
	meshToPopulate.faces.resize(meshToExtract->mNumFaces);
	m_numOfVertices += meshToPopulate.positions.size();
	m_numOfFaces += meshToPopulate.faces.size();

	if (meshToExtract->HasTextureCoords(0))
	{
		meshToPopulate.textureCoords.resize(meshToExtract->mNumVertices);
		for (int i = 0; i < meshToExtract->mNumVertices; i++)
		{
			meshToPopulate.positions[i].x = meshToExtract->mVertices[i].x;
			meshToPopulate.positions[i].y = meshToExtract->mVertices[i].y;
			meshToPopulate.positions[i].z = meshToExtract->mVertices[i].z;

			meshToPopulate.normals[i].x = meshToExtract->mNormals[i].x;
			meshToPopulate.normals[i].y = meshToExtract->mNormals[i].y;
			meshToPopulate.normals[i].z = meshToExtract->mNormals[i].z;

			meshToPopulate.textureCoords[i].x = meshToExtract->mTextureCoords[0][i].x;
			meshToPopulate.textureCoords[i].y = meshToExtract->mTextureCoords[0][i].y;
		}
	}
	else
	{
		for (int i = 0; i < meshToExtract->mNumVertices; i++)
		{
			meshToPopulate.positions[i].x = meshToExtract->mVertices[i].x;
			meshToPopulate.positions[i].y = meshToExtract->mVertices[i].y;
			meshToPopulate.positions[i].z = meshToExtract->mVertices[i].z;

			meshToPopulate.normals[i].x = meshToExtract->mNormals[i].x;
			meshToPopulate.normals[i].y = meshToExtract->mNormals[i].y;
			meshToPopulate.normals[i].z = meshToExtract->mNormals[i].z;
		}
	}

	for (int i = 0; i < meshToExtract->mNumFaces; i++)
	{
		meshToPopulate.faces[i].x = meshToExtract->mFaces[i].mIndices[0];
		meshToPopulate.faces[i].y = meshToExtract->mFaces[i].mIndices[1];
		meshToPopulate.faces[i].z = meshToExtract->mFaces[i].mIndices[2];
	}

	m_positions.insert(m_positions.end(), meshToPopulate.positions.begin(), meshToPopulate.positions.end());
	m_textureCoords.insert(m_textureCoords.end(), meshToPopulate.textureCoords.begin(), meshToPopulate.textureCoords.end());
	m_normals.insert(m_normals.end(), meshToPopulate.normals.begin(), meshToPopulate.normals.end());
	m_faces.insert(m_faces.end(), meshToPopulate.faces.begin(), meshToPopulate.faces.end());
}

void Model::loadBuffers()
{
	glGenVertexArrays(1, &m_vertexArrayId);
	glGenBuffers((unsigned int)bufferTypesPerModel::BUFFER_NUM, m_buffers);

	glBindVertexArray(m_vertexArrayId);

	//populating position buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_POSITION]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_positions[0]) * m_positions.size(), m_positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(LOCATION_POSITION);
	glVertexAttribPointer(LOCATION_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//populating texture coordinate buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_UV]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_textureCoords[0]) * m_textureCoords.size(), m_textureCoords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(LOCATION_UV);
	glVertexAttribPointer(LOCATION_UV, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//populating normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_NORMALS]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_normals[0]) * m_normals.size(), m_normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(LOCATION_NORMAL);
	glVertexAttribPointer(LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//populating indices buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[(unsigned int)bufferTypesPerModel::BUFFER_INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_faces[0]) * m_faces.size(), m_faces.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	m_positions.clear();
	m_textureCoords.clear();
	m_normals.clear();
	m_faces.clear();
}




