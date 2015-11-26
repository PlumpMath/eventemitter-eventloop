#pragma once

#include <functional>
#include <mutex>
#include <map>
#include <queue>
#include <thread>

namespace EventEmitter
{

// Simple typedef for a void function()
typedef std::function<void()> Callback;

/**
 * There are two registrys: the "static" global registry (which you should use) 
 * and an internal, thread_local ThreadLocalRegistry (which you should not).
 * Registry::DispatchEvent() dispatches an event to be processed on a specfic thread.
 * You can only DispatchEvent() to a thread that is running.  Nothing happens if the
 * thread has already terminated.
 *
 * ThreadLocalRegistry informs Registry upon thread termination that it's done, and Registry
 * will clear any unprocessed events.
*/
class Registry
{
public:
	Registry() {};
	static void DispatchEvent(std::thread::id, Callback);
	static void ProcessEvents();

	void Dispatch(std::thread::id, Callback);
	void Process();
private:
	/**
	 * ThreadLocalRegistry is thread_local: the thread deletes it.
	 */
	class ThreadLocalRegistry
	{
	public:
		ThreadLocalRegistry();
		~ThreadLocalRegistry();
		void Dispatch(Callback callback);
		void Process();

	private:
		std::queue<Callback> m_callbacks;
		std::mutex m_mutex;
	};

private:
	/**
	 * Registry does not own *localRegistry.
	 */
	void AddRegistry(std::thread::id, ThreadLocalRegistry* localRegistry);
	void RemoveRegistry(std::thread::id);

	static thread_local ThreadLocalRegistry l_registry;
	std::map<std::thread::id, ThreadLocalRegistry*> m_registryMap;
	std::mutex m_mutex;
};

} // namespace EventEmitter
