#pragma once

namespace Core {

class Application
{
public:
	Application();
	~Application();

	int Exec(int argc, char** argv);
};

} // namespace Core