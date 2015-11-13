/**
 *	Based on EventEmitter.h, lifted from
 *  https://gist.github.com/rioki/1290004d7505380f2b1d by Sean Farrell,
 *  which is itself based on the nodejs EventEmitter https://nodejs.org/api/events.html
 *
 *  @brief a cross-thread event emitter base class.
 *
 *	What's awesome about the gist is that it feels like asynchronous programming.
 *  What's not so awesome is that it isn't actually async.  Calling EventEmitter::Emit<>() is actually a
 *	blocking operation.  So, I wanted to make two small changes to it.
 *
 *  First: make the firing be either Immediate (synchronous) or Asynchronous (fired with std::async)
 *
 *  Second: turn it into a means of message passing between threads by adding a ThreadLocal event type.
 *  Events of this type will be Emit() into a thread_local storage map to be processed by an event loop later.
 *  The thread that calls On() creates the Listener with it's threadId; Emit() will put it into a cross-thread
 *  map of events, and the thread will be responsible for invoking ProcessEvents().
 *
 *	TODO:
 *	Off() should remove the callback from thread storage.
 *	Emit() should ensure the thread is running before adding a callback into it's map.
 */
#pragma once

#include "TypeTag.h"
#include "EventLoop.h"

#include <functional>
#include <thread>
#include <mutex>
#include <map>

namespace Core {

typedef TypeTag<unsigned int, class _ListenerId_Type> ListenerId;
typedef TypeTag<unsigned int, class _EventId_Type>    EventId;


class EventLoopRegistry;
/**
 * @brief      Base class for objects that can emit events.
 */
class EventEmitter
{
public:
	EventEmitter();
	~EventEmitter();

	enum EventType
	{
		Immediate = 0,
		EventLoop = 1,
		Async = 2
	};


	ListenerId On(EventId eventId, std::function<void()> callback, EventType = Async);

	template <typename... Arguments>
	ListenerId On(EventId eventId, std::function<void(Arguments...)> callback, EventType = Async);


	ListenerId Once(EventId eventId, std::function<void()> callback, EventType = Async);

	template <typename... Arguments>
	ListenerId Once(EventId eventId, std::function<void(Arguments...)> callback, EventType = Async);

	void Off(ListenerId);

	template <typename... Arguments>
	void Emit(EventId, Arguments... arguments);

	void Emit(EventId);

private:
	struct ListenerBase
	{
		ListenerBase(){}
		explicit ListenerBase(ListenerId lid, std::shared_ptr<EventLoopRegistry> registry = nullptr, bool once = false, EventType eventType = Async)
			: listenerId(lid)
			, once(once)
			, eventType(eventType)
			, threadId(std::this_thread::get_id())
		{

		}
		virtual ~ListenerBase() {}
		ListenerId listenerId; // Listener ID for management
		bool once{ false }; // should the event self-remove after processing?
		EventType eventType{ Immediate };
		std::thread::id threadId;
	};

	/**
	 *	Behold!  Type Erasure!
	 */
	template <typename... Arguments>
	struct Listener : public ListenerBase
	{
		Listener() {}
		Listener(ListenerId lid, std::function<void(Arguments...)> cb, bool once = false, EventType eventType = Async)
			: ListenerBase(lid, nullptr, once, eventType)
			, callback(cb)
		{
			// empty
		}
		std::function<void(Arguments...)> callback;
	};

private:
	ListenerId AddEventListener(EventId eventId, std::function<void()> callback, bool once, EventType eventType);

	template <typename... Arguments>
	ListenerId AddEventListener(EventId eventId, std::function<void(Arguments...)> callback, bool once, EventType eventType);

	EventEmitter(const EventEmitter&) = delete;
	const EventEmitter& operator= (const EventEmitter&) = delete;

private:
	std::mutex m_mutex;
	ListenerId m_lastRawListenerId{0};
	typedef std::multimap<EventId, std::shared_ptr<ListenerBase>> EventMap;
	EventMap m_registry;

};

} // namespace Core