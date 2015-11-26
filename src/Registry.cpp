#include "Registry.h"
namespace EventEmitter {

namespace
{
	static Registry s_registry;
	
} // anonymous namespace

// define the thread-local value.
thread_local Registry::ThreadLocalRegistry Registry::l_registry;

Registry::ThreadLocalRegistry::ThreadLocalRegistry()
{
	s_registry.AddRegistry(std::this_thread::get_id(), this);
}

Registry::ThreadLocalRegistry::~ThreadLocalRegistry() { 	
	// lock - don't let anybody do anything when our thread is being deleted 	
	std::lock_guard<std::mutex> lock(m_mutex); 	
	s_registry.RemoveRegistry(std::this_thread::get_id()); 

}

void Registry::ThreadLocalRegistry::Dispatch(Callback callback)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_callbacks.push(callback);
}

void Registry::ThreadLocalRegistry::Process()
{
	std::lock_guard<std::mutex> lock(m_mutex); 
	while (m_callbacks.size() > 0)
	{
		m_callbacks.front()();
		m_callbacks.pop();
	}
}

void Registry::AddRegistry(std::thread::id threadId, ThreadLocalRegistry* localRegistry)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_registryMap.insert(std::make_pair(threadId, localRegistry));
}

void Registry::RemoveRegistry(std::thread::id threadId)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_registryMap.erase(threadId);
}

/**
 * Just a static wrapper around an specific object.
 */
void Registry::DispatchEvent(std::thread::id threadId, Callback callback)
{
	s_registry.Dispatch(threadId, callback);
}

/**
* Just a static wrapper around an specific object.
*/
void Registry::ProcessEvents()
{
	s_registry.Process();
}

void Registry::Dispatch(std::thread::id threadId, Callback callback)
{
	// dispatching needs to be guarded to prevent the thread being
	// deleted while the registry is being looked-up or dispatched to
	std::lock_guard<std::mutex> lock(m_mutex);

	auto registry = m_registryMap.find(threadId);
	if (registry != m_registryMap.end())
	{
		registry->second->Dispatch(callback);
	}
}

void Registry::Process()
{
	// ThreadLocalRegistry::Process has it's own thread-specific mutex.
	l_registry.Process();
}

} // namespace EventEmitter
