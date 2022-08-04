#pragma once

#include "state.h"

class Engine;

class Game : public State
{
public:
	virtual void init() = 0;
	virtual void done() = 0;
	static std::shared_ptr<Game> newInstance(Engine *parent);
};
