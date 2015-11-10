#pragma once

#include <thread>
#include <iostream>
#include <map>

namespace Core
{

class ThreadWatcher
{
public:
	ThreadWatcher()
	{
		std::cout << "thread created " << std::this_thread::get_id() << std::endl;
	}
	~ThreadWatcher()
	{
		std::cout << "thread destroyed " << std::this_thread::get_id() << std::endl;
	}
};

namespace
{
	thread_local ThreadWatcher s_watcher;
}

class EventLoop
{
public:
	static EventLoop* GetThreadLoop();
	static void DispatchEvents();

private:


};

} // namespace Core
