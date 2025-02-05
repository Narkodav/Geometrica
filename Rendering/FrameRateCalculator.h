#pragma once
#include <list>
#include <mutex>
#include <shared_mutex>

#include "Utilities/SampleTracker.h"


class FrameRateCalculator
{
private:
	Utils::SampleTracker<float> frameTimes;
	float totalFrameTime = 0.f;
	float frameRate = 0.f;
	mutable std::shared_mutex mutex;

public:
	void setFrameTimeBuffer(int frameTimeBufferSize);
	void addFrameTime(float frameTime);
	float updateFrameRate();
	float getFrameRate();
};

