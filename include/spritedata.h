#ifndef SPRITEDATA_H
#define SPRITEDATA_H

class Anim;

enum SpriteSubType {
	SST_RUBBLE,	SST_BUSH, SST_TREE, SST_HOUSE, SST_TALL, SST_TANK, SST_CHOPPER, SST_HEALTH,
	SST_TURRET, SST_BUNKER, SST_MAINBUNKER, SST_ENEMYTANK, SST_SAM, SST_BULLET, SST_MISSILE,

	SST_LOGCABIN, SST_GIFT, SST_XMASTREE,

	SST_GIFT1, SST_GIFT2, SST_GIFT3, SST_GIFT4,

	SST_MAX };

class SpriteData
{
	public:
		const char *animName; // don't use Anim if NULL
		Anim *anim;
		bool solid;
		bool shadow;
		bool blocking;
		bool has_state50; // has a separate state when health gets below 50
		bool has_rubble; // has a separate state when health gets below 0
};

const int SPRITE_NUM = SST_MAX;
extern SpriteData spriteData [SPRITE_NUM];

#endif
