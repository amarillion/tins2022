#include <assert.h>
#include "door.h"

Anim *Door::doorSprite;
Anim *Door::teleportSprite;

Door::Door (Room *r, int type) : Object (r, type), otherRoom (NULL), otherDoor (NULL)
{
	assert (type == OT_DOOR || type == OT_LOCKED_DOOR || type == OT_TELEPORT);
	
	setAnim(type == OT_TELEPORT ? teleportSprite : doorSprite);
	setVisible(true);
	solid = true;
}

void Door::init(std::shared_ptr<Resources> res)
{
	doorSprite = res->getAnim("door");
	teleportSprite = res->getAnim("teleport");
}

void Door::link(Door *door, bool reverse)
{
	assert (door);
	otherDoor = door;
	otherRoom = door->getRoom();

	if (reverse) {
		assert (door->otherDoor == NULL); // we don't want to overwrite old links
		assert (door->otherRoom == NULL); 
		door->otherDoor = this;
		door->otherRoom = getRoom();
	}
}

void Door::setLocked(bool value, bool reverse) {
	
	if (value != locked) {
		if (value) {
			assert(type == OT_DOOR);
			type = OT_LOCKED_DOOR;
			setState(1);
		}
		else {
			assert(type == OT_LOCKED_DOOR);
			type = OT_DOOR;
			setState(0);
		}
		locked = value;
	}

	if (reverse) {
		assert(otherDoor);
		otherDoor->setLocked(value, false);
	}
}
