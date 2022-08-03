#include <iostream>
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "color.h"
#include "particle.h"
#include <cmath>

using namespace std;

void clearGenerateFunction (list<Particle> &particles)
{
	return; // do nothing
}

void clearUpdateFunction (Particle *p)
{
	return; // do nothing
}

ParticleEffectImpl::ParticleEffectImpl() : generateFunction(clearGenerateFunction),
		updateFunction(clearUpdateFunction), drawFunction(clearUpdateFunction) {}

void snowGenerateFunction (list<Particle> &particles, int w)
{
	if (particles.size() < 1000)
	{
		// new ones at top of screen
		Particle particle;
		particle.alive = true;
		particle.x = random(w);
		particle.y = 0;
		particle.color = WHITE;
		particles.push_back(particle);
	}
}

void antigravGenerateFunction (list<Particle> &particles, int w, int h)
{
	if (particles.size() < 500)
	{
		// new ones at bottom of screen
		Particle particle;
		particle.alive = true;
		double scale = ((double)random(100)) / 100.0;
		particle.x = random(w);
		particle.y = h;
		particle.z = 2.0; // line length multiplier
		particle.r = 2.0; // line width
		particle.dx = 0;
		particle.dy = -6.0 * (1 + scale);
		particle.color = al_map_rgb_f (0, scale, 0);
		particles.push_back(particle);
	}
}

void confettiGenerateFunction (list<Particle> &particles, int w)
{
	if (particles.size() < 1000)
	{
		// new ones at top of screen
		Particle particle;
		particle.alive = true;
		particle.x = random(w);
		particle.y = 0;
		double r = (double)(random (100)) / 100.0;
		double g = (double)(random (100)) / 100.0;
		double b = (double)(random (100)) / 100.0;
		particle.color = al_map_rgb_f(r, g, b);
		particles.push_back(particle);
	}
}

void rainGenerateFunction (list<Particle> &particles, int w)
{
	if (particles.size() < 500)
	{
		// new ones at top of screen
		Particle particle;
		particle.alive = true;
		double scale = ((double)random(100)) / 100.0;
		particle.z = 2.0; // line length multiplier
		particle.r = 2.0; // line width
		particle.x = random(w * 3 / 2);
		particle.y = 0;
		particle.dx = -3.0 * (1 + scale);
		particle.dy = 6.0 * (1 + scale);
		particle.color = GREY;
		particles.push_back(particle);
	}
}

void starGenerateFunction (list<Particle> &particles, int h)
{
	if (particles.size() < 800)
	{
		Particle particle;
		particle.alive = true;
		double scale = (random(100) / 100.0);
		double speed = scale * 4.0;
		particle.x = 0;
		particle.dx = speed;
		particle.y = random(h);
		particle.dy = 0;
		particle.color = al_map_rgb_f(scale, scale, scale);
		particles.push_back(particle);
	}
}

void meteorGenerateFunction (list<Particle> &particles, int h)
{
	if (particles.size() < 800)
	{
		Particle particle;
		particle.alive = true;
		double scale = (random(100) / 100.0);
		double speed = scale * 4.0;
		particle.x = 0;
		particle.dx = speed;
		particle.y = random(h);
		particle.dy = 0;
		particle.color = al_map_rgb_f(scale, scale, scale);
		particles.push_back(particle);
	}
}

void powGenerateFunction (list<Particle> &particles, int w, int h)
{
	if (particles.size() < 500)
	{
		Particle particle;
		particle.z = 2.0; // line length multiplier
		particle.r = 2.0; // line width
		particle.alive = true;
		particle.x = w / 2;
		particle.y = h / 2;
		double angle = (double)(random(360)) * M_PI / 180.0;
		double speed = 10;
		particle.dx = std::sin(angle) * speed;
		particle.dy = std::cos(angle) * speed;
		particle.color = MAGENTA;
		particles.push_back(particle);
	}
}

void windGenerateFunction (list<Particle> &particles, int w, int h)
{
	if (particles.size() < 500)
	{
		// new ones at right of screen
		Particle particle;
		particle.alive = true;
		particle.x = w + random(20);
		particle.y = random(h);
		particle.dx = -0.1;
		particle.dy = 0;
		particle.color = BROWN;
		particles.push_back(particle);
	}
}

void vortexGenerateFunction (list<Particle> &particles, int w, int h)
{
	if (particles.size() < 500)
	{
		double scale = ((double)random(100)) / 100.0;
		// new ones at right of screen
		Particle particle;
		particle.alive = true;
		particle.x = random(w);
		particle.y = random(h);
		particle.color = al_map_rgb_f (0, scale, 0);
		particles.push_back(particle);
	}
}

void linearUpdateFunction (Particle *p)
{
	p->x += p->dx;
	p->y += p->dy;
	p->r += p->dr;
	// fast movement with constant speed
}

// array of semi-random vectors
double windx[11] = { -0.1, -0.05, -0.15, -0.1, -0.15, -0.05, -0.02, 0.03, -0.1, 0 , -0.03 };
double windy[11] = { 0.1, -0.1, 0.05, -0.05,  0.03, -0.03,  0.1, -0.1, 0.1, -0.1, 0.03 };

void windUpdateFunction (Particle *p)
{
	int idx = (int)((p->x / 64) + (p->y / 64)) % 11;
	p->dx += windx[idx];
	p->dy += windy[idx];

	p->x += p->dx;
	p->y += p->dy;
}

// rotate around center point.
void vortexUpdateFunction (Particle *p, int w, int h)
{
	double angle = 0.01;

	double cx = p->x - (w / 2);
	double cy = p->y - (h / 2);

	double new_x = cx * cos (angle) - cy * sin (angle);
	double new_y = cx * sin (angle) + cy * cos (angle);

	new_x *= 1.01;
	new_y *= 1.01;

	p->x = new_x + (w / 2);
	p->y = new_y + (h / 2);
}

void flurryUpdateFunction (Particle *p)
{
	double dx = (double)(random(5) - 2) / 2.0;
	double dy = (double)(random(5)) / 2.0;
	p->x += dx;
	p->y += dy;
	// flurry. random movement downwards
}

void circleDrawFunction (Particle *p)
{
	al_draw_filled_circle(p->x, p->y, 2.0, p->color);
}

void lineDrawFunction (Particle *p)
{
	al_draw_line(p->x, p->y, p->x + (p->dx * p->z), p->y + (p->dy * p->z), p->color, p->r);
}

void bitmapDrawFunction(Particle *p, ALLEGRO_BITMAP *bmp) {

	double w = al_get_bitmap_width(bmp) / 2;
	double h = al_get_bitmap_height(bmp) / 2;
	al_draw_tinted_scaled_rotated_bitmap(
		bmp,
		p->color,
		w / 2, h / 2,
		p->x + w / 2, p->y + h / 2,
		p->scale, p->scale,
		p->r,
		0
	);
}

class ParticleRemover
{
public:
	bool operator()(Particle &p)
	{
		if (!p.alive)
		{
			return true;
		}
		return false;
	}
};

void Particles::update()
{
	currentEffect.generateFunction(particles);

	// update existing
	for (list<Particle>::iterator p = particles.begin(); p != particles.end(); ++p)
	{
		if (!p->alive) continue;

		currentEffect.updateFunction(&(*p));

		if (p->x > (w + 200) || p->x < (-w - 200) || p->y > (h + 100) || p->y < (-100)) { p->alive = false; }
	}

	// remove all that are not alive!
	particles.remove_if (ParticleRemover());
}


Particles::Particles() : effect(CLEAR), currentEffect(), particles()
{
	ALLEGRO_DISPLAY *d = al_get_current_display();
	w = al_get_display_width(d);
	h = al_get_display_height(d);
}

void Particles::draw(const GraphicsContext &gc)
{
	for (list<Particle>::iterator p = particles.begin(); p != particles.end(); ++p)
	{
		if (!p->alive) continue;
		currentEffect.drawFunction(&(*p));
	}
}


Particles::~Particles() {}

using namespace std::placeholders;

void Particles::setEffect(ParticleEffect p)
{
	if (p != effect)
	{
		particles.clear();
		effect = p;

		switch (effect)
		{
		case CLEAR:
			currentEffect.generateFunction = &clearGenerateFunction;
			currentEffect.updateFunction = &clearUpdateFunction;
			currentEffect.drawFunction = &clearUpdateFunction;
			break;
		case METEOR:
			currentEffect.generateFunction = std::bind (meteorGenerateFunction, _1, geth());
			currentEffect.updateFunction = &linearUpdateFunction;
			currentEffect.drawFunction = &circleDrawFunction;
			break;
		case SNOW:
			currentEffect.generateFunction = std::bind(snowGenerateFunction, _1, getw());
			currentEffect.updateFunction = &flurryUpdateFunction;
			currentEffect.drawFunction = &circleDrawFunction;
			break;
		case CONFETTI:
			currentEffect.generateFunction = std::bind(confettiGenerateFunction, _1, getw());
			currentEffect.updateFunction = &flurryUpdateFunction;
			currentEffect.drawFunction = &circleDrawFunction;
			break;
		case STARS:
			currentEffect.generateFunction = std::bind(starGenerateFunction, _1, geth());
			currentEffect.updateFunction = &linearUpdateFunction;
			currentEffect.drawFunction = &circleDrawFunction;
			break;
		case RAIN:
			currentEffect.generateFunction = std::bind(rainGenerateFunction, _1, getw());
			currentEffect.updateFunction = &linearUpdateFunction;
			currentEffect.drawFunction = &lineDrawFunction;
			break;
		case ANTIGRAV:
			currentEffect.generateFunction = std::bind(antigravGenerateFunction, _1, getw(), geth());
			currentEffect.updateFunction = &linearUpdateFunction;
			currentEffect.drawFunction = &lineDrawFunction;
			break;
		case POW:
			currentEffect.generateFunction = std::bind(powGenerateFunction, _1, getw(), geth());
			currentEffect.updateFunction = &linearUpdateFunction;
			currentEffect.drawFunction = &lineDrawFunction;
			break;
		case WIND:
			currentEffect.generateFunction = std::bind(windGenerateFunction, _1, getw(), geth());
			currentEffect.updateFunction = &windUpdateFunction;
			currentEffect.drawFunction = &circleDrawFunction;
			break;
		case VORTEX:
			currentEffect.generateFunction = std::bind(vortexGenerateFunction, _1, getw(), geth());
			currentEffect.updateFunction = std::bind(vortexUpdateFunction, _1, getw(), geth());
			currentEffect.drawFunction = &circleDrawFunction;
			break;
		}

	}

}
