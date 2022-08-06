#include <assert.h>
#include "mainloop.h"
#include "color.h"

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "textstyle.h"
#include "container.h"

#include <iostream>

using namespace std;

/**
 * Goal of this class is properly handle messages and events at the top hierarchy.
 *
 * The engine is rooted in here, but also popup menus, popup windows and overlays.
 *
 * This component can pass command messages from popupmenu to menubar.
 */
class RootComponent : public Container
{
private:
	map<ComponentPtr, ComponentPtr> popupParentMap;
public:
	RootComponent()
	{
		setLayout (Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 0, 0, 0);
	}

	virtual void update() override
	{
		Container::update();

		// popupParentMap.remove_if(MyComponentRemover);
		//TODO: clean up popupParentMap
	}

	/** add a certain component, but pass any handled messages to parent instead of leaving them on the queue */
	void addWithParent(ComponentPtr child, ComponentPtr parent)
	{
		add(child);
		popupParentMap[child] = parent;
	}

	virtual void handleMessage(ComponentPtr src, int msg) override
	{
		// pass messages to parents...
		if (popupParentMap.find(src) != popupParentMap.end())
		{
			popupParentMap[src]->handleMessage(src, msg);
		}
		else {
			Container::handleMessage(src, msg);
		}

	}

	virtual std::string const className() const override { return "RootComponent"; }
};

MainLoop *MainLoop::instance = NULL;

MainLoop *MainLoop::getMainLoop()
{
	return instance;
}

MainLoop &MainLoop::setTitle(const char *_title)
{
	title = _title;
	return *this;
}

MainLoop &MainLoop::setAppName(const char *_appname)
{
	appname = _appname;
	return *this;
}

MainLoop &MainLoop::setConfigFilename(const char *_configFilename)
{
	configFilename = _configFilename;
	return *this;
}

MainLoop &MainLoop::setEngine(ComponentPtr _engine)
{
	engine = _engine;
	engine->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 0, 0, 0);
	rootPane->add(engine);
	rootPane->setFocus(engine);
	return *this;
}

MainLoop::MainLoop() :
		buffer(NULL), equeue(NULL), logicTimer(NULL), display(NULL),
		engine(), localAppData(NULL), configPath(NULL),
		configFilename("twist.cfg"), title("untitled"), appname(NULL),
		prefGameSize(Point(640, 480)), prefDisplaySize(Point(-1, -1)), stretch (false), smokeTest(false), logicIntervalMsec(20),
#ifdef USE_MONITORING
		t0(), t1(), sums(), counts(),
#endif
		config(NULL)
{
	w = 640;
	h = 480;
	buffer = NULL;
	last_fps = 0;
	lastUpdateMsec = 0;
	frame_count = 0;
	frame_counter = 0;
#ifdef DEBUG
	// default in debug version
	screenMode = WINDOWED;
	fpsOn = true;
#else
	// default in release version
	// screenMode = FULLSCREEN_WINDOW;
	screenMode = WINDOWED; // temporarily default in release mode...
	fpsOn = false;
#endif
	instance = this;
	rootPane = make_shared<RootComponent>();
}

void MainLoop::getFromConfig(ALLEGRO_CONFIG *config)
{
	getSoundFromConfig(config);
	fpsOn = get_config_int (config, "twist", "fps", fpsOn);
	screenMode = (ScreenMode)get_config_int (config, "twist", "windowed", screenMode);
}

void MainLoop::getFromArgs(int argc, const char *const *argv)
{
	// parse command line arguments
	int i;
	for (i = 1; i < argc; i++)
	{
		if (strcmp (argv[i], "-nosound") == 0)
		{
			setSoundInstalled(false);
		}
		else if (strcmp (argv[i], "-windowed") == 0)
		{
			screenMode = WINDOWED;
		}
		else if (strcmp (argv[i], "-fullscreen") == 0)
		{
			screenMode = FULLSCREEN_WINDOW; //TODO: add third option for FULLSCREEN for extra efficiency
		}
		else if (strcmp (argv[i], "-showfps") == 0)
		{
			fpsOn = true;
		}
		else if (strcmp (argv[i], "-smoketest") == 0)
		{
			smokeTest = true;
		}
		else
		{
			options.push_back (string(argv[i]));
		}
	}	
}


int MainLoop::init(int argc, const char *const *argv)
{
	if (!al_init())
	{
		allegro_message("al_init() failed");
		return 1;
	}

	// initialise application name
	if (appname == nullptr) {
		appname = "anonymous_twist_app";
	}

	al_set_app_name(appname);
	al_set_org_name("helixsoft.nl");

	localAppData = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);

	bool result = al_make_directory(al_path_cstr(localAppData, ALLEGRO_NATIVE_PATH_SEP));
	if (!result)
	{
		cout << "Failed to create application data directory " << al_get_errno();
		//TODO: write message to log
	}

	if (!al_init_image_addon())
	{
		allegro_message("init image addon failed");
		return 1;
	}

	al_init_font_addon(); //never fails, no return value...

	if (configFilename != NULL)
	{
		configPath = al_clone_path(localAppData);
		al_set_path_filename(configPath, configFilename);

		config = al_load_config_file (al_path_cstr(configPath, ALLEGRO_NATIVE_PATH_SEP));
	}

	if (config == NULL)
	{
		config = al_create_config();
	}

	getFromConfig(config);
	getFromArgs (argc, argv);

	parseOpts(options);
	
	if (!al_install_keyboard ())
	{
		allegro_message("install keyboard failed");
		return 1;
	}

	if (!al_init_acodec_addon())
	{
		allegro_message("Could not initialize acodec addon. ");
	}

	if (!al_init_ttf_addon())
	{
		allegro_message ("Could not initialize ttf addon. ");
	}

	if (!al_init_primitives_addon())
	{
		allegro_message ("Could not initialize primitives addon. ");
	}

	// set_volume_per_voice (1); //TODO
	if (isSoundInstalled())
	{
		if (!al_install_audio())
		{
			// could not get sound to work
			setSoundInstalled(false);
//			allegro_message ("Could not initialize sound. Sound is turned off.\n%s\n", allegro_error); //TODO
			allegro_message ("Could not initialize sound. Sound is turned off.");
		}
		else
		{
			bool success = al_reserve_samples(16);
			if (!success)
			{
				allegro_message ("Could not reserve samples");
			}
		}
		initSound();
	}

	if (initDisplay() == 0)
	{
		return 1;
	}

#ifdef USE_MOUSE
	if (!al_install_mouse())
	{
		allegro_message ("could not install mouse");
		//set_gfx_mode (GFX_TEXT, 0, 0, 0, 0); //TODO....
//		allegro_exit(); //TODO.. obsolete?
		return 1;
	}
#endif
#ifdef ALLEGRO_ANDROID
	if (!al_install_touch_input())
	{
		allegro_message ("could not install touch input driver");
		//set_gfx_mode (GFX_TEXT, 0, 0, 0, 0); //TODO....
//		allegro_exit(); //TODO.. obsolete?
		return 1;
	}
#endif

	//TODO: include possibility to skip initialisation of subsystems if they are unused...
	//TODO: also make it possible to skip initialising default skin if you are going to load a different skin anyway...
	initSkin();
	return 0;
}

void MainLoop::initSkin()
{
	MASSettings::useVideoMemory = true; // should probably be the default
	theSkin = new Skin("default"); // generate default skin
	cout << "Default Skin initialized" << endl;
}

vector<ALLEGRO_DISPLAY_MODE> getAvailableDisplayModes()
{
	vector<ALLEGRO_DISPLAY_MODE> modes;
	for (int i = 0; i < al_get_num_display_modes(); ++i)
	{
		ALLEGRO_DISPLAY_MODE mode;
		al_get_display_mode(i, &mode);
		modes.push_back(mode);
	}
	return modes;
}

int MainLoop::initDisplay()
{
	bool success = false;

	int display_flags = (isResizableWindow ? ALLEGRO_RESIZABLE : 0) |
		//TODO: should these be compulsory?
		ALLEGRO_PROGRAMMABLE_PIPELINE|ALLEGRO_OPENGL;

	switch (screenMode)
	{
	case FULLSCREEN:
		display_flags |= ALLEGRO_FULLSCREEN;
		break;
	case FULLSCREEN_WINDOW:
		display_flags |= ALLEGRO_FULLSCREEN_WINDOW;
		break;
	case WINDOWED:
		display_flags |= ALLEGRO_WINDOWED;
		break;
	default:
		assert (false);
		break;
	}
	al_set_new_display_flags(display_flags);

	// if prefDisplaySize isn't set explicitly, use game size.
	if (prefDisplaySize.x() < 0) {
		prefDisplaySize = prefGameSize;
	}

	// for allegro fullscreen, try the resolutions available on the system one by one
	if (screenMode == FULLSCREEN)
	{
		// copy resolutions to a vector
		auto modes = getAvailableDisplayModes();

		for (auto mode : modes)
		{
			if (mode.width < prefDisplaySize.x() || mode.height < prefDisplaySize.y())
				continue;

			display = al_create_display(mode.width, mode.height);
			if (display != NULL)
			{
				success = true;
				break;
			}
		}
	}
	else // otherwise just try to get the requested resolution
	{
		display = al_create_display(prefDisplaySize.x(), prefDisplaySize.y());
		if (display != NULL)
		{
			success = true;
		}
	}

	if (!success)
	{
//		allegro_message("Unable initialize graphics module\n%s\n", allegro_error); //TODO
		allegro_message("Unable initialize graphics module");
		return 0;
	}

	// determine if we need to create a stretching bitmap
	if (usagiMode) {
		int dw = al_get_display_width(display);
		int dh = al_get_display_height(display);

		// buffer must be a neat factor of display size,
		// where height must between 440 and 640 or larger.
		if (dh < 960) { /* ..959 */
			w = dw;
			h = dh;
			stretch = false;
		}
		else if (dh < 1280) { /* 960..1279 */
			w = dw / 2;
			h = dh / 2;
			stretch = true;
		}
		else if (dh <= 1760) { /* 1280..1760 */
			w = dw / 3;
			h = dh / 3;
			stretch = true;
		}
		else {
			w = dw / 4;
			h = dh / 4;
			stretch = true;
		}
	}
	else if (useFixedResolution)
	{
		if (al_get_display_width(display) != prefGameSize.x() || al_get_display_height(display) != prefGameSize.y())
			stretch = true;
		// set preferred size, but do not trigger UpdateSize() event yet.
		w = prefGameSize.x();
		h = prefGameSize.y();
	}
	else
	{
		// set w, h equal to the actual display dimension
		// but do not trigger UpdateSize() event yet.
		w = al_get_display_width(display);
		h = al_get_display_height(display);
	}

	al_set_target_backbuffer(display);

	buffer = NULL;

	// use the first resolution as the primary game resolution.
	// not necessarily the same size as the actual game resolution
	if (stretch)
	{
		buffer = al_create_bitmap(w, h);
	}

	if (!buffer)
	{
		buffer = al_get_backbuffer(display);
	}

	if (!buffer)
	{
		allegro_message ("Error creating background buffer");
		return 0;
	}

	if (title != NULL)
	{
		al_set_window_title (display, title);
	}

	return 1;
}

#ifdef USE_MONITORING
void MainLoop::logStartTime(const string &stage)
{
	t2 = Clock::now();
}

void MainLoop::logEndTime(const string &stage)
{
	//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	auto t3 = Clock::now();
	auto us0 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0);
	auto us1 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t1);
	auto us2 = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2);

	if (al_get_timer_count(logicTimer) > 10)
	{
		if (counts.find(stage) == counts.end()) { counts[stage] = 0; }
		if (sums.find(stage) == sums.end()) { sums[stage] = 0; }
		counts[stage] ++;
		sums[stage] += us2.count();

		std::cout << "Cycle #" << al_get_timer_count(logicTimer) << " msecCounter: " << getMsecCounter() << "ms. Chrono: " << us0.count() << " us. Since cycle: " << us1.count() << "us. Phase: " << stage << ", duration: " << us2.count() << " us, avg: " << (sums[stage] / counts[stage]) << endl;
	}
}
#endif

bool MainLoop::checkMessages()
{
	if (rootPane->hasMsg())
	{
		int msg = rootPane->popMsg();
		if (msg == MSG_CLOSE) return true;
		else
		{
			cout << "Warning: received unhandled message: " << msg << endl;
		}
	}
	return false;
}

void MainLoop::pumpMessages()
{
	bool done = false;
	bool need_redraw = true;

	ALLEGRO_EVENT event;
	while (!done)
	{
		al_wait_for_event(equeue, &event);

		switch (event.type)
		{
			case ALLEGRO_EVENT_TIMER: {
#ifdef USE_MONITORING
				t1 = Clock::now();
				logStartTime ("Update");
#endif
				rootPane->update();
				if (checkMessages()) done = true;

				counter++;
#ifdef USE_MONITORING
				logEndTime ("Update");
#endif
				need_redraw = true;
				break;
			}
			case ALLEGRO_EVENT_DISPLAY_CLOSE: {
				done = true;
				break;
			}
			case ALLEGRO_EVENT_DISPLAY_RESIZE: {
				al_acknowledge_resize(event.display.source);
				w = al_get_display_width(display);
				h = al_get_display_height(display);
				UpdateSize();
				need_redraw = true;
				break;
			}
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			case ALLEGRO_EVENT_MOUSE_AXES: {
				adjustMickey(event.mouse.x, event.mouse.y);
				rootPane->handleEvent (event);
				if (checkMessages()) done = true;
				break;
			}
			case ALLEGRO_EVENT_TOUCH_BEGIN:
			case ALLEGRO_EVENT_TOUCH_END:
			case ALLEGRO_EVENT_TOUCH_MOVE:
			case ALLEGRO_EVENT_TOUCH_CANCEL: {
				adjustMickey(event.touch.x, event.touch.y); //TODO: cast needed?
				rootPane->handleEvent (event);
				if (checkMessages()) done = true;
				break;
			}
			case ALLEGRO_EVENT_KEY_UP:
			case ALLEGRO_EVENT_KEY_DOWN:
				rootPane->handleEvent (event);
				if (checkMessages()) done = true;
				break;

			case ALLEGRO_EVENT_KEY_CHAR: {
#ifdef DEBUG
				if (event.keyboard.keycode == ALLEGRO_KEY_F10) {
					done = true;
					break;
				}
#endif
				rootPane->handleEvent (event);
				rootPane->MsgXChar(event.keyboard.keycode, event.keyboard.modifiers);
				if (checkMessages()) done = true;

				break;
			}
		}

		if (need_redraw && al_event_queue_is_empty(equeue))
		{
#ifdef USE_MONITORING
			logStartTime ("Draw");
#endif
			GraphicsContext gc;
			gc.buffer = buffer;
			gc.xofst = 0;
			gc.yofst = 0;

			al_set_target_bitmap(buffer);
			rootPane->draw(gc);
			need_redraw = false;

	        int msecCounter = getMsecCounter();

			if ((msecCounter - frame_counter) > 1000)
			{
				last_fps = frame_count;
				frame_count = 0;
				frame_counter = msecCounter;
			}
			frame_count++;

	        if (fpsOn && getFont())
			{
				draw_textf_with_background(getFont(), WHITE, BLACK, 0, 0,
					  ALLEGRO_ALIGN_LEFT, "fps: %d msec: %07d ", last_fps, msecCounter);
			}

	        if (stretch)
	        {
	    		// I tried using ALLEGRO_TRANSFORM instead of a separate buffer and using al_stretch_blit. But it's actually a lot slower.
 	        	al_set_target_bitmap (al_get_backbuffer(display));
	        	al_draw_scaled_bitmap(buffer, 0, 0, w, h, 0, 0, al_get_display_width(display), al_get_display_height(display), 0);
	        }

			al_flip_display();

#ifdef USE_MONITORING
			logEndTime ("Draw");
#endif
		}
	}
}

void MainLoop::run()
{
#ifdef USE_MONITORING
	t0 = Clock::now();
#endif

	assert (engine); // must have initialised engine by now.
	assert (rootPane);
	rootPane->doLayout (0, 0, w, h);
	rootPane->setFont(engine->getFont()); //TODO: dirty hack, this is needed so popups have a default font.

	if (smokeTest) return;

	// Start the event queue to handle keyboard input and our timer
	equeue = al_create_event_queue();
	al_register_event_source(equeue, al_get_keyboard_event_source());
	al_register_event_source(equeue, al_get_display_event_source(display));
#ifdef USE_MOUSE
	al_register_event_source(equeue, al_get_mouse_event_source());
#endif
#ifdef ALLEGRO_ANDROID
	al_register_event_source(equeue, al_get_touch_input_event_source());
#endif
	logicTimer = al_create_timer(logicIntervalMsec / 1000.0f);
	al_start_timer(logicTimer);
	al_register_event_source(equeue, al_get_timer_event_source(logicTimer));

	// send start event before anything else to component tree.
	ALLEGRO_EVENT event;
	event.type = TWIST_START_EVENT;
	rootPane->handleEvent(event);

	pumpMessages();

	// cleanup
	if (configFilename != NULL)
	{
		al_save_config_file(al_path_cstr(configPath, ALLEGRO_NATIVE_PATH_SEP), config);
	}

	// stop sound - important that this is done before the ALLEGRO_AUDIO_STREAM resources are destroyed
	doneSound();
}

MainLoop::~MainLoop()
{
	// invoke engine destructor, destroy remaining components
	engine = nullptr;
	rootPane = nullptr; // also remove reference from rootPane to engine.

	if (localAppData)
		al_destroy_path(localAppData);

	if (configPath)
		al_destroy_path(configPath);

//	if (buffer) al_destroy_bitmap (buffer); //TODO / not usually necessary?

	if (logicTimer) al_destroy_timer(logicTimer);
	if (equeue) al_destroy_event_queue(equeue);
	if (config) al_destroy_config(config);

	if (display) al_destroy_display(display);
	al_uninstall_audio();
	al_uninstall_system();
}

bool MainLoop::isWindowed() {
	return screenMode == WINDOWED;
}

void MainLoop::toggleWindowed()
{
	if (screenMode != WINDOWED)
		screenMode = WINDOWED;
	else
		screenMode = FULLSCREEN_WINDOW;

	//TODO: reload all fonts... for FULLSCREEN - (not FULLSCREEN_WINDOW)

	al_destroy_display(display);
	initDisplay();

	set_config_int (config, "twist", "windowed", screenMode);
	
	// convert back to video bitmaps
	al_convert_memory_bitmaps();

	UpdateSize();
}

MainLoop &MainLoop::setFixedResolution (bool fixed)
{
	useFixedResolution = fixed;
	return *this;
}

//TODO
/*
MainLoop &MainLoop::setFixedAspectRatio (bool fixed, int x, int y)
{
	useFixedAspectRatio = fixed;
	fixedAspectRatio = Point(x, y);
}
*/

void MainLoop::popup (ComponentPtr popupWindow, ComponentPtr parent)
{
	if (parent)
	{
		rootPane->addWithParent(popupWindow, parent);
	}
	else
	{
		rootPane->add (popupWindow); //TODO: this will lead to duplicate addition
	}
	rootPane->setFocus(popupWindow);
	popupWindow->respawn(); // reset the isAlive flag.
	popupWindow->setVisible(true);
	popupWindow->setAwake(true);
}

void MainLoop::UpdateSize()
{
	if (engine) engine->doLayout(getx(), gety(), getw(), geth());
}

string MainLoop::getUserId() {
	assert(config);
	const char *value = al_get_config_value (config, "twist", "userid");	
	if (value == nullptr) {
		string newUserId = generateRandomId();
		al_set_config_value(config, "twist", "userid", newUserId.c_str());
		return newUserId;
	}
	else {
		return string(value);
	}
}
