#pragma once

#include <mutex>
#include <iostream>

static std::mutex s_logMutex;

#define LOG(x) \
{\
	std::lock_guard<std::mutex> lock(s_logMutex);\
	std::cout << x << std::endl;\
}