#include <iostream>
#include "MainLoop.hpp"

int main(int argc, char** argv)
{
	MainLoop mainLoop;

	if (!mainLoop.Initialize()) {
		printf("error: failed to initalize game\n");
		return -1;
	}

	mainLoop.RunLoop();
	mainLoop.Shutdown();

	return 0;
}