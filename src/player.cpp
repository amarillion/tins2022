#include "player.h"
#include "color.h"
#include "isometric.h"
#include <math.h>
#include "util.h"
#include "viewport.h"
#include "playercontrol.h"
#include "anim.h"
#include "engine.h"
#include <allegro5/allegro_audio.h>
#include "resources.h"

using namespace std;

void ActiveSprite::handleBlock()
{
	speed = 0.0;
}

void Player::handleBlock()
{
	if (getSubType() == SST_CHOPPER)
	{
		// chopper takes damage if flying against a building at high speed
		if (speed > 3.0)
		{
			decreaseHealth(10);
		}
	}
	ActiveSprite::handleBlock(); // stop	
}

ActiveSprite::ActiveSprite (PlayerControl *g, SpriteSubType _st) : Sprite (g), state (0), dead (false), speed (0), current (NULL), target(NULL), subType (_st), health (100)
{
	nozzlex = 0;
	nozzley = 0;
	nozzlez = 0;
	gravity = false;
	flying = false;
	dz = 0;
	tKill = 0;
		
	SpriteData *data = &spriteData[_st];
	// solid = data->solid;
	shadow = data->shadow;
	blocking = data->blocking;
	has_state50 = data->has_state50;
	has_rubble = data->has_rubble;
	
	if (data->anim != NULL)
	{
		current = data->anim;
		sizex = current->sizex;
		sizey = current->sizey;
		sizez = current->sizez;
	}	

	if (subType == SST_CHOPPER)
	{
		flying = true;
	}
	else
	{
		flying = false;
	}
}

void ActiveSprite::setState(int value)
{
	if (state != value && value >= 0 && value < current->getNumStates())
	{
		state = value;
	}
}

/**
	Set target and adjust locking of target
	param may be NULL (stops targeting)
*/
void ActiveSprite::setTarget(Sprite *_target)
{
	if (target != _target)
	{
		if (_target == NULL)
		{
			target->lock--;
		}
		target = _target;
		if (target != NULL)
		{
			target->lock++;
		}
	}
}

void Bullet::setSource (Sprite *_source)
{
	if (source != _source)
	{
		if (_source == NULL)
		{
			source->lock--;
		}
		source = _source;
		if (source != NULL)
		{
			source->lock++;
		}
	}
}

Bullet::Bullet(PlayerControl *g, Sprite *_source, Sprite *_target, Direction startDir, float startSpeed) : ActiveSprite (g, SST_MISSILE)
{
	source = NULL;
	setSource (_source);
	tLive = 0;
	setSolid(true);
	color = RED;
	dx = 0;
	dy = 0;
	dz = 0;
	setTarget (_target);
	angle = (startDir * M_PI / 4);
	speed = startSpeed;
	maxLive = MISSILE_LIVE_TIME;
	damage = 15;
	flying = true;
}

Bullet::Bullet(PlayerControl *g, Sprite *_source, float _dx, float _dy, float _dz) : ActiveSprite (g, SST_BULLET)
{
	source = NULL;
	setSource (_source);
	tLive = 0;
	setSolid (true);
	color = WHITE;
	dx = _dx;
	dy = _dy;
	dz = _dz;
	setTarget (NULL);
	angle = 0;
	speed = 0;
	maxLive = LIVE_TIME;
	damage = 5;
	flying = true;
}

void Bullet::update()
{
	switch (getSubType())
	{
		case SST_BULLET:
				try_move (dx, dy, dz);
			break;
		case SST_MISSILE:
			{
				float dest, vector_angle;
				delta_vector(this, target, dest, vector_angle);
				
				float delta_angle = angle - vector_angle + M_PI;
				normalize_angle (delta_angle);
				if (delta_angle < -0.1) { angle -= turning_speed; }
				if (delta_angle > 0.1) { angle += turning_speed; }
				
				float ddz = getz() - target->getz();
				
				if (ddz < -1.0)
				{
					dz = 1.0;
				}
				if (ddz > 1.0 && getz() > 5.0)
				{
					dz = -1.0;
				}
				
				if (speed < 16.0) speed += 0.3;
				
				dx = speed * cos (angle);
				dy = speed * sin (angle);
				
				setDir(dirFromRadians(angle));
				try_move (dx, dy, dz);
			}
			break;
		default:
			assert (false);
			break;
	}
	if (++tLive > maxLive || getz() == 0) kill();
}

void Bullet::handleLanding()
{
	//TODO: show particle effect
	kill();
	if (getSubType() == SST_BULLET)
	{
		explode (0);
	}
	else
	{
		explode (1);
	}
}

void Bullet::handleCollision(Sprite * s)
{
	if (s == source) { return; } // can't hit yourself
	if (s->isBlocking())
	{
		kill();
		if (getSubType() == SST_BULLET)
		{
			explode (0);
		}
		else
		{	
			explode (1);
		}
	}
}

Explosion::Explosion(PlayerControl *g, int size) : Sprite (g)
{
	setSolid (false);
	color = YELLOW;
	maxLive = size;
	sizex = 1;
	sizey = 1;
	sizez = 1;
	tLive = 0;
}

void Explosion::draw(const GraphicsContext &gc)
{
	float rx, ry;
	game->getMap()->grid.canvasFromIso_f(getx(), gety(), getz(), rx, ry);
	
	al_set_target_bitmap (gc.buffer);
	
	al_draw_filled_circle (rx + gc.xofst, ry + gc.yofst, sizex, al_map_rgba (255, 0, 0, 128));
	al_draw_filled_circle (rx + gc.xofst, ry + gc.yofst, sizex * 2 / 3, al_map_rgba (255, 255, 0, 128));
}

void Explosion::update()
{
	sizex++;
	if (++tLive > maxLive)
	{
		kill();
	}
}

Player::Player(PlayerControl *g, SpriteSubType st) : ActiveSprite (g, st)
{
	accelerate = false;
	setSolid (true);
	color = GREEN;
	nozzlez = 5;
}

Enemy::Enemy(PlayerControl *g, SpriteSubType st) : ActiveSprite (g, st)
{
	tEvasive = 0;
	tReload = 0;
	setSolid (true);
	color = BLUE;
	
	target = NULL;
	switch (st)
	{
		case SST_ENEMYTANK:
			viewRange = 500;
			mobile = true;
			nozzlez = 5;
			break;
		case SST_TURRET:
			viewRange = 200;
			mobile = false;
			nozzlez = 80;
			break;
		case SST_BUNKER:
			viewRange = 200;
			mobile = false;
			nozzlez = 5;
			break;
		case SST_SAM:
			viewRange = 200;
			mobile = false;
			nozzlez = 80;
			break;
		case SST_MAINBUNKER:
			viewRange = 200;
			mobile = false;
			nozzlez = 5;
			break;
		case SST_GIFT:
			viewRange = 0;
			mobile = false;
			nozzlez = 5;
			break;
		default:
			assert (false);
			break;
	}
}

void Enemy::handleBlock()
{
	setDir ((Direction)(rand() % DIRNUM));
	tEvasive = 50;
}
 
void Enemy::update()
{
	if (getTarget() && !getTarget()->isAlive()) setTarget (NULL);
	
	if (!dead)
	{
		if (tEvasive > 0)
		{
			tEvasive--;
		}
		else
		{
			
			if (!getTarget())
			{
				if (getSubType() == SST_SAM)
				{
					setTarget (game->getChopper());
				}
				else
				{
					setTarget (game->getNearestPlayer(getx(), gety(), viewRange));
				}
			}
			
		}
		
		if (getTarget() && tEvasive == 0)
		{
			float angle, dist;
			delta_vector(this, target, dist, angle);
			
			setDir (dirFromRadians(angle));
			if (getSubType() == SST_ENEMYTANK)
			{
				if (dist > (200.0 * 200.0))
				{
					speed += 0.2;
					if (speed > 4.0) speed = 4.0;		
				}
			}
			
			if (dist > (viewRange * viewRange))
				setTarget (NULL);
		}
		
		if(getSubType() == SST_ENEMYTANK && tEvasive > 0)
		{
			speed += 0.2;
			if (speed > 2.0) speed = 2.0;		
		}
	}
				
	if (getSubType() == SST_ENEMYTANK)
	{
		if (speed < 0.1) speed = 0;
		float angle = radiansFromDir(getDir());
		float dx = speed * cos(angle);
		float dy = speed * sin(angle);
		try_move (dx, dy, 0);
		speed -= 0.1; // decelerate
	}
	
	if (tReload > 0) { tReload--; }

	if (!dead && tReload <= 0)
	{
		float radians = radiansFromDir (getDir());

		if (getTarget())
		{
			ActiveSprite *asTarget = dynamic_cast<ActiveSprite*>(getTarget());
			if (asTarget != NULL && !asTarget->isDead())
			{
				switch (getSubType())
				{
				case SST_SAM:
					tReload = RELOAD_TIME;
					fireMissile();
					break;
				case SST_ENEMYTANK:
				case SST_TURRET:
					tReload = RELOAD_TIME;
					fireBullet4();
					break;
				case SST_GIFT:
					// don't fire any bullets
					break;
				default: // bunkers etc.
					tReload = RELOAD_TIME;
					fireBullet2 ();
					break;
				}
			}
		}
	}
		
	ActiveSprite::update();
}

void Enemy::handleCollision(Sprite * s)
{
	if (s->getType() == ST_BULLET)
	{
		Bullet *b = dynamic_cast<Bullet *>(s);
		assert (b); // must be a bullet
		if (b->getSource()->getType() == ST_ENEMY)
		{
			return; // ignore self-hit
		}
		decreaseHealth(10);		
	}
}

void Solid::handleCollision(Sprite * s)
{
	// this only goes for tall / house
	if (!(getSubType() == SST_HOUSE || getSubType() == SST_TALL
		|| getSubType() == SST_LOGCABIN)) return;
	
	if (s->getType() == ST_BULLET)
	{
		Bullet *b = dynamic_cast<Bullet *>(s);
		assert (b); // must be a bullet
		if (b->getSource()->getType() == ST_ENEMY)
		{
			return; // ignore self-hit
		}
		
		decreaseHealth(10);
	}
}


Vaporeal::Vaporeal(PlayerControl *g, SpriteSubType st, float _dx, float _dy, float _dz) : ActiveSprite (g, st)
{
	setSolid (false);
	color = WHITE;
	dx = _dx;
	dy = _dy;
	dz = _dz;
	tLive = 0;
}

void Vaporeal::update()
{
	setLocation (getx() + dx, gety() + dy, getz() + dz);

	if (++tLive > OUTRO_TIME) kill();
}

Bonus::Bonus(PlayerControl *g, SpriteSubType st) : ActiveSprite (g, st)
{
	setSolid (true);
	color = WHITE;
}

Solid::Solid(PlayerControl *g, SpriteSubType st) : ActiveSprite (g, st)
{
}

// chopper
void ActiveSprite::fireBullet()
{
	MainLoop::getMainLoop()->playSample(shoot1);
	float radians = radiansFromDir(getDir());
	float bulletSpeed = speed + 6.0;
	Bullet *b = new Bullet(game, this, bulletSpeed * cos(radians), bulletSpeed * sin(radians), -(getz() / 30));
	b->setLocation(getx(), gety(), getz() + nozzlez);
	parent->add(b);
}

// player tank
void ActiveSprite::fireBullet3()
{
	MainLoop::getMainLoop()->playSample(shoot2);
	float radians = radiansFromDir(getDir());
	float bulletSpeed = speed + 6.0;
	Bullet *b = new Bullet(game, this, bulletSpeed * cos(radians), bulletSpeed * sin(radians), 0);
	b->setLocation(getx(), gety(), getz() + nozzlez);
	parent->add(b);
}

// enemy bunker
void ActiveSprite::fireBullet2()
{
	MainLoop::getMainLoop()->playSample(shoot3);

	float dx = target->getx() - getx();
	float dy = target->gety() - gety();
	float dz = target->getz() - getz();
	float dist = sqrt ((dx * dx) + (dy * dy));
	
	if (dist == 0) return; // div by 0 prevention
	
	float bulletSpeed = 6.0;
	Bullet *b = new Bullet(game, this, dx / dist * bulletSpeed, dy / dist * bulletSpeed, 0);
	
	b->setLocation(getx(), gety(), getz() + nozzlez);
	parent->add(b);
}

// enemy turret
void ActiveSprite::fireBullet4()
{
	MainLoop::getMainLoop()->playSample(shoot3);

	float dx = target->getx() - getx();
	float dy = target->gety() - gety();
	float dz = target->getz() - getz() - nozzlez;
	float dist = sqrt ((dx * dx) + (dy * dy) + (dz * dz));
	
	if (dist == 0) return; // div by 0 prevention
	
	float bulletSpeed = 6.0;
	Bullet *b = new Bullet(game, this, dx / dist * bulletSpeed, dy / dist * bulletSpeed, dz / dist * bulletSpeed);
	
	b->setLocation(getx(), gety(), getz() + nozzlez);
	parent->add(b);
}

void ActiveSprite::fireMissile()
{
	MainLoop::getMainLoop()->playSample(shoot4);
	Bullet *b = new Bullet(game, this, getTarget(), getDir(), speed);
	b->setLocation(getx(), gety(), getz() + nozzlez);
	parent->add(b);
}

void ActiveSprite::explode(int scale)
{
	bool tremble = false;
	float ampl = 0.0;
	int size = 5;
	
	switch (scale)
	{
		case 0: // bullet impact
			tremble = false;
			size = 5;
			break;
		case 1: // missile impact
			tremble = false;
			size = 10;
			break;
		case 2: // bunker, house or tall building destroyed
			MainLoop::getMainLoop()->playSample(expl1);
			tremble = true;
			ampl = 10.0;
			size = 50;
			break;
		case 3: // player unit destroyed
			MainLoop::getMainLoop()->playSample(lose);
			tremble = true;
			ampl = 3.0;
			size = 30;
			break;
		case 5: // mainbunker destroyed
			MainLoop::getMainLoop()->playSample(expl2);
			tremble = true;
			size = 70;
			ampl = 20.0;
			break;
	}
	


	Explosion *e = new Explosion (game, size);
	e->setLocation(getx(), gety(), getz());
	parent->add(e);
	if (tremble)
	{
		game->getViewPort()->tremble(ampl);
	}
}


ALLEGRO_SAMPLE *ActiveSprite::expl1 = NULL;
ALLEGRO_SAMPLE *ActiveSprite::expl2 = NULL;
ALLEGRO_SAMPLE *ActiveSprite::shoot1 = NULL;
ALLEGRO_SAMPLE *ActiveSprite::shoot2 = NULL;
ALLEGRO_SAMPLE *ActiveSprite::shoot3 = NULL;
ALLEGRO_SAMPLE *ActiveSprite::shoot4 = NULL;
ALLEGRO_SAMPLE *ActiveSprite::lose = NULL;
ALLEGRO_SAMPLE *ActiveSprite::gift = NULL;
ALLEGRO_SAMPLE *ActiveSprite::hurt = NULL;

void ActiveSprite::init(shared_ptr<Resources> res)
{
	for (int i = 0; i < SPRITE_NUM; ++i)
	{
		if (spriteData[i].animName != NULL)
		{
			spriteData[i].anim = res->getAnim (spriteData[i].animName);
		}
	}

	expl1 = res->getSample("explode1");
	expl2 = res->getSample("explode2");
	shoot1 = res->getSample("shoot1");
	shoot2 = res->getSample("shoot2");
	shoot3 = res->getSample("shoot3");
	shoot4 = res->getSample("rocket1");
	hurt = res->getSample("hurt1");
	gift = res->getSample("gift");
	lose = res->getSample("lose");
}

void ActiveSprite::draw(const GraphicsContext &gc)
{
	al_set_target_bitmap(gc.buffer);
	if (current)
	{
		float rx, ry;
		game->getMap()->grid.canvasFromIso_f(getx(), gety(), getz(), rx, ry);

		current->drawFrame (state, getDir(), MainLoop::getMainLoop()->getMsecCounter(), rx + gc.xofst, ry + gc.yofst);
		
		if (game->isDebug())
		{
			Sprite *t = getTarget();
			if (t)
			{
				float r2x, r2y;
				game->getMap()->grid.canvasFromIso_f(t->getx(), t->gety(), t->getz(), r2x, r2y);
				al_draw_line (rx + gc.xofst, ry + gc.yofst, r2x + gc.xofst, r2y + gc.yofst, YELLOW, 1.0);
			}
		}
	}
	else
	{
		Sprite::draw (gc);
	}
}

void Player::setAccelerate()
{
	if (dead) return; // already dead
	accelerate = true;
}

void Player::update()
{	
	if (accelerate)
	{
		speed += PLAYER_SPEED_INC;
		if (speed > PLAYER_MAX_SPEED) speed = PLAYER_MAX_SPEED;
	}
	else
	{
		speed -= PLAYER_SPEED_INC;
		if (speed < 0) speed = 0;
	}
	accelerate = false; // needs to be activated constantly
	
	float radians = radiansFromDir(getDir());
	
	float dx = speed * cos (radians);
	float dy = speed * sin (radians);

	double surfacez = game->getMap()->getSurfaceIsoz(getx(), gety());
	if (gravity && getz() > surfacez)
	{
		dz -= 0.1; //TODO: check for touch down and turn off flying
	}
	
	ActiveSprite::update();
	
	try_move (dx, dy, dz);
}

void Player::decreaseHealth(int amount)
{
	game->takeDamage (amount);
	ActiveSprite::decreaseHealth(amount);
}

void ActiveSprite::decreaseHealth(int amount)
{
	if (dead) return; // already dead
	
	// if crossing the border of 50
	if (has_state50 && health - amount < 50 && health >= 50)
	{
		assert (current->getNumStates() > 1);
		setState(1); // set damaged state
	}
		
	health -= amount;

	if (getType() == ST_PLAYER)
	{
		MainLoop::getMainLoop()->playSample(hurt);
	}

	if (health < 0)
	{	
		health = 0;
		if (subType == SST_CHOPPER) turnOnGravity();
		// TODO: ugly if statement
		if (has_rubble)
		{
			tKill = 100;
			assert (current->getNumStates() > 1);
			setState (1); // set rubble state
			dead = true;
		}
		else
		{
			kill();
		}
		if (/* subType == SST_MAINBUNKER */ subType == SST_GIFT)
		{
			game->decreaseTargets();
			explode(5);
			MainLoop::getMainLoop()->playSample(gift);
			Vaporeal *s = new Vaporeal(game, (SpriteSubType)(SST_GIFT1 + game->getCurrentLevel()), 0, 0, 1);
			s->setLocation(getx(), gety(), getz());
			parent->add (s);
		}
		else if (getType() == ST_PLAYER)
		{
			explode(3);
		}
		else
		{
			explode(2);
		}
	}
}

void ActiveSprite::update()
{
	if (dead)
	{
		tKill--;
		if (tKill <= 0)
		{
			if (getType() == ST_PLAYER)
				game->playerKilled(
					dynamic_cast<Player*>(this));
			kill();
		}
	}
}

void Player::lift (float dz)
{
	assert (getSubType() == SST_CHOPPER); // only chopper can move up / down
	
	if (dead) return; // don't move, we're dead
	
	try_move(0, 0, dz);
}

void Player::handleCollision(Sprite * s)
{
	if (s->getType() == ST_BULLET)
	{
		Bullet *b = dynamic_cast<Bullet *>(s);
		assert (b); // must be a bullet
		if (b->getSource() == this)
		{
			return; // ignore self-hit
		}
		decreaseHealth (b->getDamage());
	}
}

float sq_dist (Sprite *src, Sprite *dest)
{
	float dx = dest->getx() - src->getx();
	float dy = dest->gety() - src->gety();
	return (dx * dx) + (dy * dy);
}

void delta_vector (Sprite *src, Sprite *dest, float &sq_dist, float &radians)
{
	float dx = dest->getx() - src->getx();
	float dy = dest->gety() - src->gety();
	sq_dist = (dx * dx) + (dy * dy);
	radians = atan2 (dy, dx);
}

void normalize_angle (float &angle)
{
	while (angle < -M_PI) { angle += 2 * M_PI; }
	while (angle > M_PI) { angle -= 2 * M_PI; }
}
