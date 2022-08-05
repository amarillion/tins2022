#ifndef DOOR_H
#define DOOR_H

#include "object.h"
#include "color.h"

class Door : public Object
{	
	public:
		Door(Room *r, int type);
		void link (Door *_door, bool reverse = true); // links these two doors
		Room *otherRoom;
		Door *otherDoor;
		static void init(std::shared_ptr<Resources> res);
		
		virtual void handleCollission(ObjectBase *o) {};

		bool locked = false;

		static Anim *doorSprite, *teleportSprite;

		void setLocked(bool value, bool reverse = true);
};

#endif
