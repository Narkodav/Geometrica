#include "ChunkProcessingProfiler.h"

Utils::SampleTracker<double> ChunkProcessingProfiler::generationTimes{ "GenerationTimes", 100 };
Utils::SampleTracker<double> ChunkProcessingProfiler::generationMeshingPauseTimes{ "GenerationMeshingPauseTimes", 100 };
Utils::SampleTracker<double> ChunkProcessingProfiler::meshingTimes{ "MeshingTimes", 100 };
Utils::SampleTracker<double> ChunkProcessingProfiler::meshingBufferingPauseTimes{ "MeshingBufferingPauseTimes", 100 };
Utils::SampleTracker<double> ChunkProcessingProfiler::bufferingTimes{ "BufferingTimes", 100 };

std::unordered_map<glm::ivec2, ChunkProcessingProfiler::ChunkStats> ChunkProcessingProfiler::chunkStats;

unsigned int ChunkProcessingProfiler::chunkStatsBufferSize = 400;