#pragma once
#include "imgui.h"
#include "ChunkProcessingProfiler.h"

class GlobalParamsLogger
{
public:
	enum class LogParam
	{
		PARAM_CHUNK_LOAD_QUEUE_SIZE,
		PARAM_CHUNK_UNLOAD_QUEUE_SIZE,
		PARAM_CHUNK_MESH_QUEUE_SIZE,
		PARAM_CHUNK_UNMESH_QUEUE_SIZE,
		PARAM_CHUNK_POOL_ALLOCATED_SIZE,
		PARAM_CHUNK_POOL_FREE_SIZE,
		PARAM_CHUNK_MESH_POOL_ALLOCATED_SIZE,
		PARAM_CHUNK_MESH_POOL_FREE_SIZE,
		PARAM_NUMBER_OF_LOADED_CHUNKS,
		PARAM_NUMBER_OF_LOADED_MESHES,
		PARAM_NUMBER_OF_LOADED_BORDER_MESHES,
		PARAM_COLLISION_X,
		PARAM_COLLISION_Y,
		PARAM_COLLISION_Z,
	};

private:

	static std::atomic<unsigned int> chunkLoadQueueSize;
	static std::atomic<unsigned int> chunkUnloadQueueSize;

	static std::atomic<unsigned int> chunkMeshQueueSize;
	static std::atomic<unsigned int> chunkUnmeshQueueSize;

	static std::atomic<unsigned int> chunkPoolAllocatedSize;
	static std::atomic<unsigned int> chunkPoolFreeSize;

	static std::atomic<unsigned int> chunkMeshPoolAllocatedSize;
	static std::atomic<unsigned int> chunkMeshPoolFreeSize;

	static std::atomic<unsigned int> numberOfLoadedChunks;
	static std::atomic<unsigned int> numberOfLoadedMeshes;
	static std::atomic<unsigned int> numberOfLoadedBorderMeshes;

	static std::atomic<bool> collisionX;
	static std::atomic<bool> collisionY;
	static std::atomic<bool> collisionZ;
public:

	static void logParam(LogParam param, unsigned int value)
	{
		switch (param)
		{
		case LogParam::PARAM_CHUNK_LOAD_QUEUE_SIZE:
			chunkLoadQueueSize = value;
			break;
		case LogParam::PARAM_CHUNK_UNLOAD_QUEUE_SIZE:
			chunkUnloadQueueSize = value;
			break;
		case LogParam::PARAM_CHUNK_MESH_QUEUE_SIZE:
			chunkMeshQueueSize = value;
			break;
		case LogParam::PARAM_CHUNK_UNMESH_QUEUE_SIZE:
			chunkUnmeshQueueSize = value;
			break;
		case LogParam::PARAM_CHUNK_POOL_ALLOCATED_SIZE:
			chunkPoolAllocatedSize = value;
			break;
		case LogParam::PARAM_CHUNK_POOL_FREE_SIZE:
			chunkPoolFreeSize = value;
			break;
		case LogParam::PARAM_CHUNK_MESH_POOL_ALLOCATED_SIZE:
			chunkMeshPoolAllocatedSize = value;
			break;
		case LogParam::PARAM_CHUNK_MESH_POOL_FREE_SIZE:
			chunkMeshPoolFreeSize = value;
			break;
		case LogParam::PARAM_NUMBER_OF_LOADED_CHUNKS:
			numberOfLoadedChunks = value;
			break;
		case LogParam::PARAM_NUMBER_OF_LOADED_MESHES:
			numberOfLoadedMeshes = value;
			break;
		case LogParam::PARAM_NUMBER_OF_LOADED_BORDER_MESHES:
			numberOfLoadedBorderMeshes = value;
			break;
		case LogParam::PARAM_COLLISION_X:
			collisionX = value;
			break;
		case LogParam::PARAM_COLLISION_Y:
			collisionY = value;
			break;
		case LogParam::PARAM_COLLISION_Z:
			collisionZ = value;
			break;
		default:
			break;
		}
	};

	static unsigned int getParam(LogParam param)
	{
		switch (param)
		{
		case LogParam::PARAM_CHUNK_LOAD_QUEUE_SIZE:
			return chunkLoadQueueSize;
			break;
		case LogParam::PARAM_CHUNK_UNLOAD_QUEUE_SIZE:
			return chunkUnloadQueueSize;
			break;
		case LogParam::PARAM_CHUNK_MESH_QUEUE_SIZE:
			return chunkMeshQueueSize;
			break;
		case LogParam::PARAM_CHUNK_UNMESH_QUEUE_SIZE:
			return chunkUnmeshQueueSize;
			break;
		case LogParam::PARAM_CHUNK_POOL_ALLOCATED_SIZE:
			return chunkPoolAllocatedSize;
			break;
		case LogParam::PARAM_CHUNK_POOL_FREE_SIZE:
			return chunkPoolFreeSize;
			break;
		case LogParam::PARAM_CHUNK_MESH_POOL_ALLOCATED_SIZE:
			return chunkMeshPoolAllocatedSize;
			break;
		case LogParam::PARAM_CHUNK_MESH_POOL_FREE_SIZE:
			return chunkMeshPoolFreeSize;
			break;
		case LogParam::PARAM_NUMBER_OF_LOADED_CHUNKS:
			return numberOfLoadedChunks;
			break;
		case LogParam::PARAM_NUMBER_OF_LOADED_MESHES:
			return numberOfLoadedMeshes;
			break;
		case LogParam::PARAM_NUMBER_OF_LOADED_BORDER_MESHES:
			return numberOfLoadedBorderMeshes;
			break;
		case LogParam::PARAM_COLLISION_X:
			return collisionX;
			break;
		case LogParam::PARAM_COLLISION_Y:
			return collisionY;
			break;
		case LogParam::PARAM_COLLISION_Z:
			return collisionZ;
			break;
		default:
			break;
		}
	};

	static void displayDebugParams()
	{
		ImGui::Text("Global Parameters:");
		ImGui::Text("Number of loaded chunks: %u", numberOfLoadedChunks.load());
		ImGui::Text("Number of loaded meshes: %u", numberOfLoadedMeshes.load());
		ImGui::Text("Number of loaded border meshes: %u", numberOfLoadedBorderMeshes.load());
		ImGui::Text("Chunk load queue size: %u", chunkLoadQueueSize.load());
		ImGui::Text("Chunk unload queue size: %u", chunkUnloadQueueSize.load());
		ImGui::Text("Chunk mesh queue size: %u", chunkMeshQueueSize.load());
		ImGui::Text("Chunk unmesh queue size: %u", chunkUnmeshQueueSize.load());
		ImGui::Text("Chunk pool allocated size: %u", chunkPoolAllocatedSize.load());
		ImGui::Text("Chunk pool free size: %u", chunkPoolFreeSize.load());
		ImGui::Text("Chunk mesh pool allocated size: %u", chunkMeshPoolAllocatedSize.load());
		ImGui::Text("Chunk mesh pool free size: %u", chunkMeshPoolFreeSize.load());

		double totalTime = ChunkProcessingProfiler::getAverageTotalTime();

		ImGui::Text("Average total time: %u", totalTime);
		ImGui::Text("Average generation time: %u", ChunkProcessingProfiler::getAverageGenerationTime() / totalTime * 100.f, "%");
		ImGui::Text("Average generation-meshing pause time: %u", ChunkProcessingProfiler::getAverageGenerationMeshingPauseTime() / totalTime * 100.f, "%");
		ImGui::Text("Average meshing time: %u", ChunkProcessingProfiler::getAverageMeshingTime() / totalTime * 100.f, "%");
		ImGui::Text("Average meshing-buffering pause time: %u", ChunkProcessingProfiler::getAverageMeshingBufferingPauseTime() / totalTime * 100.f, "%");
		ImGui::Text("Average buffering time: %u", ChunkProcessingProfiler::getAverageBufferingTime() / totalTime * 100.f, "%");

		if (collisionX)
			ImGui::Text("Collision on X axis");
		if (collisionY)
			ImGui::Text("Collision on Y axis");
		if (collisionZ)
			ImGui::Text("Collision on Z axis");
	}
};