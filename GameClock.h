#pragma once
#include <chrono>
#include <atomic>

//counts global time in ticks
//doesn't actually correspond to real time

class GameClock
{
public:
	static inline const float ticksPerSecond = 60.f;
	static inline const float deltaTime = 1 / ticksPerSecond; //one tick in seconds

private:
	std::atomic<uint32_t> globalTime = 0;

public:
	inline void reset() { globalTime.store(0); };
	inline uint32_t getGlobalTime() const { return globalTime.load(); };
	inline void increment() { ++globalTime; };

};

