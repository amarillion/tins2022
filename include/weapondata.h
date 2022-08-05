#ifndef WEAPONDATA_H
#define WEAPONDATA_H

class WeaponData;

class Weapon
{
	WeaponData *data;
public:
	Weapon (ActiveSprite &parent);
	int tReload;
	void update();
	void fire();
};

class WeaponData
{
	public:
		int damage;
		int reloadTime;
		int bulletLiveTime;
		
		BulletType bulletType;
		bool hasGravity;
		float speed;
};

enum { WPN_CHOPPER, WPN_TANK, WPN_TURRET, WPN_SAM, WPN_BUNKER };

const WeaponData weapons[] =
{
	{
		damage : 1,
		reloadTime : 100,
		bulletLiveTime : 50,
		bulletType : BT_BULLET,
		hasGravity : false,
		speed : 2.0
	},
};

#endif
