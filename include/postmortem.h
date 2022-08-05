#ifndef __FASHIONISTA_POSTMORTEM_H__
#define __FASHIONISTA_POSTMORTEM_H__

#include "state.h"

class Game;
class Engine;

class PostMortem : public State {
public:
	static std::shared_ptr<PostMortem> newInstance(Engine *engine, std::shared_ptr<Game> &aGame);
	virtual void init() = 0;
	virtual void prepare() = 0;
};

#endif
