#include "Core/EventEmitter.h"
#include "Core/Log.h"
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
		using namespace std::literals;

		// scope to serialize access to std::cout
		LOG("             binding event on thread " << std::this_thread::get_id())

		g_emitter->On(testEvent, m_function, Core::EventEmitter::EventLoop);
		do
		{
			std::this_thread::sleep_for(1s);
			Core::EventLoop::ProcessEvents();
		} while (g_running);
	}

	std::function<void()> m_function;
};

int main(int argc, char** argv)
{
	{
		using namespace std::literals;

		g_emitter = new Core::EventEmitter;

		g_emitter->On(testEvent, []()
		{
			LOG("[EVENT      ] This should be the main thread " << std::this_thread::get_id())
		}, Core::EventEmitter::Immediate);


		g_emitter->On(testEvent, []()
		{
			LOG("[EVENT ASYNC] This should be async " << std::this_thread::get_id())
		});


		auto t1 = TestBase([]()
		{
			LOG("[EVENT t1   ] This should be the first thread " << std::this_thread::get_id())
		});

		auto t2 = TestBase([]()
		{
			LOG("[EVENT t2   ] This should be the second thread " << std::this_thread::get_id())
		});


		std::thread thread1(&TestBase::run, t1);
		std::thread thread2(&TestBase::run, t2);
		thread1.detach();
		thread2.detach();

		std::this_thread::sleep_for(4s);
		LOG("=============== FIRING EVENT 1 FROM " << std::this_thread::get_id() << " ==================")
		g_emitter->Emit(testEvent);
		std::this_thread::sleep_for(4s);
		g_running = false;

	}
#if defined(WIN32)
	system("pause");
#else
	system("read");
#endif
}