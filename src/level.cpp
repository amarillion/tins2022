#include <assert.h>
#include "level.h"
#include <math.h>

#include "door.h"
#include "monster.h"
#include "player.h"
#include "util.h"
#include "tilemap.h"
#include "strutil.h"

#include "levelGen.h"
#include <algorithm>

using namespace std;
using namespace xdom;

Level *createTestLevel(RoomSet *roomSet, shared_ptr<Resources> resources, Objects *objects, int monsterHp)
{
	Level *level = new Level();

	Room *temp[15];
	for (int i = 0; i < 14; ++i) { temp[i] = NULL; }
	temp[0] = new Room(objects, roomSet->findRoom(true, true, true, true, false), monsterHp);
	temp[1] = new Room(objects, roomSet->findRoom(true, true, false, true, false), monsterHp);
	temp[2] = new Room(objects, roomSet->findRoom(false, true, false, false, true), monsterHp);
	temp[3] = new Room(objects, roomSet->findRoom(false, false, true, true, false), monsterHp);
	temp[4] = new Room(objects, roomSet->findRoom(false, false, true, false, false), monsterHp);
	temp[5] = new Room(objects, roomSet->findRoom(true, true, false, false, false), monsterHp);
	temp[6] = new Room(objects, roomSet->findRoom(true, false, false, false, true), monsterHp);
	temp[7] = new Room(objects, roomSet->findRoom(false, true, true, false, false), monsterHp);
	temp[8] = new Room(objects, roomSet->findRoom(true, false, false, true, true), monsterHp);
	temp[9] = new Room(objects, roomSet->findRoom(false, false, true, false, false), monsterHp);
	temp[10] = new Room(objects, roomSet->findRoom(true, false, true, true, true), monsterHp);
	temp[11] = new Room(objects, roomSet->findRoom(true, false, false, true, false), monsterHp);
	temp[12] = new Room(objects, roomSet->findRoom(false, true, false, true, false), monsterHp);
	temp[13] = new Room(objects, roomSet->findRoom(false, true, true, false, false), monsterHp);
	for (int i = 0; i < 14; ++i) { assert (temp[i]); level->rooms.push_back (temp[i]); }
	
	temp[0]->linkDoor (temp[1], 0);
	temp[0]->linkDoor (temp[5], 1);
	temp[0]->linkDoor (temp[10], 2);
	temp[0]->linkDoor (temp[7], 3);
	temp[1]->linkDoor (temp[2], 0);
	temp[1]->linkDoor (temp[3], 3);
	temp[2]->linkTeleport (temp[6]);
	temp[3]->linkDoor (temp[4], 3);
	temp[5]->linkDoor (temp[6], 1);
	temp[7]->linkDoor (temp[8], 1);
	temp[8]->linkDoor (temp[9], 3);
	temp[8]->linkTeleport (temp[10]);
	temp[10]->linkDoor (temp[13], 0);
	temp[10]->linkDoor (temp[11], 2);
	temp[11]->linkDoor (temp[12], 0);
	temp[12]->linkDoor (temp[13], 3);

	// test level
	// 	level->rooms.push_back (new Room(objects, roomSet->findRoom(true, true, true, true, false), monsterHp));
	
	return level;
}

RoomSet *RoomSet::init (shared_ptr<Resources> res) {
	RoomSet *result = new RoomSet();
	
	vector<string> roomNames { 
		"cross", "ldoor", "bdoor", "rdoor", 
		"tdoor", "bar1", "bar2", "tee1", 
		"tee2", "tee3", "tee4", "bend1", 
		"bend2", "bend3", "bend4", "enclosed" 
	};

	for (string name : roomNames) {
		RoomInfo ri;
		/* create two rooms, one with and one without teleporter */
		ri.map = res->getJsonMap(name)->map;
		ri.name = name;
		int playerInitialisation = 0;

		for (int x = 0; x < ri.map->w; ++x)
			for (int y = 0; y < ri.map->h; ++y)
			{
				// get object data from layer 2.
				int tile = teg_mapget(ri.map, 2, x, y);
				int flag = ri.map->tilelist->tiles[tile].flags;

				if (tile >= 0 && flag >= 100)
				{
					ObjectInfo oi;
					oi.x = x;
					oi.y = y;
					switch (flag)
					{
					case 100:
						oi.type = ObjectInfo::DOOR;
						oi.doorDir = 0;
						ri.up = true;
						break;
					case 101:
						oi.type = ObjectInfo::DOOR;
						oi.doorDir = 3;
						ri.right = true;
						break;
					case 102:
						oi.type = ObjectInfo::DOOR;
						oi.doorDir = 1;
						ri.down = true;
						break;
					case 103:
						oi.type = ObjectInfo::DOOR;
						oi.doorDir = 2;
						ri.left = true;
						break;
					case 104:
						oi.type = ObjectInfo::BANANA;
						ri.bananas++;
						break;
					case 105: // green bug
						oi.type = ObjectInfo::MONSTER;
						oi.monsterType = GREEN_BUG;
						break;
					case 106: // blue bell
						oi.type = ObjectInfo::MONSTER;
						oi.monsterType = BLUE_BELL;
						break;
					case 107: // orange bell
						oi.type = ObjectInfo::MONSTER;
						oi.monsterType = ORANGE_BELL;
						break;
					case 108: // blue bug
						oi.type = ObjectInfo::MONSTER;
						oi.monsterType = BLUE_BUG;
						break;
					case 112: // organge flower
						oi.type = ObjectInfo::MONSTER;
						oi.monsterType = ORANGE_FLOWER;
						break;
					case 113: // blue flower
						oi.type = ObjectInfo::MONSTER;
						oi.monsterType = BLUE_FLOWER;
						break;
					case 114: // red bug
						oi.type = ObjectInfo::MONSTER;
						oi.monsterType = RED_BUG;
						break;
					case 109:
						oi.type = ObjectInfo::TELEPORT;
						break;
					case 110:
						oi.type = ObjectInfo::PLAYER;
						oi.pi = 0;
						ri.playerStart = true;
						playerInitialisation |= 1;
						break;
					case 111:
						oi.type = ObjectInfo::PLAYER;
						oi.pi = 1;
						ri.playerStart = true;
						playerInitialisation |= 2;
						break;
					default:
						cerr << string_format ("Found flag %i for tile %i at (%i, %i) in map %s\n", flag, tile, x, y,  ri.name);
						assert (false); // wrong type
						break;
					}
					ri.objectInfo.push_back(oi);
				}

		}
		
		// each room should have exactly three placeholders for bananas
		if (ri.bananas < 3) {
			cerr << string_format("Missing placeholders in %s\n", name.c_str());
			assert(false);
		}
		if (playerInitialisation != 0x3) {
			cerr << string_format("Missing player initialisation in %s\n", name.c_str());
			assert(false);
		}
		
		result->rooms.push_back (ri);
	}
	return result;
}

RoomInfo *RoomSet::findRoom (bool up, bool down, bool left, bool right, bool teleport)
{
	RoomInfo *result = nullptr;
	vector <RoomInfo>::iterator i;
	for (i = rooms.begin(); i != rooms.end(); ++i)
	{
		if (i->up == up && i->down == down && i->left == left && i->right == right /* && i->teleport == teleport */)
		{
			result = &(*i);
			break;
		}	
	}
	if (!result) {
		cerr << string_format("Couldn't find room with %i %i %i %i %i\n", up, down, left, right, teleport);
		assert(false);
	}
	
	return result;
}

Room::Room (Objects *o, RoomInfo *ri, int monsterHp, int aInitFlags) : roomInfo(ri), objects (o), map (NULL)
{
	assert(ri);
	doors[0] = NULL;
	doors[1] = NULL;
	doors[2] = NULL;
	doors[3] = NULL;
	teleport = NULL;
	map = ri->map;
	initFlags = aInitFlags;
	bananaCount = 0;
	
	int keyCount = 0;
	int bonusCount = 0;

	int maxKeys = (initFlags & INIT_KEY ? 1 : 0);
	int maxBananas = (initFlags & INIT_BANANA ? 1 : 0);
	int maxBonus = (initFlags & INIT_BONUS ? 1 : 0);

	// add monsters, doors, etc. (but do not link doors yet)
	vector <ObjectInfo>::iterator i;
	for (i = ri->objectInfo.begin(); i != ri->objectInfo.end(); ++i)
	{
		switch (i->type)
		{
			case ObjectInfo::DOOR:
				{
					Door *d = new Door (this, OT_DOOR);
					d->setDir (i->doorDir);
					d->setLocation (i->x * TILE_SIZE, i->y * TILE_SIZE);
					objects->add (d);
					doors[i->doorDir] = d;
				}
				break;
			case ObjectInfo::TELEPORT:
				{
					if (initFlags & INIT_TELEPORTER) {
						Door *t = new Door (this, OT_TELEPORT);
						t->setLocation (i->x * TILE_SIZE, i->y * TILE_SIZE);
						objects->add (t);
						teleport = t;
					}
				}
				break;
			case ObjectInfo::MONSTER:
				{
					Monster *m = new Monster (this, i->monsterType, monsterHp);
					m->setLocation (i->x * TILE_SIZE, i->y * TILE_SIZE);
					objects->add (m);
				}
				break;
			case ObjectInfo::BANANA:
				{
					if (bananaCount < maxBananas) {
						PickUp *b = new PickUp (this, OT_BANANA);
						b->setLocation (i->x * TILE_SIZE, i->y * TILE_SIZE);
						objects->add (b);
						bananaCount++;
					}
					else if (keyCount < maxKeys) {
						PickUp *b = new PickUp (this, OT_KEY);
						b->setLocation (i->x * TILE_SIZE, i->y * TILE_SIZE);
						objects->add (b);
						keyCount++;
					}
					else if (bonusCount < maxBonus) {
						vector<int> choices = { OT_BONUS1, OT_BONUS2, OT_BONUS3, OT_BONUS4, OT_HEALTHCONTAINER };
						PickUp *b = new PickUp (this, choice(choices));
						b->setLocation (i->x * TILE_SIZE, i->y * TILE_SIZE);
						objects->add (b);
						bonusCount++;
					}
				}
				break;
			case ObjectInfo::PLAYER:
				break;
		}
	}

	assert(keyCount == maxKeys);
	assert(bananaCount == maxBananas);
}

void Room::linkDoor (Room *otherRoom, int dir, bool reverse, bool locked)
{
	int odir = LEGACY_DIR[dir].reverse;
	assert (doors[dir]);
	assert (otherRoom);
	assert (otherRoom->doors[odir]);
	doors[dir]->link(otherRoom->doors[odir], reverse);
}

void Room::lockDoor (int dir) {
	assert(doors[dir]);
	doors[dir]->setLocked(true);
}

void Room::linkTeleport (Room *otherRoom, bool reverse)
{
	assert (teleport);
	assert (otherRoom);
	assert (otherRoom->teleport);
	teleport->link(otherRoom->teleport, reverse);
}

void Room::getPlayerLocation(int player, int *x, int *y) {
	auto it = std::find_if(roomInfo->objectInfo.begin(), roomInfo->objectInfo.end(), 
		[=](ObjectInfo &o) { return o.type == ObjectInfo::PLAYER && o.pi == player; }
	);
	*x = it->x;
	*y = it->y;
}

Room *Level::getStartRoom(int player) { 
	int flag = player == 0 ? INIT_P1_START : INIT_P2_START;
	auto it = std::find_if(rooms.begin(), rooms.end(), [=](Room *room){ 
		return (room->getFlags() & flag) > 0;
	});
	return *it;
}

Level::~Level()
{
	vector<Room *>::iterator i;
	for (i = rooms.begin(); i != rooms.end(); ++i)
	{
		delete (*i);
	}
}

int Level::getBananaCount()
{
	int result = 0;
	vector <Room *>::iterator i;
	for (i = rooms.begin(); i != rooms.end(); ++i)
	{
		result += (*i)->getBananaCount();
	}
	return result;
}

Level* createLevel(RoomSet *roomSet, Objects *objects, unsigned int numRooms, int monsterHp) {
	// each node becomes a room

	Map2D<Cell> grid = createKruskalMaze(numRooms);
	auto nodes = getAllNodes(grid);

	Level *level = new Level();
	map<Node*, Room*> node2room;

	for (auto n : nodes) {

		int initFlags = 
			(n->hasLink(N) ? INIT_DOOR_N : 0) |
			(n->hasLink(E) ? INIT_DOOR_E : 0) |
			(n->hasLink(S) ? INIT_DOOR_S : 0) |
			(n->hasLink(W) ? INIT_DOOR_W : 0) |
			(n->hasLink(TELEPORT) ? INIT_TELEPORTER : 0) |
			(n->hasLock(N) ? INIT_LOCK_N : 0) |
			(n->hasLock(E) ? INIT_LOCK_E : 0) |
			(n->hasLock(S) ? INIT_LOCK_S : 0) |
			(n->hasLock(W) ? INIT_LOCK_W : 0) |
			(n->hasBanana ? INIT_BANANA : 0) |
			(n->hasKeycard ? INIT_KEY : 0) |
			(n->hasBonus ? INIT_BONUS : 0) |
			((n->pStart == 1) ? INIT_P1_START : 0) |
			((n->pStart == 2) ? INIT_P2_START : 0);
			
		Room *r = new Room(objects, roomSet->findRoom(
			n->hasLink(N), n->hasLink(S), n->hasLink(W), n->hasLink(E), n->hasLink(TELEPORT)), monsterHp, initFlags);
		level->rooms.push_back(r);
		node2room[n] = r;
	}

	for (auto n : nodes) {
		Room *src = node2room[n];
		for (auto pair : n->links) {
			Dir dir = pair.first;
			Room* dest = node2room[pair.second];
			assert(src);
			assert(dest);

			int legacyDir = -1;
			switch(dir) {
				case N: legacyDir = 0; break;
				case S: legacyDir = 1; break;
				case W: legacyDir = 2; break;
				case E: legacyDir = 3; break;
				case TELEPORT: /* no corresponding legacy dir */ break;
			}

			switch(dir) {
				case N: case S: case W: case E:
					src->linkDoor(dest, legacyDir, false); 
					if (n->hasLock(dir)) src->lockDoor(legacyDir);
					break;
				case TELEPORT: src->linkTeleport(dest, false); 
					break;
			}
		}
	}

	return level;
}
