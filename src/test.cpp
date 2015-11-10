#include "Core/EventEmitter.h"
#include "Core/EventLoop.h"

#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>


namespace {
	std::mutex g_mutex;
	std::mutex g_logMutex;
	bool g_running = true;
	Core::EventEmitter* g_emitter = nullptr;
	auto testEvent = Core::EventId(1);

} // anonymous namespace

#define LOG(x) \
{\
	std::lock_guard<std::mutex> lock(g_mutex);\
	x\
}

class TestBase
{
public:
	explicit TestBase(std::function<void()> callFunction)
		: m_function(callFunction)
	{}

	void run()
	{
		// scope to serialize access to std::cout
		LOG(std::cout << "starting thread " << std::this_thread::get_id() << std::endl;)

		g_emitter->On(testEvent, m_function);
		do {} while (g_running);
	}

	std::function<void()> m_function;
};

int main(int argc, char** argv)
{
	{
		using namespace std::literals;
		std::cout << "This is the main thread " << std::this_thread::get_id() << std::endl;

		g_emitter = new Core::EventEmitter;

		g_emitter->On(testEvent, []()
		{
			LOG(std::cout << "This should be the main thread " << std::this_thread::get_id() << std::endl;)
		}, Core::EventEmitter::Immediate);


		g_emitter->On(testEvent, []()
		{
			LOG(std::cout << "This should be async " << std::this_thread::get_id() << std::endl;)
		});


		auto t1 = TestBase([]()
		{
			LOG(std::cout << "This should be the first thread " << std::this_thread::get_id() << std::endl;)
		});

		auto t2 = TestBase([]()
		{
			LOG(std::cout << "This should be the second thread " << std::this_thread::get_id() << std::endl;)
		});


		std::thread thread1(&TestBase::run, t1);
		std::thread thread2(&TestBase::run, t2);
		thread1.detach();
		thread2.detach();

		std::this_thread::sleep_for(1s);
		g_emitter->Emit(testEvent);
		std::this_thread::sleep_for(1s);
		g_running = false;

	}
#if defined(WIN32)
	system("pause");
#else
	system("read");
#endif
}