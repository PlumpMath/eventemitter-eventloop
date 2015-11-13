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

---

Copyright (c) 2014 Sean Farrell

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
#include "EventEmitter.h"

#include <algorithm>
#include <cassert>
#include <future>
#include <iterator>
#include <list>
#include <mutex>


namespace Core
{

namespace
{
	static std::mutex s_processEventMutex;
} // anonymous namespace
EventEmitter::EventEmitter() {}

EventEmitter::~EventEmitter() {}

ListenerId EventEmitter::On(EventId eventId, std::function<void()> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, false, eventType);
}

template <typename... Arguments>
ListenerId EventEmitter::On(EventId eventId, std::function<void(Arguments...)> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, false, eventType);
}

ListenerId EventEmitter::Once(EventId eventId, std::function<void()> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, true, eventType);
}

template <typename... Arguments>
ListenerId EventEmitter::Once(EventId eventId, std::function<void(Arguments...)> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, true, eventType);
}


ListenerId EventEmitter::AddEventListener(EventId eventId, std::function<void()> cb, bool once, EventType eventType)
{
	if (!cb)
	{
		return ListenerId(0);
	}
	std::lock_guard<std::mutex> lock(m_mutex);
	ListenerId nextListenerId = ++m_lastRawListenerId++;
	m_registry.insert(
		std::make_pair(eventId, std::make_shared<Listener<>>(nextListenerId, cb, once, eventType))
	);

	return nextListenerId;
}

template <typename... Arguments>
ListenerId EventEmitter::AddEventListener(EventId, std::function<void(Arguments...)> cb, bool once, EventType eventType)
{
	if (!cb)
	{
		return ListenerId(0);
	}

	std::lock_guard<std::mutex> lock(m_mutex);

	auto nextListenerId = m_lastRawListenerId++;
	m_registry.insert(
		std::make_pair(eventId, std::make_shared<Listener<Arguments...>>(nextListenerId, cb, once, eventType) )
	);

	return nextListenerId;
}

void EventEmitter::Off(ListenerId listenerId)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto findIter = std::find_if(m_registry.begin(), m_registry.end(),
		[&] (std::pair<EventId, std::shared_ptr<ListenerBase>> p){
			return p.second->listenerId == listenerId;
		});

	if (findIter != m_registry.end())
	{
		m_registry.erase(findIter);
	}
}

template <typename... Arguments>
void EventEmitter::Emit(EventId eventId, Arguments... args)
{
	std::list<std::shared_ptr<Listener<Arguments...>>> callbacks;

	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto range = m_registry.equal_range(eventId);
		callbacks.resize(std::distance(range.first, range.second));
		std::transform(range.first, range.second, callbacks.begin(), [] (std::pair<EventId, std::shared_ptr<ListenerBase>> p)
		{
			auto l = std::dynamic_pointer_cast<Listener<Arguments...>>(p.second);
			assert(l);

			return l;
		});
	}

	for (auto& c : callbacks)
	{
		if (c->eventType == Async)
		{
			auto future = std::async(std::launch::async, c->callback, args...);
		}
		else if (c->eventType == EventLoop)
		{
			EventLoop::DispatchEvent(c->threadId, std::make_shared<Callee<Arguments...>>(c->callback, args...));
		}
		else
		{
			c->callback(args...);
		}
		if (c->once)
		{
			Off(c->listenerId);
		}

	}
}

void EventEmitter::Emit(EventId eventId)
{
	Emit<>(eventId);
}

} // namespace Core