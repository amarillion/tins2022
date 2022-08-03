#pragma once

#include <list>
#include <functional>
#include <allegro5/allegro.h>
#include "component.h"

enum ParticleEffect {
	SNOW, RAIN, STARS, CLEAR, CONFETTI, ANTIGRAV, METEOR, WIND, POW, VORTEX
};

struct Particle
{
	double x;
	double y;
	double z; // used for z-coordinate, scale, or line length
	double dx;
	double dy;
	double dz;
	double r; // used for rotation or radius, or line width
	double dr;
	double scale;
	bool alive;
	ALLEGRO_COLOR color;
};

typedef std::function< void(Particle *)> ParticleFunc;
typedef std::function< void(std::list<Particle> &)> ParticleFactoryFunc;

void linearUpdateFunction (Particle *p);
void circleDrawFunction (Particle *p);
void lineDrawFunction (Particle *p);
void bitmapDrawFunction(Particle *p, ALLEGRO_BITMAP *bmp);

struct ParticleEffectImpl
{
public:
	ParticleEffectImpl();

	ParticleFactoryFunc generateFunction;
	ParticleFunc updateFunction;
	ParticleFunc drawFunction;
};

class Particles : public Component
{
private:
	ParticleEffect effect;
	ParticleEffectImpl currentEffect;
	std::list<Particle> particles;
public:
	Particles();
	virtual void update();
	virtual void draw(const GraphicsContext &gc);
	virtual ~Particles();
	
	// TODO: make external. Use setEffect(Particles, ParticleEffect) instead
	void setEffect(ParticleEffect p);

	void clear() { particles.clear(); }
	void setGenerateFunction(ParticleFactoryFunc value) { currentEffect.generateFunction = value; }
	void setUpdateFunction(ParticleFunc value) { currentEffect.updateFunction = value; }
	void setDrawFunction(ParticleFunc value) { currentEffect.drawFunction = value; }
};
