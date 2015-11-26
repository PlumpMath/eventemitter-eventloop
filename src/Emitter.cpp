#include "Emitter.h"
#include "Registry.h"

#include <algorithm>
#include <cassert>
#include <future>
#include <list>
#include <mutex>


namespace EventEmitter
{

namespace
{
	static std::mutex s_processEventMutex;
} // anonymous namespace
Emitter::Emitter() {}

Emitter::~Emitter() {}

ListenerId Emitter::On(EventId eventId, std::function<void()> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, false, eventType);
}

template <typename... Arguments>
ListenerId Emitter::On(EventId eventId, std::function<void(Arguments...)> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, false, eventType);
}

ListenerId Emitter::Once(EventId eventId, std::function<void()> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, true, eventType);
}

template <typename... Arguments>
ListenerId Emitter::Once(EventId eventId, std::function<void(Arguments...)> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, true, eventType);
}


ListenerId Emitter::AddEventListener(EventId eventId, std::function<void()> cb, bool once, EventType eventType)
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
ListenerId Emitter::AddEventListener(EventId, std::function<void(Arguments...)> cb, bool once, EventType eventType)
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

void Emitter::Off(ListenerId listenerId)
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
void Emitter::Emit(EventId eventId, Arguments... args)
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
		else if (c->eventType == Dispatch)
		{
			Registry::DispatchEvent(c->threadId, [c]() {c->callback(args...); });
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

void Emitter::Emit(EventId eventId)
{
	Emit<>(eventId);
}

} // namespace Core