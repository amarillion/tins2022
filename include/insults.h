#ifndef INSULTS_H
#define INSULTS_H

#include "component.h"

const int INSULT_NUM = 7;
extern const char *insults[INSULT_NUM];

const int MIN_INSULT_TIME = 500;
const int INSULT_VARIATION_TIME = 1000;
const int INSULT_DISPLAY_TIME = 300;

class Engine;

class Insults : public Component {
	int tInsult;
	bool active;
	const char *insult; 
	ALLEGRO_BITMAP *fole1, *fole2;
	ALLEGRO_FONT *font;
	int pos;
	Engine &parent;
public:
	Insults (Engine &e);
	virtual ~Insults() {}
	virtual void draw (const GraphicsContext &gc) override;
	virtual void update () override;
	void init ();
};

#endif
