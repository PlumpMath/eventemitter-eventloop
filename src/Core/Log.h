#pragma once

#include <mutex>
#include <iostream>

namespace Core
{

static std::mutex s_logMutex;

#define LOG(x) \
{\
	std::lock_guard<std::mutex> lock(Core::s_logMutex);\
	std::cout << x << std::endl;\
}

} // namespace Core