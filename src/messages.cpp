#include "messages.h"
#include "text.h"
#include "motionimpl.h"
#include "easing.h"
#include "widget.h"
#include "animator.h"

using namespace std;

class RainbowGenerator2 : public IColorGenerator
{
private:
	int span;
public:
	RainbowGenerator2(int span);
	virtual ALLEGRO_COLOR getColor(int msec, int index) override;
};

RainbowGenerator2::RainbowGenerator2(int span) : span(span) {};

ALLEGRO_COLOR RainbowGenerator2::getColor(int msec, int index)
{
	return getRainbowColor(index, span);
}

void Messages::showMessage(std::string text, Behavior behavior) {

	// cout << text << endl;
	shared_ptr<Animator<Point>> animator;
	shared_ptr<Text> t;

	const int HEIGHT = 120;
	t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER, text).xywh(0, 0, 640, HEIGHT).get();
	t->setLetterColorGenerator(make_shared<RainbowGenerator2>(12));
	if (behavior == POP_UP) {
		animator = make_shared<Animator<Point>>(
			Point(0, 480), Point(0, 360), 
			160,
			[=](const Point &p){ t->setxy(p.x(), p.y()); },
			[=](){ t->kill(); },
			makeAppearance
		);
	}
	else if (behavior == RIGHT_TO_LEFT) {
		int yco = (480 - HEIGHT) / 2;
		animator = make_shared<Animator<Point>>(
			Point(640, yco), Point(-640, yco), 
			240, // At a speed of 160, I got complaints about the text being unreadable
			[=](const Point &p){ t->setxy(p.x(), p.y()); },
			[=](){ t->kill(); }
		);
	}

	add(t);
	add(animator);

};
