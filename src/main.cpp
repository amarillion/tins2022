#include <assert.h>
#include "constants.h"
#include "mainloop.h"
#include "engine.h"
#include "crossfade.h"

using namespace std;

// definition of main must be exactly this, or you'll get al_mangled_main errors on Mac os x.
// so no const char antything
int main(int argc, char **argv)
{
    // Fix unneeded copy, clang doesn't like it on C++14
    MainLoop mainloop {};
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
