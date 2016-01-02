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
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#include <gtest/gtest.h>

EventEmitter::EventId s_testEvent = 1;

TEST(Sanity, Defaults)
{
	EXPECT_TRUE(true);
	EXPECT_EQ(2, 2);
}

/**
 *	Test that Emitter->On(eventId, callback, EventEmitter::Immediate) is called immediately on the main thread.
 */
TEST(On, On_Immediate_Is_Called_On_Same_Thread)
{
	auto e = std::unique_ptr<EventEmitter::Emitter>(new EventEmitter::Emitter);
	auto currentThreadId = std::this_thread::get_id();

	volatile bool testWasRun = false;

	e->On(s_testEvent, [&currentThreadId, &testWasRun]()
	{
		EXPECT_EQ(currentThreadId, std::this_thread::get_id());
		testWasRun = true;
	}, EventEmitter::Immediate);
	auto testStart = Clock::now();
	e->Emit(s_testEvent);

	while (!testWasRun || std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - testStart).count() < 1000)
	{
		// wait for the test to run.
	}
	EXPECT_TRUE(testWasRun);
}

/**
 *	Test that Emitter->On(eventId, callback, EventEmitter::Async) is not called on the main thread.
 */
TEST(On, On_Async_Is_Not_Called_On_Main_Thread)
{
	auto e = std::unique_ptr<EventEmitter::Emitter>(new EventEmitter::Emitter);
	auto currentThreadId = std::this_thread::get_id();
	volatile bool testWasRun = false;

	e->On(s_testEvent, [&currentThreadId, &testWasRun]()
	{
		EXPECT_NE(currentThreadId, std::this_thread::get_id());
		testWasRun = true;
	}, EventEmitter::Async);

	auto testStart = Clock::now();
	e->Emit(s_testEvent);
	while (!testWasRun || std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - testStart).count() < 1000)
	{
		// wait for the test to run.
	}
	EXPECT_TRUE(testWasRun);
}

/**
 *	Test that Emitter->On(eventId, callback) is called as if EventEmitter::Async was passed.
 */
TEST(On, On_Default_Behavior_Is_Async)
{
	auto e = std::unique_ptr<EventEmitter::Emitter>(new EventEmitter::Emitter);
	auto currentThreadId = std::this_thread::get_id();
	volatile bool testWasRun = false;

	e->On(s_testEvent, [&currentThreadId, &testWasRun]()
	{
		EXPECT_NE(currentThreadId, std::this_thread::get_id());
		testWasRun = true;
	});

	auto testStart = Clock::now();
	e->Emit(s_testEvent);
	while (!testWasRun || std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - testStart).count() < 1000)
	{
		// wait for the test to run.
	}
	EXPECT_TRUE(testWasRun);
}

/**
 *	Test that Emitter->On(eventId, callback, EventEmitter::Dispatch) is called on the correct thread.
 *	This requires setting up a thread which will call On.  For simplicity, we don't use a two-lock scheme for
 *	synchronization.  Intead, we give the thread 2 seconds to start, assume it attaches a callback, and then Emit().
 *
 *	TODO: Assumptions are bad in unit tests.  This should be synchronized.
 */
TEST(On, On_Dispatch_Is_Called_On_Dispatching_Thread)
{
	auto e = std::unique_ptr<EventEmitter::Emitter>(new EventEmitter::Emitter);
	auto eptr = e.get();
	volatile bool testWasRun = false;
	auto currentThreadId = std::this_thread::get_id();

	std::thread t([&currentThreadId, &testWasRun, eptr]()
	{
		auto threadId = std::this_thread::get_id();
		volatile bool wait = true;
		eptr->On(s_testEvent, [&threadId, &currentThreadId, &wait, &testWasRun]()
		{
			EXPECT_EQ(threadId, std::this_thread::get_id());
			EXPECT_NE(currentThreadId, threadId);
			EXPECT_NE(currentThreadId, std::this_thread::get_id());
			wait = false;
			testWasRun = true;
		}, EventEmitter::Dispatch);

		while (wait)
		{
			EventEmitter::Registry::ProcessEvents();
		}
	});
	t.detach();

	std::this_thread::sleep_for(std::chrono::seconds(2));
	auto testStart = Clock::now();
	e->Emit(s_testEvent);
	while (!testWasRun)
	{
		auto current = Clock::now();
		auto d = std::chrono::duration_cast<std::chrono::seconds>(current - testStart).count();
		if (d > 3)
		{
			break;
		}
		// wait for the test to run.
	}
	EXPECT_TRUE(testWasRun);
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
