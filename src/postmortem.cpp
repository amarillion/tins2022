#include "postmortem.h"
#include "game.h"
#include <allegro5/allegro.h>
#include <assert.h>
#include "engine.h"
#include "color.h"
#include <math.h>
#include <list>
#include <algorithm>
#include "game.h"
#include "engine.h"
#include "color.h"
#include "level.h"
#include "anim.h"
#include <iostream>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "mainloop.h"
#include "text.h"
#include "bitmap.h"
#include "particle.h"
#include <functional>
#include "easing.h"

using namespace std;
using namespace std::placeholders;

/** 
 * sets a certain value on a certain object, every time update is called.
 * Can be in LOOP, LOOP_MIRROR or SINGLE mode.
 * If in Single mode, emits an event at the end and kills itself
 */
class Tween : public Component {
public:
	enum class Mode { LOOP, MIRROR, SINGLE };
private:
	int counter;
	Mode mode;
	int durationMsec;
	function<double(double)> easingFunc;
	function<void(double)> setter;
	double src;
	double dest;
public:
	Tween(int msec, 
		function<double(double)> easingFunc, 
		function<void(double)> setter, double src, double dest, Mode mode = Mode::SINGLE) :
		counter(0), mode(mode), durationMsec(msec),
		easingFunc(easingFunc), setter(setter), src(src), dest(dest) {
		setVisible(false);
	}

	virtual void update() override {
		counter++;
		int msec = MSEC_FROM_TICKS(counter);
		double pos = 0.0;
		switch(mode) {
		case Mode::SINGLE:
			pos = ((double)msec / (double)durationMsec);
			if (pos > 1.0) {
				pos = 1.0;
				// pushMsg(); //TODO
				kill();
			}
			break;
		case Mode::MIRROR:
			msec %= (durationMsec * 2);
			pos = 1.0 - abs((((double)msec / (double)durationMsec)) - 1.0);
			break;
		case Mode::LOOP:
			msec %= durationMsec;
			pos = (double)msec / (double)durationMsec;
			break;
		}

		pos = easingFunc(pos);
		setter(src + pos * (dest - src));
	}
};

class Counter : public Component {
private:
	int destValue;
	int showValue;
public:
	virtual void update() override;
	virtual void draw(const GraphicsContext &gc) override;

	Counter(int startValue, int destValue);
	static ComponentBuilder<Counter> build(int startValue, int destValue);
};

void Counter::update()
{
	if (showValue < destValue)
	{
		showValue++;
	}
	else if (showValue > destValue)
	{
		showValue--;
	}
}

void Counter::draw(const GraphicsContext &gc)
{
	repr (0, cout);
	al_draw_textf(sfont, BLACK, x + w, y, ALLEGRO_ALIGN_RIGHT, "%i", showValue);
}

Counter::Counter(int _startValue, int _destValue)
{
	destValue = _destValue;
	showValue = _startValue;
}

ComponentBuilder<Counter> Counter::build(int startValue, int destValue)
{
	return ComponentBuilder<Counter>(make_shared<Counter>(startValue, destValue));
}

class AyumiComp : public Component {
private:

	/**
	 * For owned bitmaps, this will ensure that rle is deleted
	 */
	std::shared_ptr<ALLEGRO_BITMAP> rleHolder;

	/**
	 * Actual reference used for drawing in held and non-held situation
	 */
	ALLEGRO_BITMAP *rle;

	double zoom;
	double hx, hy;
	int camx;
	int camy;
public:

	/**
	 * Call this constructor when ownership is transferred to BitmapComp. BitmapComp will ensure al_destroy_bitmap is called.
	 */
	AyumiComp(std::shared_ptr<ALLEGRO_BITMAP> _rle) : rleHolder(_rle), rle(_rle.get()), zoom(1.0), hx(0.0), hy(0.0) {}

	/**
	 * Call this constructor when ownership of bitmap is not transferred
	 */
	AyumiComp(ALLEGRO_BITMAP*  _rle) : rleHolder(), rle(_rle), zoom(1.0), hx(0.0), hy(0.0) {}

	/**
	 * Call this builder when ownership is transferred to BitmapComp. BitmapComp will ensure al_destroy_bitmap is called.
	 */
	static ComponentBuilder<BitmapComp> build(std::shared_ptr<ALLEGRO_BITMAP> _rle);

	/**
	 * Call this builder when ownership of bitmap is not transferred
	 */
	static ComponentBuilder<BitmapComp> build(ALLEGRO_BITMAP *_rle);

	void setZoom(double _zoom) { zoom = _zoom; }
	double getZoom() { return zoom; }

	/**
	 * For use in combination with zoom, you can set a hotspot pixel. to use a center point for the zoom.
	 * The hotspot will remain in position.
	 * */
	void setHotspot (double hxval, double hyval) { hx = hxval; hy = hyval; }
	void setHotX (double hxval) { hx = hxval; }
	void setHotY (double hyval) { hy = hyval; }

	virtual std::string const className() const override { return "BitmapComp"; }

	virtual void draw (const GraphicsContext &gc) override
	{
		int xofst = 0;
		int yofst = 0;

		xofst = getx() + (motion ? motion->getdx(counter) : 0);
		yofst = gety() + (motion ? motion->getdy(counter) : 0);

		xofst += gc.xofst;
		yofst += gc.yofst;

		int origw = al_get_bitmap_width(rle);
		int origh = al_get_bitmap_height(rle);

		int neww = origw * zoom;
		int newh = origh * zoom;

		double invzoom = 1.0 - zoom;
		al_draw_scaled_bitmap(rle,
			0, 0, origw, origh,
			- xofst - (zoom * hx), - yofst - (zoom * hy), neww, newh, 0);
	}

};

void generateHStripes (list<Particle> &particles, int w, int h)
{
	if (particles.size() < 30)
	{
		// new ones at left of screen
		Particle particle;
		particle.alive = true;
		double scale = ((double)random(100)) / 100.0;
		particle.x = -100;
		particle.y = random(h);
		particle.dx = 50.0 * (1 + scale);
		particle.dy = 0;
		particle.z = 5.0; // line length multiplier
		particle.r = 20.0; // line width
		double col = (random(100) / 100.0) * 0.5 + 0.5;
		particle.color = al_map_rgb_f (col, 1.0, col);
		particles.push_back(particle);
	}
}

void generateVStripes (list<Particle> &particles, int w, int h)
{
	if (particles.size() < 30)
	{
		// new ones at left of screen
		Particle particle;
		particle.alive = true;
		double scale = ((double)random(100)) / 100.0;
		particle.x = random(w);
		particle.y = - 100;
		particle.dx = 0;
		particle.dy = + 50.0 * (1 + scale);
		particle.z = 5.0; // line length multiplier
		particle.r = 20.0; // line width
		double col = (random(100) / 100.0) * 0.5 + 0.5;
		particle.color = al_map_rgb_f (col, col, 1.0);
		particles.push_back(particle);
	}
}

void generateStars(std::list<Particle> &particles, int w, int h, ALLEGRO_COLOR color) {
	while (particles.size() < 50)
	{
		Particle particle;
		particle.alive = true;
		double speed = (random(100) / 100.0);
		particle.x = random(w);
		particle.dx = (random(100) / 100.0) - 0.5;
		particle.y = random(h);
		particle.dy = (random(100) / 100.0) - 0.5;
		particle.r = 0;
		particle.dr = ((random(100) / 100.0) * 0.1) - 0.05;
		particle.scale = (random(100) / 100.0) * 0.6 + 0.1;
		particle.color = color;
		particles.push_back(particle);
	}
}

class PostMortemImpl : public PostMortem
{
	std::shared_ptr<Game> game;
	Engine *parent;
	ALLEGRO_BITMAP *star, *heart;
	std::shared_ptr<AyumiComp> doll;
public:
	PostMortemImpl (Engine *engine, std::shared_ptr<Game> &aGame) : game(aGame), parent(engine), doll() {}

	virtual void prepare() override
	{
		killAll();

		setFont (game->gamefont);

		auto bg = ClearScreen::build(YELLOW).get();
		add(bg);

		auto particles = make_shared<Particles>();
		add(particles);

		int x = 320;
		int y = 16;

		add (Text::build(BLACK, ALLEGRO_ALIGN_LEFT, "LEVEL CLEAR").layout(Layout::LEFT_TOP_W_H, x, y, 128, 40).get()); /* game->gamefont, ,  */

	//	al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR); //???
		shared_ptr<ALLEGRO_BITMAP> dollBmp = make_shared_bitmap(al_get_bitmap_width(game->doll[0]), al_get_bitmap_height(game->doll[0]));
		al_set_target_bitmap (dollBmp.get());
		al_clear_to_color (MAGIC_PINK);

		al_draw_bitmap (game->doll[0], 0, 0, 0);
		al_draw_bitmap (game->doll[1], 0, 0, 0); // hair... TODO: optional

		for (int i = 0; i < 4; ++i)
		{
			if (game->clothes[i] >= 0) {
				al_draw_bitmap (clothesData[game->clothes[i]].largebmp, 0, 0, 0);
			}
		}

		doll = make_shared<AyumiComp>(dollBmp);
		doll->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, -200, -200, 0, 0);
		add (doll);

		// step 1.
		bg->setColor(al_map_rgb_f(0.6, 0.9, 0.6));
		setStripeEffect(particles, false);
		doll->setZoom(1.2);
		doll->setHotspot(300.0, 1600.0);
		tween(1900, linear, bind(&AyumiComp::setHotX, doll, _1), 800.0, -100.0);

		// step 2.
		after(2000, [=] {
			bg->setColor(al_map_rgb_f(0.6, 0.6, 0.9));
			setStripeEffect(particles, true);
			doll->setZoom(1.0);
			doll->setHotX(400.0);
			tween(1900, linear, bind(&AyumiComp::setHotY, doll, _1), 1400.0, -300.0);
		});
		// step 3.
		after(4000, [=] {
			if (game->matchBonus) {
				bg->setColor(al_map_rgb_f(0.9, 0.6, 0.6));
				setHeartEffect(particles);
			}
			else {
				bg->setColor(al_map_rgb_f(0.9, 0.9, 0.6));
				setStarEffect(particles);
			}			
			doll->setZoom(1.0);
			doll->setHotspot(300.0, 750.0);
			tween(3000, linear /* TODO: exponential decrease */, bind(&AyumiComp::setZoom, doll, _1), 1.0, 0.25);
		});

		y += 16;

		auto lbl1 = Text::build (BLACK, ALLEGRO_ALIGN_LEFT, 
			game->matchBonus ? "MATCH BONUS RECEIVED" : "no match bonus"
		).layout(Layout::LEFT_TOP_W_H, x, y, 256, 40).get();
		add(lbl1);

		y += 16;

		y += 8;
		add(Text::buildf (BLUE, ALLEGRO_ALIGN_LEFT, "Time score: %d", game->timeScore).layout(Layout::LEFT_TOP_RIGHT_H, x, y, 0, 40).get());
		y += 24;
		add(Text::buildf (BLUE, ALLEGRO_ALIGN_LEFT, "Total score: %d", game->totalScore).layout(Layout::LEFT_TOP_RIGHT_H, x + 16, y, 0, 40).get());
		y += 64;
		add(Text::buildf (BLUE, ALLEGRO_ALIGN_LEFT, "Previous hi score: %d", game->previousHiScore).layout(Layout::LEFT_TOP_RIGHT_H, x + 16, y, 0, 40).get());
		y += 16;
		add(Text::buildf (BLUE, ALLEGRO_ALIGN_LEFT, "Score to beat: %d", levelData[game->lastLevel].scoreToBeat).layout(Layout::LEFT_TOP_RIGHT_H, x + 16, y, 0, 40).get());
		y += 16;

		if (game->newLevelUnlocked)
		{
			add(Text::build (BLACK, ALLEGRO_ALIGN_LEFT, "YOU GOT A STAR, NEW LEVELS UNLOCKED").layout(Layout::LEFT_TOP_RIGHT_H, x, y, 0, 40).get());
		}
		y += 16;
		add(Text::buildf (BLUE, ALLEGRO_ALIGN_LEFT, "Total stars: %d", game->getTotalStars()).layout(Layout::LEFT_TOP_RIGHT_H, x, y, 0, 40).get());
		y += 32;
		add(Text::buildf (BLUE, ALLEGRO_ALIGN_LEFT, "PRESS + OR - to ZOOM").layout(Layout::LEFT_TOP_RIGHT_H, x, y, 0, 40).get());
		y += 16;
		add(Text::buildf (BLUE, ALLEGRO_ALIGN_LEFT, "PRESS ANY OTHER KEY TO CONTINUE").layout(Layout::LEFT_TOP_RIGHT_H, x, y, 0, 40).get());
	}

	virtual void handleEvent (ALLEGRO_EVENT &event) override
	{
		if (event.type != ALLEGRO_EVENT_KEY_CHAR) return;

		int c = event.keyboard.keycode;
		pushMsg(Engine::E_MAINMENU);
	}

	virtual void init() override
	{
		star = parent->getResources()->getBitmap("star");
		heart = parent->getResources()->getBitmap("heart");
	}

	void setStripeEffect(shared_ptr<Particles> particles, bool isVertical) {
		particles->clear();
		particles->setGenerateFunction(
			isVertical
				? std::bind(generateVStripes, _1, MAIN_WIDTH, MAIN_HEIGHT)
				: std::bind(generateHStripes, _1, MAIN_WIDTH, MAIN_HEIGHT)
			);
		particles->setUpdateFunction(linearUpdateFunction);
		particles->setDrawFunction(lineDrawFunction);
	}

	void setStarEffect(shared_ptr<Particles> particles) {
		particles->clear();
		particles->setGenerateFunction(std::bind(generateStars, _1, MAIN_WIDTH, MAIN_HEIGHT, al_map_rgb_f(1.0, 1.0, 0.6)));
		particles->setUpdateFunction(linearUpdateFunction);
		particles->setDrawFunction(std::bind(bitmapDrawFunction, _1, star));
	}

	void setHeartEffect(shared_ptr<Particles> particles) {
		particles->clear();
		particles->setGenerateFunction(std::bind(generateStars, _1, MAIN_WIDTH, MAIN_HEIGHT, al_map_rgb_f(1.0, 0.6, 0.6)));
		particles->setUpdateFunction(linearUpdateFunction);
		particles->setDrawFunction(std::bind(bitmapDrawFunction, _1, heart));
	}

	shared_ptr<Tween> tween(int msec, 
		function<double(double)> easingFunc, 
		function<void(double)> setter, double src, double dest) {
		
		auto obj = make_shared<Tween>(msec, easingFunc, setter, src, dest, Tween::Mode::SINGLE);
		add(obj);
		return obj;
	}

	void after(int msec, function<void()> action) {
		setTimer(TICKS_FROM_MSEC(msec), action);
	}

};

shared_ptr<PostMortem> PostMortem::newInstance(Engine *engine, std::shared_ptr<Game> &aGame) {
	return make_shared<PostMortemImpl>(engine, aGame);
}
