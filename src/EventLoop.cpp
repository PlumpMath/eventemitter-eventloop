/**
Copyright (c) 2015 Matthew Schnee

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**/
#include "EventLoop.h"

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
