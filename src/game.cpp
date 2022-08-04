#include "game.h"

#include <assert.h>

#include "engine.h"
#include "text.h"
#include "resources.h"
#include "DrawStrategy.h"

using namespace std;

class GameImpl : public Game
{
	Engine *parent;

public:
	GameImpl(Engine *parent) :  
			parent(parent)
	{}

	virtual ~GameImpl() {
	}

	virtual void init() override {
		auto res = parent->getResources();
		add (ClearScreen::build(BLACK).get());
		add (Text::build(CYAN, ALLEGRO_ALIGN_CENTER, "Hello World").center().get());
	}

	virtual void done() override {

	}

};

shared_ptr<Game> Game::newInstance(Engine *parent)
{
	return make_shared<GameImpl>(parent);
}
