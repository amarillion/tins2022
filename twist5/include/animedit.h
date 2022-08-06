#pragma once

#include "state.h"

class Engine;

class AnimEditor : public State
{
public:
	virtual void init() = 0;
	virtual void done() = 0;
	static std::shared_ptr<AnimEditor> newInstance(Engine *parent);
};
