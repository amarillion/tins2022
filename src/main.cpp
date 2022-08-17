#include <assert.h>
#include "constants.h"
#include "mainloop.h"
#include "engine.h"
#include "crossfade.h"

using namespace std;

int main(int argc, const char *const *argv)
{
	auto mainloop = MainLoop();
	auto engine = Engine::newInstance();

	mainloop
		.setEngine(engine)
		.setAppName("tins22")
		.setTitle("TINS 2022 Entry")
		.setConfigFilename("tins22.cfg")
		.setLogicIntervalMsec(MSEC_PER_TICK)
		.setPreferredGameResolution(GAME_WIDTH, GAME_HEIGHT)
		.setAudioModule(make_unique<CrossFadeAudio>());

	if (!mainloop.init(argc, argv) && !engine->init())
	{
		mainloop.run();
		engine->done();
	}

	return 0;
}
