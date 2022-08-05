#include <anim.h>
#include <input.h>
#include <assert.h>
#include "mainloop.h"
#include "engine.h"

using namespace std;

int main(int argc, const char *const *argv)
{
	auto mainloop = MainLoop();
	auto engine = make_shared<Engine>();

	mainloop
		.setEngine(engine)
		.setAppName("Dr_F")
		.setTitle("Dr. Evil F, Xmas edition")
		.setConfigFilename("Dr_F.cfg");

	if (!mainloop.init(argc, argv) && !engine->init())
	{
		mainloop.run();
		engine->done();
	}

	return 0;
}
