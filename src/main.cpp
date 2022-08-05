#include <assert.h>
#include "mainloop.h"
#include "engine.h"

const int GAME_WIDTH=800;
const int GAME_HEIGHT=480;

int main(int argc, const char *const *argv)
{
	auto mainloop = MainLoop();
	auto engine = Engine::newInstance();

	mainloop
		.setEngine(engine)
		.setAppName("LaundryDay")
		.setTitle("Laundry day at bananas manor")
		.setConfigFilename("tins12.cfg")
		.setPreferredGameResolution(GAME_WIDTH, GAME_HEIGHT);

	if (!mainloop.init(argc, argv) && !engine->init())
	{
		mainloop.run();
		engine->done();
	}

	return 0;
}
