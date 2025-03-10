#pragma once
#include "MultiThreading/EventSystem.h"
#include "MultiThreading/ThreadPool.h"
#include "MultiThreading/TaskCoordinator.h"
#include "GameClock.h"

template <typename EventPolicy>
class GameServicesInterface;

template <typename EventPolicy>
struct GameServices //only passed by reference
{
	MT::EventSystem<EventPolicy> eventSystem;
	MT::ThreadPool threadPool;
	GameClock clock;

	GameServices() = default;

	GameServices(const GameServices&) = delete;
	GameServices& operator=(const GameServices&) = delete;

	GameServices(GameServices&&) = delete;
	GameServices& operator=(GameServices&&) = delete;

	GameServicesInterface<EventPolicy> getInterface() {
		return GameServicesInterface<EventPolicy>(*this);
	}
};

//interface function signatures and names match exactly with the ones they forward to

template <typename EventPolicy>
class EventSystemInterface
{
private:
	mutable MT::EventSystem<EventPolicy>* m_eventSystem;

public:
	inline explicit EventSystemInterface(GameServices<EventPolicy>& gameServices) :
		m_eventSystem(&gameServices.eventSystem) {};

	EventSystemInterface(const EventSystemInterface&) = default;
	EventSystemInterface& operator=(const EventSystemInterface&) = default;

	EventSystemInterface(EventSystemInterface&&) = default;
	EventSystemInterface& operator=(EventSystemInterface&&) = default;

	template<MT::EventSystem<EventPolicy>::EventEnum E, typename... Args>
	inline void emit(Args&&... args) const {
		m_eventSystem->emit<E>(std::forward<Args>(args)...);
	};

	template<MT::EventSystem<EventPolicy>::EventEnum E>
	inline MT::EventSystem<EventPolicy>::Subscription subscribe(
		MT::EventSystem<EventPolicy>::Callback<E> callback) const {
		return m_eventSystem->subscribe<E>(callback);
	}
};

class ThreadPoolInterface
{
private:
	mutable MT::ThreadPool* m_threadPool;

public:
	template <typename EventPolicy>
	inline explicit ThreadPoolInterface(GameServices<EventPolicy>& gameServices) :
		m_threadPool(&gameServices.threadPool) {};

	ThreadPoolInterface(const ThreadPoolInterface&) = default;
	ThreadPoolInterface& operator=(const ThreadPoolInterface&) = default;

	ThreadPoolInterface(ThreadPoolInterface&&) = default;
	ThreadPoolInterface& operator=(ThreadPoolInterface&&) = default;

	inline void pushTask(std::function<void()> task) const
	{
		m_threadPool->pushTask(task);
	}

	//// Vector of tasks with move semantics
	//inline void pushTasks(std::vector<std::function<void()>>&& tasks) {
	//	m_threadPool.pushTasks(tasks);
	//}

	//// Const reference vector version
	//inline void pushTasks(const std::vector<std::function<void()>>& tasks) {
	//	m_threadPool.pushTasks(tasks);
	//}

	//// Iterator range version
	//template<typename Iterator>
	//inline void pushTasks(Iterator begin, Iterator end) {
	//	m_threadPool.pushTasks(begin, end);
	//}

	//// Variadic template for multiple individual tasks
	//template<typename... Tasks>
	//inline void pushTasks(Tasks&&... tasks) {
	//	m_threadPool.pushTasks(std::forward<Tasks>(tasks)...);
	//}

	inline void pushPriorityTask(std::function<void()> task) const
	{
		m_threadPool->pushPriorityTask(task);
	}

	//// Vector of tasks with move semantics
	//inline void pushPriorityTasks(std::vector<std::function<void()>>&& tasks) {
	//	m_threadPool.pushPriorityTasks(tasks);
	//}

	//// Const reference vector version
	//inline void pushPriorityTasks(const std::vector<std::function<void()>>& tasks) {
	//	m_threadPool.pushPriorityTasks(tasks);
	//}

	//// Iterator range version
	//template<typename Iterator>
	//inline void pushPriorityTasks(Iterator begin, Iterator end) {
	//	m_threadPool.pushPriorityTasks(begin, end);
	//}

	//// Variadic template for multiple individual tasks
	//template<typename... Tasks>
	//inline void pushPriorityTasks(Tasks&&... tasks) {
	//	m_threadPool.pushPriorityTasks(std::forward<Tasks>(tasks)...);
	//}


	inline MT::TaskCoordinator getTaskCoordinator(unsigned int maxPendingTasks) const
	{
		return MT::TaskCoordinator(*m_threadPool, maxPendingTasks);
	}

	inline int getFreeWorkers() const { return m_threadPool->getFreeWorkers(); };

	inline unsigned int getWorkerAmount() const { return m_threadPool->getWorkerAmount(); };

	inline size_t getQueueSize() const { return m_threadPool->getQueueSize(); };
};

class GameClockInterface
{
private:
	mutable GameClock* m_clock;

public:
	template <typename EventPolicy>
	inline explicit GameClockInterface(GameServices<EventPolicy>& gameServices) :
		m_clock(&gameServices.clock) {};

	GameClockInterface(const GameClockInterface&) = default;
	GameClockInterface& operator=(const GameClockInterface&) = default;

	GameClockInterface(GameClockInterface&&) = default;
	GameClockInterface& operator=(GameClockInterface&&) = default;

	inline uint32_t getGlobalTime() const { return m_clock->getGlobalTime(); };
};

template <typename EventPolicy>
class GameServicesInterface :
	public EventSystemInterface<EventPolicy>,
	public ThreadPoolInterface,
	public GameClockInterface
{
public:

	GameServicesInterface(GameServices<EventPolicy>& gameServices) :
		EventSystemInterface<EventPolicy>(gameServices),
		ThreadPoolInterface(gameServices),
		GameClockInterface(gameServices) {};

	GameServicesInterface(const GameServicesInterface&) = default;
	GameServicesInterface& operator=(const GameServicesInterface&) = default;

	GameServicesInterface(GameServicesInterface&&) = default;
	GameServicesInterface& operator=(GameServicesInterface&&) = default;
};
