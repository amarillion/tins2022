#pragma once

#include "component.h"
#include <memory>
#include "graphicscontext.h"
#include <allegro5/allegro.h>

#include "componentbuilder.h"

struct ALLEGRO_BITMAP;
struct TEG_MAP;

class ClearScreen : public Widget
{
public:
	ClearScreen(ALLEGRO_COLOR color = WHITE) { this->color = color; }
	virtual void draw(const GraphicsContext &gc) override;
	static ComponentBuilder<ClearScreen> build(ALLEGRO_COLOR color = WHITE);
	virtual std::string const className() const override { return "ClearScreen"; }
	void setColor(ALLEGRO_COLOR val) { color = val; }
private:
	ALLEGRO_COLOR color;
};

class Pattern : public Component
{
public:
	Pattern(ALLEGRO_BITMAP *bmp) : texture(bmp), textureHolder() {}
	Pattern(std::shared_ptr<ALLEGRO_BITMAP> _bmp) : texture(_bmp.get()), textureHolder(_bmp) {}

	virtual void draw(const GraphicsContext &gc) override;
	static ComponentBuilder<Pattern> build(ALLEGRO_BITMAP *bmp);
	static ComponentBuilder<Pattern> build(std::shared_ptr<ALLEGRO_BITMAP> bmp);
	virtual std::string const className() const override { return "Pattern"; }
private:

	/**
	 * Actual reference used for drawing in held and non-held situation
	 */
	ALLEGRO_BITMAP *texture;

	/**
	 * For owned bitmaps, this will ensure that texture is deleted
	 */
	std::shared_ptr<ALLEGRO_BITMAP> textureHolder;
};

class BitmapComp : public Component
{
private:

	/**
	 * For owned bitmaps, this will ensure that rle is deleted
	 */
	std::shared_ptr<ALLEGRO_BITMAP> rleHolder;

	/**
	 * Actual reference used for drawing in held and non-held situation
	 */
	ALLEGRO_BITMAP *rle;

	double zoom;
	bool isZoomed;
	double hx, hy;
public:

	/**
	 * Call this constructor when ownership is transferred to BitmapComp. BitmapComp will ensure al_destroy_bitmap is called.
	 */
	BitmapComp(std::shared_ptr<ALLEGRO_BITMAP> _rle) : rleHolder(_rle), rle(_rle.get()), zoom(1.0), isZoomed(false), hx(0.0), hy(0.0) {}

	/**
	 * Call this constructor when ownership of bitmap is not transferred
	 */
	BitmapComp(ALLEGRO_BITMAP*  _rle) : rleHolder(), rle(_rle), zoom(1.0), isZoomed(false), hx(0.0), hy(0.0) {}

	virtual void draw(const GraphicsContext &gc) override;

	/**
	 * Call this builder when ownership is transferred to BitmapComp. BitmapComp will ensure al_destroy_bitmap is called.
	 */
	static ComponentBuilder<BitmapComp> build(std::shared_ptr<ALLEGRO_BITMAP> _rle);

	/**
	 * Call this builder when ownership of bitmap is not transferred
	 */
	static ComponentBuilder<BitmapComp> build(ALLEGRO_BITMAP *_rle);

	void setZoom(double _zoom) { zoom = _zoom; isZoomed = true; }
	double getZoom() { return zoom; }

	/**
	 * For use in combination with zoom, you can set a fractional hotspot to use a center point for the zoom.
	 * The hotspot will remain in position.
	 * */
	void setHotspot (double hxval, double hyval) { hx = hxval; hy = hyval; }

	virtual std::string const className() const override { return "BitmapComp"; }
};

class TileMap : public Component
{
	int counter = 0;
	int layer = 0;
public:
	TileMap(const TEG_MAP *tilemap, int layer = 0);
	virtual void draw(const GraphicsContext &gc) override;
	virtual void update() override { counter++; }
	static ComponentBuilder<TileMap> build(TEG_MAP *map, int _layer = 0);
	virtual std::string const className() const override { return "TileMap"; }
private:
	const TEG_MAP *tilemap;
};

class Anim;

class AnimComponent : public Component
{
private:
	int exitCode;
	Anim *anim;
	int dir;
	int state;
public:
	static ComponentBuilder<AnimComponent> build(Anim *anim);
	AnimComponent (Anim *anim);
	virtual void draw(const GraphicsContext &gc) override;
	virtual std::string const className() const override { return "AnimComponent"; }
	void setState(int val) { state = val; }
	void setDir(int val) { dir = val; }
};
