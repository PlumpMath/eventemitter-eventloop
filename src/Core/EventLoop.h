#pragma once

namespace Core
{

class EventLoop
{
public:
	static EventLoop* GetThreadLoop();
	void DispatchEvents();

private:


};

} // namespace Core
