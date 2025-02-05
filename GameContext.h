#pragma once
#include "GameEvents.h"
#include "MultiThreading/EventSystem.h"
#include "MultiThreading/ThreadPool.h"

struct GameContext
{
	MT::EventSystem<GameEventPolicy>& gameEvents;
	MT::ThreadPool& threadPool;

	GameContext(MT::EventSystem<GameEventPolicy>& e, MT::ThreadPool& tp)
		: gameEvents(e), threadPool(tp) {}
};


