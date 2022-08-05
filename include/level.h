#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <list>
#include <map>
#include "tegel5.h"
#include "resources.h"
#include "object.h"
#include "dom.h"

class Object;
class Objects;
class Door;

const int TILE_SIZE = 32;

enum RoomFlags {
	INIT_KEY = 0x01, 
	INIT_BONUS = 0x02, 
	INIT_BANANA = 0x04,

	INIT_P1_START = 0x10,
	INIT_P2_START = 0x20,
	
	INIT_DOOR_N = 0x100,
	INIT_DOOR_E = 0x200,
	INIT_DOOR_S = 0x400,
	INIT_DOOR_W = 0x800,

	INIT_LOCK_N = 0x1000,
	INIT_LOCK_E = 0x2000,
	INIT_LOCK_S = 0x4000,
	INIT_LOCK_W = 0x8000,

	INIT_TELEPORTER = 0x10000
};

class ObjectInfo
{
	public:
		enum ObjectInfoType {DOOR, TELEPORT, MONSTER, BANANA, PLAYER};
		ObjectInfoType type;
		
		int doorDir; // only for DOOR
		int x;
		int y;
		int monsterType; // only for MONSTER
		int pi; // only for PLAYER
};

class RoomInfo
{
	public:
		RoomInfo() : objectInfo(), map(NULL), 
			up(false), down(false), left(false), right(false), 
			teleport(false), bananas(0), playerStart(false) {}
		std::vector <ObjectInfo> objectInfo;
		std::string name;
		TEG_MAP *map;
		bool up;
		bool down;
		bool left;
		bool right;
		bool teleport;
		int bananas;
		bool playerStart;
};

// container for all rooms
class RoomSet
{
	private:
	public:
		std::vector<RoomInfo> rooms;		
		static RoomSet *init (std::shared_ptr<Resources> res);
		RoomInfo *findRoom (bool up, bool down, bool left, bool right, bool teleport);
};

class Room
{
		RoomInfo *roomInfo;
		Objects *objects; // global object container
		
		Door *doors[4]; // list of Doors in this room
 		Door *teleport;

		int initFlags; // room initialization flags...
		int bananaCount; 
	public:		
		Room(Objects *o, RoomInfo *ri, int monsterHp, int initFlags = -1);
		TEG_MAP *map;
		int getStartX (int pi); // get start location of player pi
		int getStartY (int pi);
		void linkDoor (Room *otherRoom, int dir, bool reverse = true, bool locked = false);
		void linkTeleport (Room *otherRoom, bool reverse = true);
		void lockDoor (int dir);
		int getBananaCount() { return bananaCount; }
		int getFlags() { return initFlags; }
		void getPlayerLocation(int player, int *x, int *y);
};

class Level
{
	public:
		std::vector <Room *> rooms;
		Room *getStartRoom(int player);
		
		Level() : rooms() {}
		~Level();
		int getBananaCount();
		// ALLEGRO_BITMAP* getMinimap();
};

Level *createTestLevel(RoomSet *roomSet, std::shared_ptr<Resources> resources, Objects *objects, int monsterHp);
Level *createLevel(RoomSet *roomSet, Objects *objects, unsigned int numRooms, int monsterHp);

#endif
