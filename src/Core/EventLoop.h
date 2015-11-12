#pragma once

#include <thread>
#include <functional>
#include <map>

namespace Core
{

struct CalleeBase
{
	CalleeBase() {}
	virtual ~CalleeBase() {}
	std::function<void()> callback;
};

template <typename... Arguments>
struct Callee : public CalleeBase
{
	explicit Callee(std::function<void(Arguments...)> cb, Arguments... arguments)
		: CalleeBase()
	{
		callback = [=]()
		{
			cb(arguments...);
		};
	}
};

class EventLoop
{
public:
	static void ProcessEvents();

	static void DispatchEvent(std::thread::id threadId, std::shared_ptr<CalleeBase> callee);

private:


	typedef std::multimap<std::thread::id, std::shared_ptr<CalleeBase>> CalleeMap;
	typedef std::multimap<std::thread::id, std::function<void()>> FunctionMap;

	static CalleeMap& GetCalleeMap();
	static FunctionMap& GetFunctionMap();
};

} // namespace Core
