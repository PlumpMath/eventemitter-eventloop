#include "Core/EventEmitter.h"

#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>

namespace {
	std::mutex g_mutex;
	bool g_running = true;
	Core::EventEmitter* g_emitter = nullptr;
	auto testEvent = Core::EventId(1);

} // anonymous namespace

class TestBase
{
public:
	explicit TestBase(std::function<void()> callFunction)
		: m_function(callFunction)
	{}

	void run()
	{
		// scope to serialize access to std::cout
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			std::cout << "starting thread " << std::this_thread::get_id() << std::endl;
		}
		g_emitter->On(testEvent, m_function);
		do {} while (g_running);
	}

	std::function<void()> m_function;
};

int main(int argc, char** argv)
{
	using namespace std::literals;
	std::cout << "This is the main thread " << std::this_thread::get_id() << std::endl;

	g_emitter = new Core::EventEmitter;

	g_emitter->On(testEvent, []()
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		std::cout << "This should be the main thread " << std::this_thread::get_id() << std::endl;
	});

	/*
	auto t1 = TestBase([]()
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		std::cout << "This should be the first thread " << std::this_thread::get_id() << std::endl;
	});

	auto t2 = TestBase([]()
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		std::cout << "This should be the second thread " << std::this_thread::get_id() << std::endl;
	});


	std::thread thread1(&TestBase::run, t1);
	std::thread thread2(&TestBase::run, t2);
	thread1.join();
	thread2.detach();
	*/
	std::this_thread::sleep_for(1s);
	g_emitter->Emit(testEvent);
	std::this_thread::sleep_for(1s);
	g_running = false;
}