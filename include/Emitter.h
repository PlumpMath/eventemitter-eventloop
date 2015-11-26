#pragma once

#include "TypeTag.h"

#include <functional>
#include <thread>
#include <map>
#include <mutex>

namespace EventEmitter {

typedef TypeTag<unsigned int, class _ListenerId_Type> ListenerId;
typedef TypeTag<unsigned int, class _EventId_Type>    EventId;


struct CalleeBase
{
	CalleeBase() {}
	virtual ~CalleeBase() {}
	std::function<void()> callback;
};

enum EventType
{
	Immediate = 0,
	Dispatch = 1,
	Async = 2
};

/**
 * @brief      Base class for objects that can emit events.
 */
class Emitter
{
public:
	Emitter();
	~Emitter();

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
		explicit ListenerBase(ListenerId lid, bool once = false, EventType eventType = Async)
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
			: ListenerBase(lid, once, eventType)
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

	Emitter(const Emitter&) = delete;
	const Emitter& operator= (const Emitter&) = delete;

private:
	std::mutex m_mutex;
	ListenerId m_lastRawListenerId{0};
	typedef std::multimap<EventId, std::shared_ptr<ListenerBase>> EventMap;
	EventMap m_registry;

};

} // namespace Core