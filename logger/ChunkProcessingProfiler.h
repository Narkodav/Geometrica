#pragma once
#include <string>
#include <unordered_map>
#include <chrono>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/hash.hpp"

#include "Utilities/SampleTracker.h"

class ChunkProcessingProfiler
{
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double>;

    struct ChunkStats {
        TimePoint generationStart;
        Duration generationTime;
        TimePoint meshingStart;
        Duration meshingTime;
        TimePoint bufferLoadStart;
        Duration bufferLoadTime;
        bool completed = 0;
    };

    static unsigned int chunkStatsBufferSize;

    static Utils::SampleTracker<double> generationTimes;
    static Utils::SampleTracker<double> generationMeshingPauseTimes;
    static Utils::SampleTracker<double> meshingTimes;
    static Utils::SampleTracker<double> meshingBufferingPauseTimes;
    static Utils::SampleTracker<double> bufferingTimes;

    static std::unordered_map<glm::ivec2, ChunkStats> chunkStats;

    static TimePoint getCurrentTimePoint() {
        return Clock::now();
    }

    static Duration getDurationInSeconds(TimePoint start, TimePoint end) {
        return std::chrono::duration_cast<Duration>(end - start);
    }

    static void cutBufferOverflow()
    {
        if (chunkStats.size() <= chunkStatsBufferSize)
            return;

        for (auto it = chunkStats.begin(); it != chunkStats.end(); ++it)
        {
            if (it->second.completed)
            {
                chunkStats.erase(it->first);
                return;
            }
        }
    }

public:

    static void startChunkGeneration(glm::ivec2 chunkPos) {
        chunkStats[chunkPos].generationStart = getCurrentTimePoint();
    }

    static void finishChunkGeneration(glm::ivec2 chunkPos) {
        auto chunk = chunkStats.find(chunkPos);
        if(chunk != chunkStats.end()) {
            chunk->second.generationTime = getCurrentTimePoint() - chunk->second.generationStart;
            generationTimes.addSample(chunk->second.generationTime.count());
        }
    }

    static void startChunkMeshing(glm::ivec2 chunkPos) {
        auto chunk = chunkStats.find(chunkPos);
        if (chunk != chunkStats.end()) {
            chunk->second.meshingStart = getCurrentTimePoint();
            generationMeshingPauseTimes.addSample((getCurrentTimePoint() - chunk->second.generationStart - chunk->second.generationTime).count());
        }
    }

    static void finishChunkMeshing(glm::ivec2 chunkPos) {
        auto chunk = chunkStats.find(chunkPos);
        if (chunk != chunkStats.end()) {
            chunk->second.meshingTime = getCurrentTimePoint() - chunk->second.meshingStart;
            meshingTimes.addSample(chunk->second.meshingTime.count());
        }
    }

    static void startMeshBuffering(glm::ivec2 chunkPos) {
        auto chunk = chunkStats.find(chunkPos);
        if (chunk != chunkStats.end()) {
            chunk->second.bufferLoadStart = getCurrentTimePoint();
            meshingBufferingPauseTimes.addSample((getCurrentTimePoint() - chunk->second.meshingStart - chunk->second.meshingTime).count());
        }
    }

    static void finishMeshBuffering(glm::ivec2 chunkPos) {
        auto chunk = chunkStats.find(chunkPos);
        if (chunk != chunkStats.end()) {
            chunk->second.bufferLoadTime = getCurrentTimePoint() - chunk->second.bufferLoadStart;
            bufferingTimes.addSample(chunk->second.bufferLoadTime.count());
            chunk->second.completed = 1;
            cutBufferOverflow();
        }
    }

    static double getAverageGenerationTime() {
        return generationTimes.getAverage();
    }

    static double getAverageGenerationMeshingPauseTime() {
        return generationMeshingPauseTimes.getAverage();
    }

    static double getAverageMeshingTime() {
        return meshingTimes.getAverage();
    }

    static double getAverageMeshingBufferingPauseTime() {
        return meshingBufferingPauseTimes.getAverage();
    }

    static double getAverageBufferingTime() {
        return bufferingTimes.getAverage();
    }

    static double getAverageTotalTime() {
        return bufferingTimes.getAverage() + meshingBufferingPauseTimes.getAverage() +
        meshingTimes.getAverage() + generationMeshingPauseTimes.getAverage() + generationTimes.getAverage();
    }
};

