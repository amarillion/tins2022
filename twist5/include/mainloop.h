#pragma once

#include <allegro5/allegro.h>
#include "component.h"
#include "sound.h"
#include "timer.h"
#include "point.h"

#include <string>
#include <vector>
#include <map>

#ifdef USE_MONITORING
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
#endif

/**
 * Equivalent of mainLoop->getw().
 * see there.
 * <p>
 * Represents the logical screen size, the optimal screen size for which the game was designed to run.
 * Buffer or display size may be different because they may have a transformation on them.
 * <p>
 * For example, the game may (conservatively) be designed for a 640x480 screen resolution.
 * However, the desktop resolution is 1920x1080, and this is the size you'd get for
 * the buffer bitmap width or the display width. Because the transformation applies, you'd draw
 * to the buffer as though it was sized 640x480.
 */
#define MAIN_WIDTH MainLoop::getMainLoop()->getw()

/**
 * Equivalent of mainLoop->geth().
 * See MAIN_WIDTH
 */
#define MAIN_HEIGHT MainLoop::getMainLoop()->geth()

#define TICKS_FROM_MSEC(x) ((x) / MainLoop::getMainLoop()->getLogicIntervalMsec())
#define MSEC_FROM_TICKS(x) ((x) * MainLoop::getMainLoop()->getLogicIntervalMsec())

class RootComponent;

class MainLoop final : public Component, public Sound, public ITimer
{
private:
	ALLEGRO_BITMAP *buffer;
	ALLEGRO_EVENT_QUEUE *equeue;
	ALLEGRO_TIMER *logicTimer;
	ALLEGRO_DISPLAY *display;

	std::shared_ptr<RootComponent> rootPane;
	ComponentPtr engine;

	ALLEGRO_PATH *localAppData;
	ALLEGRO_PATH *configPath;

	const char *configFilename;
	const char *title;
	const char *appname;
	
	int frame_count;
	int frame_counter;
	int last_fps;
	int lastUpdateMsec;

	enum ScreenMode { WINDOWED = 0, FULLSCREEN_WINDOW, FULLSCREEN };
	ScreenMode screenMode;

	/**
	 * If resolution == fixed, this will be the backbuffer size.
	 * otherwise, this will be only used in windowed mode to determine the default window size.
	 */
	Point prefGameSize;

	/**
	 * This will be the default display size
	 * Usually equal to prefGameSize.
	 */
	Point prefDisplaySize;

	// indicates whether buffer size is different from display size
	bool stretch;

	// If useFixedResolution is on, the buffer will be the same size as w, h.
	// if useFixedResolution is off, the buffer can be any size, but w, h will be used as the default window size if there is that flexibility.
	// this could also be described as responsive mode.
	bool useFixedResolution = true;

	// quick hack for usagi.
	// usagi means a responsive resolution with a hardcoded set of breakpoints where game scale is increased
	bool usagiMode = false;
/*
 //TODO - implement this with letterboxing
	bool useFixedAspectRatio = false;
	Point fixedAspectRatio;
 */

	// in smoke test mode: don't create display, just test loading resources.
	// smokeTest is in headless mode.
	bool smokeTest;
	
	void getFromConfig(ALLEGRO_CONFIG *config);
	void getFromArgs(int argc, const char *const *argv);

	int logicIntervalMsec;

	bool isResizableWindow = false;

	static MainLoop *instance;
	int initDisplay();

	/** initialises default skin */
	void initSkin();

	std::vector<std::string> options;
	bool checkMessages();
#ifdef USE_MONITORING
	Clock::time_point t0; // time since start of program
	Clock::time_point t1; // time since start of update loop
	Clock::time_point t2; // time since start of phase
	void logStartTime(const std::string &phase);
	void logEndTime(const std::string &phase);
	std::map<std::string, long> sums;
	std::map<std::string, int> counts;
#endif
protected:
	ALLEGRO_CONFIG *config;
	bool fpsOn;
	virtual void UpdateSize() override;
public:
	bool isSmokeTest() { return smokeTest; }

	// randomly generated id used to identify recurring user
	std::string getUserId();

	template <typename V>
	void adjustMickey(V &x, V &y)
	{
		if (stretch)
		{
			x = x * w / al_get_display_width(display);
			y = y * h / al_get_display_height(display);
		}
	}

	/** return vector of unhandled command-line arguments */
	std::vector<std::string> &getOpts() { return options; }

	ALLEGRO_CONFIG *getConfig() { return config; }
	
	int getMsecCounter () { return al_get_timer_count(logicTimer) * logicIntervalMsec; }
	void setFpsOn (bool value) { fpsOn = value; }

	MainLoop (Component *_engine, const char *configFilename, const char *title, int _bufw = 640, int _bufh = 480);
	MainLoop ();

	/**
	 * indicate that the game is
	 *
	 * a. designed for a certain fixed resolution,
	 * OR
	 * b. adapts to any resolution it's given
	 *
	 * In case a), a fixed size buffer will be created and stretched / transformed to match the actual display resolution.
	 *      (possibly letterboxing may be used to maintain aspect ratio depending on the setFixedAspectRatio setting)
	 *
	 * In case b), game routines are expected to handle a variety of resolutions dynamically.
	 *      w, h are used for the default window size size in windowed mode.
	 *
	 */
	MainLoop &setFixedResolution (bool fixed);

	//TODO: not yet implemented
//	MainLoop &setFixedAspectRatio (bool fixed, int x = 4, int y = 3);

	MainLoop &setTitle(const char *_title);
	MainLoop &setAppName(const char *_appname);
	MainLoop &setConfigFilename(const char *_configFilename);
	MainLoop &setEngine(ComponentPtr _engine);
	MainLoop &setLogicIntervalMsec (int value) { logicIntervalMsec = value; return *this; }
	
	// responsive, but scales screen at specific breakpoints. TODO: control breakpoints
	MainLoop &setUsagiMode() { usagiMode = true; return *this; }

	MainLoop &setPreferredGameResolution (int w, int h) { prefGameSize = Point(w, h); return *this; }

	/** Can be used to scale up the display for really low resolutions. if this isn't set, game resolution is used */
	MainLoop &setPreferredDisplayResolution (int w, int h) { prefDisplaySize = Point(w, h); return *this; }

	MainLoop &setResizableWindow (bool value) { isResizableWindow = value; return *this; }

	ComponentPtr getEngine() { return engine; }
	/**
	 * returns 0 on success, 1 on failure
	 */
	int init(int argc, const char *const *argv);
	void run();	
	virtual ~MainLoop();
	
	virtual void parseOpts(std::vector<std::string> &opts) {};
	int getLogicIntervalMsec () { return logicIntervalMsec; }

	void toggleWindowed();
	bool isWindowed();

	/**
	 * If parent != null, any command messages coming from the popupWindow
	 * are passed to the parent.
	 */
	void popup (ComponentPtr popupWindow, ComponentPtr parent = nullptr);

	void pumpMessages();

	static MainLoop *getMainLoop();
};
