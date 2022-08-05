#include <assert.h>
#include "mainloop.h"
#include "engine.h"

const int GAME_WIDTH=640;
const int GAME_HEIGHT=480;

using namespace std;

int main(int argc, const char *const *argv)
{
	auto mainloop = MainLoop();
	auto engine = make_shared<Engine>();

	mainloop
		.setEngine(engine)
		.setAppName("Fashionista")
		.setTitle("Fashionista: Match or Die!")
		.setConfigFilename("fashionista.cfg");

	if (!mainloop.init(argc, argv) && !engine->init())
	{
		mainloop.run();
	}

	return 0;
}
