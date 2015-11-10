#include "EventEmitter.h"


#include <algorithm>
#include <cassert>
#include <future>
#include <iterator>
#include <list>
#include <mutex>


namespace Core
{

EventEmitter::EventEmitter() {}

EventEmitter::~EventEmitter() {}

ListenerId EventEmitter::On(EventId eventId, std::function<void()> cb, EventType eventType)
{
	return AddEventListener(eventId, cb, false, eventType);
}

template <typename... Arguments>
ListenerId EventEmitter::On(EventId, std::function<void(Arguments...)> cb, EventType eventType)
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
		else if (c->eventType == ThreadLocal)
		{
			s_threadCallees.insert(std::make_pair(
					std::this_thread::get_id(),
					std::make_shared<Callee<Arguments...>>(c->callback, args...)
			));

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

void EventEmitter::ProcessEvents()
{

	std::list<std::shared_ptr<CalleeBase>> callbacks;

	// scope - mutex guard to copy and remove the callees
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto range = s_threadCallees.equal_range(std::this_thread::get_id());
		callbacks.resize(std::distance(range.first, range.second));

		std::transform(range.first, range.second, callbacks.begin(), [](auto& pair)
		{
			return pair.second;
		});
		s_threadCallees.erase(std::this_thread::get_id());
	}

	// Safe to invoke the callees.
	for (auto& c : callbacks)
	{
		c->callback();
	}

	// the shared pointers should go out of scope entirely now, and be deleted.
}

} // namespace Core