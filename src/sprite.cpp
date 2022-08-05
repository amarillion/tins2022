#include <assert.h>
#include "sprite.h"
#include "color.h"
#include <math.h>
#include "isometric.h"
#include <list>
#include <algorithm>
#include <util.h>
#include <allegro5/allegro_font.h>
#include "mainloop.h"
#include "playercontrol.h"
#include "player.h"

using namespace std;

void Sprite::setDir(Direction _dir) 
{ 
	if (_dir != dir) 
	{ 
		dir = _dir; 
	} 
}

Sprite::Sprite (PlayerControl *g) : game (g)
{
	lock = 0;
	x = 0;
	y = 0;
	z = 0;
	sizex = 50;
	sizey = 50;
	sizez = 50;
	color = RED;
	alive = true;
	visible = true;
	solid = true;
	dir = E;
	parent = NULL;
	shadow = false;
	blocking = false;
}

void Sprite::setParent (SpriteLayer *_parent)
{
	assert (_parent);
	parent = _parent;
	parent->addToGrid (this);
}

void SpriteLayer::killAll()
{
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); ++i)
	{
		delete (*i);
		(*i) = NULL;
	}
	sprites.clear();
	for (int j = 0; j < CELLNUM; ++j) hashgrid[j].clear();
}

void Sprite::update()
{
	assert (parent);
}

void Sprite::kill()
{
	alive = false;
	if (parent) parent->removeFromGrid (this);
}

void SpriteLayer::add(Sprite *o)
{
	sprites.push_back (o);
	o->setParent (this);
}

class MyObjectRemover
{
   public:
	  bool operator()(Sprite *s)
	  {
		 if (s->canDestroy())
		 {
			delete s;
			return 1;
		 }
		 return 0;
	  }
};

void SpriteLayer::collisionDetection()
{
	list<Sprite*>::iterator i;
	dbgCountTest = 0;
	
	// collision detection!	
	list<Sprite*>::iterator j;
	for (i = sprites.begin(); i != sprites.end(); i++)
		if ((*i)->isAlive() && (*i)->isSolid())
		{
			int cellx = (*i)->getx() / CELLSIZE;
			int celly = (*i)->gety() / CELLSIZE;
			for (int celldx = -1; celldx <= 1; ++celldx)
				for (int celldy = -1; celldy <= 1; ++celldy)
				{
					int cell = getFlatIndex(cellx + celldx, celly + celldy);
					for (j = hashgrid[cell].begin(); j != hashgrid[cell].end(); j++)
					{
						if (*j == *i) continue;
						// see if bb interesect
						if ((*j)->isAlive() && (*j)->isSolid())
						{
							dbgCountTest++;
							int x1 = (int)(*i)->getx();
							int y1 = (int)(*i)->gety();
							int z1 = (int)(*i)->getz();
							int dx1 = (*i)->sizex;
							int dy1 = (*i)->sizey;
							int dz1 = (*i)->sizez;
							int x2 = (int)(*j)->getx();
							int y2 = (int)(*j)->gety();
							int z2 = (int)(*j)->getz();
							int dx2 = (*j)->sizex;
							int dy2 = (*j)->sizey;
							int dz2 = (*j)->sizez;
							if(!(
									(x1 >= x2+dx2) || (x2 >= x1+dx1) || 
									(y1 >= y2+dy2) || (y2 >= y1+dy1) ||
									(z1 >= z2+dz2) || (z2 >= z1+dz1) 
								)
								)
							{
								(*i)->handleCollision ((*j));
							}
						}		
					}	
				}
		}
}

void SpriteLayer::update()
{
	testSprites();
	
	//update
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		if ((*i)->isAlive()) (*i)->update();
	}
	
	testSprites();
	collisionDetection();
	
	// remove all that are not alive!
	sprites.remove_if (MyObjectRemover());
	
#ifdef DEBUG
	//testGrid();
#endif
}

static bool sortFunc (Sprite *a, Sprite *b)
{
	float rza = a->getx() + a->gety() + a->getz();
	float rzb = b->getx() + b->gety() + b->getz();
	return rza < rzb;
}

void SpriteLayer::draw (const GraphicsContext &gc)
{
	//testSprites();
	al_set_target_bitmap (gc.buffer);
	
	int xofst = 0;
	int yofst = 0;

	xofst = -gc.xofst;
	yofst = -gc.yofst;

	// convert camera coordinates to isometric
	float isox, isoy;
	parent->getMap()->grid.isoFromCanvas (MAIN_WIDTH / 2 + xofst, MAIN_HEIGHT / 2 + yofst, isox, isoy);
	
	list<Sprite *>visible;
	
	int cellx = isox / CELLSIZE;
	int celly = isoy / CELLSIZE;
	
	// rough measure: just take 3x3 grid around center of screen
	for (int celldx = -1; celldx <= 1; ++celldx)
		for (int celldy = -1; celldy <= 1; ++celldy)
		{
			int cell = getFlatIndex(cellx + celldx, celly + celldy);
			visible.insert (visible.end(), hashgrid[cell].begin(), hashgrid[cell].end());
		}
	
	// sort sprites by z order
	visible.sort (&sortFunc);
	
	list<Sprite*>::iterator i;
	
	for (i = visible.begin(); i != visible.end(); i++)
	{
		if ((*i)->isVisible() && (*i)->isAlive())
		{
			// first draw shadow if this object is floating
			if ((*i)->hasShadow())
			{
				(*i)->drawShadow(gc);
			}
			(*i)->draw(gc);
#ifdef DEBUG
			if (parent->isDebug())
			{
				parent->getMap()->grid.drawWireFrame(gc,
					(*i)->getx(), (*i)->gety(), (*i)->getz(),
					(*i)->sizex, (*i)->sizey, (*i)->sizez, CYAN);
			}
#endif
		}
	}	
	
#ifdef DEBUG
	if (parent->isDebug())
	{
		int max = 0;
		int sum = 0;
		int empty = 0;
		for (int i = 0; i < CELLNUM; ++i)
		{
			int c = hashgrid[i].size();
			if (c > max) max = c;
			if (c == 0) empty++;
			sum += c;
		}	
		float mean = sum / (float)CELLNUM;
		float sumsq = 0;
		for (int i = 0; i < CELLNUM; ++i)
		{
			float c = hashgrid[i].size();
			sumsq += (c - mean)  * (c - mean);
		}
		float stddev = sqrt (sumsq / (CELLNUM - 1));
	}
#endif	
}

void Sprite::drawShadow (const GraphicsContext &gc)
{
	float rx, ry;

	float surfz = game->getMap()->getSurfaceIsoz(getx(), gety());
	game->getMap()->grid.canvasFromIso_f(x, y, surfz, rx, ry);

	ALLEGRO_COLOR shade = al_map_rgba (0, 0, 0, 64);
	int ssize = (int)(sizex * pow (0.995, z - surfz));
	al_draw_filled_ellipse(rx + gc.xofst, ry + gc.yofst + (ssize / 2),
		ssize, ssize / 2, shade);
}

void Sprite::draw(const GraphicsContext &gc)
{
	float rx, ry;
	game->getMap()->grid.canvasFromIso_f(x, y, z, rx, ry);
	
	al_draw_filled_ellipse(rx + gc.xofst, ry + gc.yofst, sizex, sizex / 2, color);
	al_draw_ellipse(rx + gc.xofst, ry + gc.yofst, sizex, sizex / 2, BLACK, 1.0);
	float radians = radiansFromDir(dir);
	float dx = sizex * cos (radians);
	float dy = sizey * sin (radians);

	float rdx, rdy;
	game->getMap()->grid.canvasFromIso_f(dx, dy, 0, rdx, rdy);
	al_draw_line (rx + gc.xofst, ry + gc.yofst, rx + gc.xofst + rdx, ry + gc.yofst + rdy, BLACK, 1.0);
}

// check if space is occupied by a building
// returns false if the space is occupied by something
bool SpriteLayer::checkSpace (Sprite *s, float nx, float ny, float nz)
{
	list<Sprite*>::iterator j;
	
	int cellx = s->getx() / CELLSIZE;
	int celly = s->gety() / CELLSIZE;
	for (int celldx = -1; celldx <= 1; ++celldx)
		for (int celldy = -1; celldy <= 1; ++celldy)
		{
			int cell = getFlatIndex(cellx + celldx, celly + celldy);
			for (j = hashgrid[cell].begin(); j != hashgrid[cell].end(); j++)
			{
				if (*j == s) continue; // skip self
				if ((*j)->isAlive() && (*j)->isSolid() && (*j)->isBlocking())
				{
					
						int x1 = (int)nx;
						int y1 = (int)ny;
						int z1 = (int)nz;
						int dx1 = s->sizex;
						int dy1 = s->sizey;
						int dz1 = s->sizez;
						int x2 = (int)(*j)->getx();
						int y2 = (int)(*j)->gety();
						int z2 = (int)(*j)->getz();
						int dx2 = (*j)->sizex;
						int dy2 = (*j)->sizey;
						int dz2 = (*j)->sizez;
						if(!(
								(x1 >= x2+dx2) || (x2 >= x1+dx1) || 
								(y1 >= y2+dy2) || (y2 >= y1+dy1) ||
								(z1 >= z2+dz2) || (z2 >= z1+dz1) 
							)
							)
						{
							return false;
						}
				}
			}
		}
	return true;
}

// check if space is occupied by a building
// returns false if the space is occupied by something
bool SpriteLayer::checkSpace_ex (int sizex, int sizey, int sizez, float nx, float ny, float nz)
{
	list<Sprite*>::iterator j;

	int cellx = nx / CELLSIZE;
	int celly = ny / CELLSIZE;
	for (int celldx = -1; celldx <= 1; ++celldx)
		for (int celldy = -1; celldy <= 1; ++celldy)
		{
			int cell = getFlatIndex(cellx + celldx, celly + celldy);
			for (j = hashgrid[cell].begin(); j != hashgrid[cell].end(); j++)
			{
				if ((*j)->isAlive() && (*j)->isSolid() && (*j)->isBlocking())
				{

						int x1 = (int)nx;
						int y1 = (int)ny;
						int z1 = (int)nz;
						int dx1 = sizex;
						int dy1 = sizey;
						int dz1 = sizez;
						int x2 = (int)(*j)->getx();
						int y2 = (int)(*j)->gety();
						int z2 = (int)(*j)->getz();
						int dx2 = (*j)->sizex;
						int dy2 = (*j)->sizey;
						int dz2 = (*j)->sizez;
						if(!(
								(x1 >= x2+dx2) || (x2 >= x1+dx1) ||
								(y1 >= y2+dy2) || (y2 >= y1+dy1) ||
								(z1 >= z2+dz2) || (z2 >= z1+dz1)
							)
							)
						{
							return false;
						}
				}
			}
		}
	return true;
}

void Sprite::try_move(float dx, float dy, float dz)
{
	float nx = x + dx;
	float ny = y + dy;
	float nz = z + dz;

	if (nx < 0 || ny < 0 || nx + sizex > game->getMap()->grid.getDimIX() || ny + sizey > game->getMap()->grid.getDimIY())
	{
		return;
		// not blocked, simply fail.
	}

	float surfacez = game->getMap()->getSurfaceIsoz(nx, ny);

	bool blocked = false;

	if (!flying)
	{
		nz = surfacez;
	}
	else if (nz <= surfacez)
	{
		handleLanding();
		blocked = true;
	}

	if (isBlocking())
		if (parent)
			if (!parent->checkSpace (this, nx, ny, nz))
				blocked = true;
	
	if (blocked)
	{
		handleBlock();
	}
	else
	{
		setLocation (nx, ny, nz);
	}
}

void Sprite::setSolid (bool value)
{
	solid = value;
}

void Sprite::setSurfaceLocation (float nx, float ny)
{
	float nz = game->getMap()->getSurfaceIsoz(nx, ny);
	setLocation(nx, ny, nz);
}

void Sprite::setLocation (float nx, float ny, float nz)
{
	if (parent) parent->removeFromGrid(this);
	x = nx;
	y = ny;
	z = nz;
	if (parent) parent->addToGrid (this);
}

/**
 * Find sprite nearest to x, y that matches the predicate.
 */
Sprite * SpriteLayer::findNearestSprite(float x, float y, std::function<bool(Sprite * s, int distance)> predicate)
{
	float minDist;
	Sprite *min = nullptr;

	for (auto i : sprites)
	{
		if (!i->isAlive()) continue;

		float dist, dx, dy;
		dx = x - i->getx();
		dy = y - i->gety();
		dist = dx * dx + dy * dy;

		bool ok = predicate(i, dist);
		if (ok && (min == NULL || dist < minDist))
		{
			min = i;
			minDist = dist;
		}
	}
	return min;
}

/*
Sprite * SpriteLayer::getNearestSprite(SpriteType st, float x, float y)
{
	list<Sprite*>::iterator i;
	
	float minDist;
	Sprite *min = NULL;
	
	// first draw shadows
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		if ((*i)->isAlive() && (*i)->getType() == st)
		{
			float xx = (*i)->getx();
			float yy = (*i)->gety();
			float dist = (x - xx) * (x - xx) + (y - yy) * (y - yy);
			if (min == NULL || dist < minDist)
			{
				min = *i;
				minDist = dist;
			}
		}
	}
	return min;
}
*/
Sprite * SpriteLayer::getNearestSprite_ex (
	SpriteType st, 
	bool selectSubType,
	SpriteSubType sst, 
	float x, float y,
	bool selectRange,
	float maxRange, 
	bool selectSpread,
	float range_angle,
	float range_spread)
{
	list<Sprite*>::iterator i;
	
	float minDist;
	Sprite *min = NULL;
	
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		if ((*i)->isAlive() && (*i)->getType() == st)
		{	
			bool ok = true;
			float dist, dx, dy;
			
			if (selectSubType)
			{
				ActiveSprite *a = dynamic_cast<ActiveSprite*>(*i);
				ok = (a && a->getSubType() == sst);
			}
			if (ok)
			{
				dx = x - (*i)->getx();
				dy = y - (*i)->gety();
				dist = dx * dx + dy * dy;
			}
			if (ok && selectRange)
			{
				if (dist > (maxRange * maxRange))
				{
					ok = false;
				}
			}
			if (ok && selectSpread)
			{
				float vector_angle = atan2 (dx, dy);
				float delta_angle = vector_angle - range_angle;
				normalize_angle (delta_angle);
				if (fabs(delta_angle) > range_spread)
				{
					ok = false;
				}
			}
			if (ok && (min == NULL || dist < minDist))
			{
				min = *i;
				minDist = dist;
			}
		}
	}
	return min;
}

Sprite *SpriteLayer::getNextBunker ()
{
	return 
		getNearestSprite_ex (
			ST_ENEMY, 
			true,
			SST_GIFT,  /* was: SST_MAINBUNKER */
			0, 0,
			false,
			0,
			false,
			0,
			0); 
}

float radiansFromDir (Direction dir)
{
	return dir * M_PI / 4;
}

Direction dirFromRadians (float radians)
{
	int result = int(round(radians * 4 / M_PI)) & 7;
	return (Direction) result;
}

void SpriteLayer::testSprites()
{
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		assert ((*i)->getDir() < 8 && (*i)->getDir() >= 0);
		assert ((*i)->getx() != NAN);
		assert ((*i)->gety() != NAN);
	}
}

// get cell for a given x, y
int SpriteLayer::getCell(Sprite *s)
{
	return getFlatIndex ((int)s->getx() / CELLSIZE, (int)s->gety() / CELLSIZE); 
}

// NB: removeFromGrid may be called by sprites without checking if they are actually in the grid!
void SpriteLayer::removeFromGrid (Sprite *s)
{
	int cell = getCell (s);
	list<Sprite*>::iterator i = find(hashgrid[cell].begin(), hashgrid[cell].end(), s);
	if (i != hashgrid[cell].end())
	{
		hashgrid[cell].erase (i);
	}
}

// NB: addToGrid should only be called by sprites that are not in the grid yet.
void SpriteLayer::addToGrid (Sprite *s)
{
	int cell = getCell (s);
	assert (cell >= 0);
	assert (cell < CELLNUM);
	hashgrid[cell].push_back (s);	
}

#ifdef DEBUG
void SpriteLayer::testGrid()
{
	// check that all sprites are in the correct cell
	
	for (int i = 0; i < CELLNUM; ++i)
	{
		for (list<Sprite*>::iterator j = hashgrid[i].begin(); j != hashgrid[i].end(); ++j)
		{
			int cell = getCell (*j);
			assert (cell == i);
			assert ((*j)->isAlive());
		
			// check for duplicates
			assert (count (hashgrid[i].begin(), hashgrid[i].end(), (*j)) == 1);
		}
	}	
}
#endif
