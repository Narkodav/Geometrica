#include "GlobalParamsLogger.h"


std::atomic<unsigned int> GlobalParamsLogger::chunkLoadQueueSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::chunkUnloadQueueSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::chunkMeshQueueSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::chunkUnmeshQueueSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::chunkPoolAllocatedSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::chunkPoolFreeSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::chunkMeshPoolAllocatedSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::chunkMeshPoolFreeSize = 0;
std::atomic<unsigned int> GlobalParamsLogger::numberOfLoadedChunks = 0;
std::atomic<unsigned int> GlobalParamsLogger::numberOfLoadedMeshes = 0;
std::atomic<unsigned int> GlobalParamsLogger::numberOfLoadedBorderMeshes = 0;
std::atomic<bool> GlobalParamsLogger::collisionX = 0;
std::atomic<bool> GlobalParamsLogger::collisionY = 0;
std::atomic<bool> GlobalParamsLogger::collisionZ = 0;