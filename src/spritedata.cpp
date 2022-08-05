#include <allegro5/allegro.h>
#include "spritedata.h"

SpriteData spriteData [SPRITE_NUM] =
{
	{
		// SST_RUBBLE
		animName : "rock",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
	{
		//SST_BUSH
		animName : "cactus",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_TREE
		animName : "tree",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: true,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_HOUSE
		animName : "house",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: true,
		has_state50: true,
		has_rubble: false,
	},
	{
		// SST_TALL
		animName : "tall",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: true,
		has_state50: true,
		has_rubble: false,
	},
	{
		// SST_TANK
		animName : "tank",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: true,
		has_state50: false,
		has_rubble: true,
	},
	{
		// SST_CHOPPER
		animName : "chopper",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: true,
		has_state50: false,
		has_rubble: true,
	},
	{
		// SST_HEALTH
		animName : "health",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: true,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_TURRET
		animName : "turret",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: true,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_BUNKER
		animName : "bunker",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: true,
		has_state50: true,
		has_rubble: false,
	},
	{
		// SST_MAINBUNKER
		animName : "mainbunker",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: true,
		has_state50: true,
		has_rubble: false,
	},
	{
		// SST_ENEMYTANK
		animName : "enemytank",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: true,
		has_state50: false,
		has_rubble: true,
	},
	{
		// SST_SAM
		animName : "sam",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: true,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_BULLET
		animName : "bullet",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_MISSILE
		animName : "missile",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_LOGCABIN
		animName : "logcabin",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: true,
		has_state50: true,
		has_rubble: false,
	},
	{
		// SST_GIFT
		animName : "gift",
		anim : NULL,
		solid : true,
		shadow : false,
		blocking: true,
		has_state50: false,
		has_rubble: true,
	},
	{
		// SST_XMASTREE
		animName : "xmastree",
		anim : NULL,
		solid : true,
		shadow : true,
		blocking: true,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_GIFT1
		animName : "gift1",
		anim : NULL,
		solid : false,
		shadow : false,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_GIFT2
		animName : "gift2",
		anim : NULL,
		solid : false,
		shadow : false,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_GIFT3
		animName : "gift3",
		anim : NULL,
		solid : false,
		shadow : false,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
	{
		// SST_GIFT4
		animName : "gift4",
		anim : NULL,
		solid : false,
		shadow : false,
		blocking: false,
		has_state50: false,
		has_rubble: false,
	},
};
