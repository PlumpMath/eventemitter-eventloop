/**
Copyright (c) 2015 Matthew Schnee

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**/
#include "Emitter.h"
#include "Registry.h"
#include "Log.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>


namespace {
	std::mutex g_mutex;
	bool g_running = true;
	EventEmitter::Emitter* g_emitter = nullptr;
	auto testEvent = EventEmitter::EventId(1);
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

			g_emitter->On(testEvent, m_function, EventEmitter::Dispatch);
		do
		{
			std::this_thread::sleep_for(1s);
			EventEmitter::Registry::ProcessEvents();
		} while (g_running);
	}

	std::function<void()> m_function;
};

int main(int argc, char** argv)
{
	{
		using namespace std::literals;

		g_emitter = new EventEmitter::Emitter;

		g_emitter->On(testEvent, []()
		{
			LOG("[EVENT      ] This should be the main thread " << std::this_thread::get_id())
		}, EventEmitter::Immediate);


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
	system("pause");
}