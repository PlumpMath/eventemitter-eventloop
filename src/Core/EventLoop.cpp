#include "EventLoop.h"
#include "Log.h"

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace Core
{

namespace
{
	static std::mutex s_eventLoopMutex;
} // anonymous namespace

void EventLoop::DispatchEvent(std::thread::id threadId, std::shared_ptr<CalleeBase> callee)
{
	LOG("dispatching thread-specific Callee to " << threadId)
	std::lock_guard<std::mutex> lock(s_eventLoopMutex);
	GetCalleeMap().insert(std::make_pair(
		threadId,
		callee
	));
}

void EventLoop::ProcessEvents()
{
	std::list<std::shared_ptr<CalleeBase>> callbacks;
	std::list<std::function<void()>> functions;

	// scope - mutex guard to copy and remove the callees
	{
		std::lock_guard<std::mutex> lock(s_eventLoopMutex);


		auto range = GetCalleeMap().equal_range(std::this_thread::get_id());
		callbacks.resize(std::distance(range.first, range.second));

		std::transform(range.first, range.second, callbacks.begin(), [](auto& pair)
		{
			return pair.second;
		});
		GetCalleeMap().erase(std::this_thread::get_id());

		auto frange = GetFunctionMap().equal_range(std::this_thread::get_id());
		functions.resize(std::distance(frange.first, frange.second));
		std::transform(frange.first, frange.second, functions.begin(), [](auto& pair)
		{
			return pair.second;
		});
		GetFunctionMap().erase(std::this_thread::get_id());

	}

	// Safe to invoke the callees.
	for (auto& c : callbacks)
	{
		c->callback();
	}
	for (auto& f : functions)
	{
		f();
	}
}

EventLoop::CalleeMap& EventLoop::GetCalleeMap()
{
	static CalleeMap s_map;
	return s_map;
}

EventLoop::FunctionMap& EventLoop::GetFunctionMap()
{
	static FunctionMap s_map;
	return s_map;
}


} // namespace Core
