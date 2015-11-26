#pragma once

#include <functional>
#include <mutex>
#include <map>
#include <queue>
#include <thread>

namespace EventEmitter
{

typedef std::function<void()> Callback;

class Registry
{
public:
	Registry() {};
	static void DispatchEvent(std::thread::id, Callback);
	static void ProcessEvents();

	void Dispatch(std::thread::id, Callback);
	void Process();
private:
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
	void AddRegistry(std::thread::id, ThreadLocalRegistry* localRegistry);
	void RemoveRegistry(std::thread::id);

	static thread_local ThreadLocalRegistry l_registry;
	std::map<std::thread::id, ThreadLocalRegistry*> m_registryMap;
};

} // namespace EventEmitter
