#include "WorldMesh.h"

WorldMesh::WorldMesh(int renderAreaRadius, GameContext gameContext, const ChunkMap& chunkMap) :
	m_eventSystem(gameContext.gameEvents),
	m_taskMeshCoordinator(gameContext.threadPool,
		gameContext.threadPool.getWorkerAmount() / 4),
	m_taskUnmeshCoordinator(gameContext.threadPool,
		gameContext.threadPool.getWorkerAmount() / 4),
	m_chunkMap(chunkMap), m_renderAreaRadius(renderAreaRadius),
	m_blockUpdateSubscription(gameContext.gameEvents.subscribe<GameEventTypes::BLOCK_REMESH>
		([this](BlockRemeshEvent data)
			{ blockModified(data); }))
{
	m_chunkMeshPool.set(pow(m_renderAreaRadius * 2 + 1, 2) * 2);
	m_renderArea = Utils::getSortedCircleCoords(m_renderAreaRadius, 0.5);
	m_renderAreaRadiusWithPaddingSquared = pow(m_renderAreaRadius + 0.5, 2);
}

void WorldMesh::set()
{
	m_skybox.set({ "res/resourcePack/skybox/posx.png",
	"res/resourcePack/skybox/negx.png",
	"res/resourcePack/skybox/posy.png",
	"res/resourcePack/skybox/negy.png",
	"res/resourcePack/skybox/posz.png",
	"res/resourcePack/skybox/negz.png" });
	m_blockHighlight.emplace(DataRepository::getAtlas().getStorageBufferIndex("highlight_texture"));
}
void WorldMesh::drawChunks(Shader& shaderCuboid, Shader& shaderBlock, glm::mat4 view, glm::mat4 projection)
{
	shaderCuboid.Bind();
	glUniformMatrix4fv(shaderCuboid.GetUniformLocation("u_viewTransform"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(shaderCuboid.GetUniformLocation("u_projectionTransform"), 1, GL_FALSE, &projection[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DataRepository::getAtlas().getId());
	glUniform1i(shaderCuboid.GetUniformLocation("u_Texture"), 0);
	DataRepository::getAtlas().bindStorageBuffer();

	auto access = m_meshData.getReadAccess();

	for (auto& chunkMesh : access->meshes)
	{
		auto chunkAccess = chunkMesh.second->getReadAccess();
		if (chunkAccess->getIsSet())
		{
			chunkAccess->draw(BlockMesherType::MESHING_CUBOID);
		}
	}
	//DataRepository::getCuboid("slabHorizontalDown").drawTest({ glm::vec3(0), glm::vec3(0.0f, 0.0f, -1.0f) }, { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2 });
	//DataRepository::getCuboid("stairsBaseDownXPos").drawTest({ glm::vec3(0.0f, 0.0f, 1.0f) }, { 0, 1, 2, 0, 1, 2 });
	//DataRepository::getCuboid("slabHorizontalDown").drawTest({ glm::vec3(0.0f, 0.0f, 2.0f) }, { 0, 1, 2, 0, 1, 2 });


	//DataRepository::getCuboid("cube").drawTest({ glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(2.0f, 0.0f, -1.0f) }, { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2 });

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void WorldMesh::drawSkybox(Shader& shader, glm::mat4 viewWithoutTranspos, glm::mat4 projection)
{
	m_skybox.draw(shader, viewWithoutTranspos, projection);
}

void WorldMesh::drawBlockHighlight(Shader& shader, glm::mat4 view, glm::mat4 projection, Area selectedArea)
{
	m_blockHighlight.value().draw(shader, selectedArea, view, projection);
}

void WorldMesh::iterate(glm::ivec2 newCoords)
{
	if (!(m_taskMeshCoordinator.canAddTask() || m_taskUnmeshCoordinator.canAddTask()))
		return;
	auto accessChunkData = m_chunkMap.getMapDataAccess();
	auto accessMeshData = m_meshData.getWriteAccess();
	
	for (auto& coord : m_renderArea)
	{
		auto globalCoord = coord + newCoords;
		auto chunkState = accessChunkData->states.find(globalCoord);
		auto meshState = accessMeshData->states.find(globalCoord);

		if (chunkState != accessChunkData->states.end() &&
			chunkState->second == ChunkMap::ChunkState::GENERATED &&
			meshState == accessMeshData->states.end())
		{
			auto& region = accessChunkData->regions.find(globalCoord)->second;
			if (region.getAdjAmount() == 4)
			{
				if (!m_taskMeshCoordinator.tryAddTask([this, data = region.getRegionData()] {
					meshChunk(data); }))
					break;
				accessMeshData->states.emplace(globalCoord, MeshState::BEING_MESHED);
			}
		}
	}

	for (auto& mesh : accessMeshData->states)
	{
		glm::vec2 localCoords = mesh.first - newCoords;

		if (mesh.second == MeshState::MESHED &&
			glm::dot(localCoords, localCoords) > m_renderAreaRadiusWithPaddingSquared)
		{
			if (!m_taskUnmeshCoordinator.tryAddTask([this, coord = mesh.first] {
				unmeshChunk(coord); }))
				break;
			mesh.second = MeshState::BEING_UNMESHED;
		}
	}
}

void WorldMesh::meshChunk(ChunkRegionData chunkData)
{
	MT::MemoryPool<MT::Synchronized<ChunkMesh>>::SharedPointer mesh = m_chunkMeshPool.makeShared();
	glm::ivec2 coords;
	{
		auto access = mesh->getWriteAccess();
		access->set(chunkData);
		coords = access->getCoords();
	}	

	auto access = m_meshData.getWriteAccess();
	if (access->meshes.find(coords) != access->meshes.end())
		__debugbreak();
	access->meshes.emplace(coords, mesh);
	access->states.find(coords)->second = MeshState::MARKED_FOR_BUFFER_LOAD;
	m_bufferLoadQueue.getWriteAccess()->push({ coords, mesh });
	//m_numOfMeshes++;
	//GlobalParamsLogger::logParam(GlobalParamsLogger::LogParam::PARAM_CHUNK_MESH_QUEUE_SIZE, m_bufferLoadQueue.size());
	//GlobalParamsLogger::logParam(GlobalParamsLogger::LogParam::PARAM_CHUNK_MESH_POOL_ALLOCATED_SIZE, m_chunkMeshPool.getAllocatedSize());
	//GlobalParamsLogger::logParam(GlobalParamsLogger::LogParam::PARAM_CHUNK_MESH_POOL_FREE_SIZE, m_chunkMeshPool.getFreeSize());
}

void WorldMesh::unmeshChunk(glm::ivec2 coords)
{
	MT::MemoryPool<MT::Synchronized<ChunkMesh>>::SharedPointer mesh;
	{
		auto access = m_meshData.getWriteAccess();
		auto it = access->meshes.find(coords);
		if (it == access->meshes.end())
			__debugbreak();
		mesh = it->second;
		access->meshes.erase(it);
		access->states.find(coords)->second = MeshState::MARKED_FOR_BUFFER_UNLOAD;
	}
	m_bufferUnloadQueue.getWriteAccess()->push({ mesh->getReadAccess()->getCoords(), mesh });
	//m_numOfMeshes--;
}

void WorldMesh::processBuffers()
{
	auto meshDataAccess = m_meshData.getWriteAccess();
	{
		auto access = m_bufferLoadQueue.getWriteAccess();
		if (!access->empty())
		{
			auto elem = access->top();
			access->pop();
			auto meshAccess = elem.meshPtr->getWriteAccess();
			meshAccess->loadBuffers();
			meshDataAccess->states.find(elem.coord)->second = MeshState::MESHED;
		}
	}
	{
		auto access = m_bufferUnloadQueue.getWriteAccess();
		if (!access->empty())
		{
			auto elem = access->top();
			access->pop();
			auto meshAccess = elem.meshPtr->getWriteAccess();
			meshAccess->deleteBuffers();
			meshDataAccess->states.erase(elem.coord);
		}
	}
}

void WorldMesh::blockModified(BlockRemeshEvent data)
{
	glm::ivec2 chunkCoords = Utils::tileToChunkCoord(data.blockCoord, constChunkSize);
	glm::ivec3 localCoords = Utils::globalToLocal(data.blockCoord, constChunkSize);
	auto access = m_meshData.getWriteAccess();
	auto chunk = access->meshes.find(chunkCoords);
	if (chunk != access->meshes.end())
	{
		data.blockCoord = localCoords;
		chunk->second->getWriteAccess()->notifyBlockUpdate(data);
		m_bufferLoadQueue.getWriteAccess()->push({ chunkCoords, chunk->second }); //updates are applied at buffer load
		
		//if (localCoords.x == 0)
		//{
		//	auto chunkAdj = access->meshes.find(glm::ivec2(chunkCoords.x - 1, chunkCoords.y));
		//	if (chunkAdj != access->meshes.end())
		//	{
		//		data.blockCoord.x = constChunkSize;
		//		chunkAdj->second->getWriteAccess()->notifyBlockUpdate(data);
		//		m_bufferLoadQueue.getWriteAccess()->push(
		//			{ glm::ivec2(chunkCoords.x - 1, chunkCoords.y), chunkAdj->second });
		//	}
		//}
		//else if (localCoords.x == constChunkSize - 1)
		//{
		//	auto chunkAdj = access->meshes.find(glm::ivec2(chunkCoords.x + 1, chunkCoords.y));
		//	if (chunkAdj != access->meshes.end())
		//	{
		//		data.blockCoord.x = -1;
		//		chunkAdj->second->getWriteAccess()->notifyBlockUpdate(data);
		//		m_bufferLoadQueue.getWriteAccess()->push(
		//			{ glm::ivec2(chunkCoords.x + 1, chunkCoords.y), chunkAdj->second });
		//	}
		//}
		//data.blockCoord = localCoords;
		//if (localCoords.z == 0)
		//{
		//	auto chunkAdj = access->meshes.find(glm::ivec2(chunkCoords.x, chunkCoords.y - 1));
		//	if (chunkAdj != access->meshes.end())
		//	{
		//		data.blockCoord.z = constChunkSize;
		//		chunkAdj->second->getWriteAccess()->notifyBlockUpdate(data);
		//		m_bufferLoadQueue.getWriteAccess()->push(
		//			{ glm::ivec2(chunkCoords.x, chunkCoords.y - 1), chunkAdj->second });
		//	}
		//}
		//else if (localCoords.z == constChunkSize - 1)
		//{
		//	auto chunkAdj = access->meshes.find(glm::ivec2(chunkCoords.x, chunkCoords.y + 1));
		//	if (chunkAdj != access->meshes.end())
		//	{
		//		data.blockCoord.z = -1;
		//		chunkAdj->second->getWriteAccess()->notifyBlockUpdate(data);
		//		m_bufferLoadQueue.getWriteAccess()->push(
		//			{ glm::ivec2(chunkCoords.x, chunkCoords.y + 1), chunkAdj->second });
		//	}
		//}
	}
}