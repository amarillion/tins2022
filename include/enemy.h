#pragma once

#include "spriteex.h"
#include <coroutine>

struct ReturnObject {
	struct promise_type {
		promise_type() = default;
		ReturnObject get_return_object() {return {}; } 
		std::suspend_never initial_suspend() {return {};}
		std::suspend_never final_suspend() noexcept {return {};}
		void unhandled_exception(){}
	}; //end of struct promise_type
}; //end of struct ReturnObject

struct Awaiter {
  std::coroutine_handle<> *hp_;
  constexpr bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) { *hp_ = h; }
  constexpr void await_resume() const noexcept {}
};

class Enemy : public SpriteEx
{
	int enemyType;
	int phase;
	int period;
	float hsign;
	float vsign;
	int hittimer;

	int estate;
	double destx;
	double desty;
	int bulletTimer;
	int jumpTimer;
	std::coroutine_handle<> handle;

	void hit(int damage);
public:
	Enemy(Game *, int x, int y, int _type);
	~Enemy() {
		if(handle) { handle.destroy(); }
	}
	virtual void draw(const GraphicsContext &gc);
	virtual void update();
	void spawn(int val);
	void generatorSpawn();
	virtual void onCol (SpriteType st, Sprite *s, int dir);
	virtual void kill();
	void moveTo (double destx, double desty, double speed);
	bool nearDest ();
	void update1();
	void update2();
	void update3();
	void update4();
	void update5();
	void update6();
	ReturnObject update7(std::coroutine_handle<> *continuation_out);
	enum { ELECTRICAT, SLINKYCAT, SPIDERCAT, 
		DRAGONCAT, GENERATOR, TELECAT, ROLLINGCAT, SHARKCAT };
};
