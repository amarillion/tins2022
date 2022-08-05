#ifndef SPRITE_H
#define SPRITE_H

#include <allegro5/allegro.h>
#include <list>
#include <map>
#include "spritedata.h"

#include "component.h"
#include "container.h" // viewport

class SpriteLayer;
class Resources;
class ViewPort;
class PlayerControl;

enum SpriteType { ST_BONUS, ST_BULLET, ST_OTHER, ST_PLAYER, ST_ENEMY, ST_VAPOREAL };

//NB: we use mathematical order (start at x-axis, ccw)
// but mapped back from isometric to screen coordinates.
enum Direction { SE = 0, S, SW, W, NW, N, NE, E };

const float INV_SQRT2 = 0.7071;

float radiansFromDir (Direction dir);
Direction dirFromRadians (float radians);

class Sprite
{
	friend class SpriteLayer;
private:

	float x, y, z;	
	bool solid;
	Direction dir; 
	void setParent (SpriteLayer *_parent);
	bool alive;
protected:
	bool gravity = false; // if this is currently affected by the pull of gravity. If yes, will move downward
	bool flying = false; // if this is currently up in the air. If not, z coord will automatically move to surface.
	
	ALLEGRO_COLOR color;
	int sizex;
	int sizey;
	int sizez;
	
	bool blocking;
	bool visible;
	bool shadow;
	
	SpriteLayer* parent;
	PlayerControl *game;
	

public:
	void kill ();

	// lock == reference counting mechanism
	int lock;
	Direction getDir() const { return dir; }
	void setDir(Direction _dir);

	Sprite (PlayerControl *g);
	virtual ~Sprite () {}
	
	// only destroy if no references are held
	bool canDestroy() const { return (!alive && lock == 0); };
	
	bool isAlive() const { return alive; } // if not, scheduled to be killed
	bool isVisible() const { return visible; }
	bool isSolid() const { return solid; } // solid == collision detection
	bool hasShadow() const { return shadow; }
	bool isBlocking() const { return blocking; } // blocking == blocks try_move
			
	virtual void drawShadow (const GraphicsContext &gc);
	virtual void draw(const GraphicsContext &gc);
	
	virtual void setLocation (float nx, float ny, float nz);
	virtual void setSurfaceLocation (float nx, float ny);
	virtual void try_move (float dx, float dy, float dz);
	void setSolid(bool value);
		
	float getx () const { return x; }
	float gety () const { return y; }
	float getz () const { return z; }

	int getSizex () { return sizex; }
	int getSizey () { return sizey; }
	int getSizez () { return sizez; }

	virtual void update();
	virtual SpriteType getType () const = 0;
	virtual void handleCollision(Sprite *s) = 0;
	virtual void handleBlock() {}
	virtual void handleLanding() {}
};

// num cells = CELLNUM * CELLNUM. does not have to match map size.
const int CELLBASE = 5;
const int CELLNUM_SQRT = 1 << CELLBASE; // 32
const int CELLNUM = CELLNUM_SQRT * CELLNUM_SQRT; // 1024
const int CELLMASK = CELLNUM_SQRT - 1; // 31
const int CELLSIZE = 500; // cell width / height, must be larger than largest sprite

class SpriteLayer : public Component {
private:
	int dbgCountTest;
	
	friend class Sprite;
	std::list <Sprite*> sprites;
	
	// each sprite is only in one cell, based on the x,y location (not z)
	// only sprites with parent set are in hashgrid.
	std::list <Sprite*> hashgrid [CELLNUM];
	
	int getCell (Sprite *s);
	
	// turns any cellx, celly coordinate into a flat index that is guaranteed to be 0 <= i < CELLNUM,
	// even if cellx  |celly >> CELLNUM_SQRT
	int getFlatIndex (int cellx, int celly) { return (celly & CELLMASK) * CELLNUM_SQRT  + (cellx & CELLMASK); } 
	
	// NB: removeFromGrid may be called by sprites without checking if they are actually in the grid!
	void removeFromGrid (Sprite *s);
	// NB: addToGrid should only be called by sprites that are not in the grid yet.
	void addToGrid (Sprite *s);

	// helper for update: do collision detection
	void collisionDetection();
	
	PlayerControl *parent;
	// helper for try_move: check if a certain space is occupied by a blocking sprite
	bool checkSpace (Sprite *s, float nx, float ny, float nz);
#ifdef DEBUG	
	void testGrid(); // test consistency of grid, debug helper
#endif
public:
	// used in map generation.
	bool checkSpace_ex (int sizex, int sizey, int sizez, float nx, float ny, float nz);


	SpriteLayer (PlayerControl *e) : sprites(), parent(e)
	{	dbgCountTest = 0; }
	virtual ~SpriteLayer() { killAll(); }
		
	std::list<Sprite*>& getChildren() { return sprites; }

	void add(Sprite *o);
	virtual void update() override;
	void killAll();
	void draw (const GraphicsContext &gc);
	int size() { return sprites.size(); }; // returns # of objects
	
	Sprite * findNearestSprite(float x, float y, std::function<bool(Sprite * s, int distance)> predicate);

	Sprite * getNearestSprite_ex (
		SpriteType st, 
		bool selectSubType,
		SpriteSubType sst, 
		float x, float y,
		bool selectRange,
		float maxRange, 
		bool selectSpread,
		float range_angle,
		float range_spread);
	
	//Sprite *getNearestSprite (SpriteType st, float x, float y);
	Sprite *getNextBunker ();
	
	void testSprites();
//	std::shared_ptr<ViewPort> getViewPort();
};

#endif
