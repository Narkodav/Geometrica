#include "FrameRateCalculator.h"


void FrameRateCalculator::setFrameTimeBuffer(int frameTimeBufferSize)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    frameTimes.rename("FrameTimes");
    frameTimes.resize(frameTimeBufferSize);
}

void FrameRateCalculator::addFrameTime(float frameTime)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    frameTimes.addSample(frameTime);
}

float FrameRateCalculator::updateFrameRate()
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    return frameRate = 1 / frameTimes.getAverage();
}

float FrameRateCalculator::getFrameRate()
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return frameRate;
}