#pragma once

struct ALLEGRO_BITMAP;

/// contains a text description of the last error that occurred
extern char teg_error[256];

/** description of a single tile
	Depending on the type of the tilelist, this holds either a pointer to an RLE_SPRITE, 
	a BITMAP or it holds an index of a part of a larger BITMAP.
	the flags member may hold user defined information such as tile properties.
*/
typedef struct TEG_TILE
{	
	ALLEGRO_BITMAP *bmp;
	int index; 
	int flags;
} TEG_TILE;

/**
	description of a set of tiles.
*/
typedef struct TEG_TILELIST
{
	// int flags;

	int tilew, tileh, tilenum;    
	// tilenum is number of indices
	
	int animsteps; // number of animation steps per cycle
	
	TEG_TILE* tiles; // list of tiles, tilenum * animsteps
	ALLEGRO_BITMAP* largebmp;
	char *rawdata;
	int rawsize;
	
	char *filename;
	char *bitmapfilename;
} TEG_TILELIST;

typedef struct TEG_MAP
{
	int *data; // array of int of size dl * w * h
	int dl, w, h; // number of layers, width and height
	TEG_TILELIST *tilelist;
	char *tiles_filename;
} TEG_MAP;

/// create empty map
TEG_MAP *teg_createmap (int dl, int w, int h, TEG_TILELIST *tilelist);

/// destroy a map
void teg_destroymap (TEG_MAP* map);

/// get index at a certain pos in the map
int teg_mapget (const TEG_MAP* map, int l, int x, int y);
/// put index at a certain pos in the map
void teg_mapput (TEG_MAP* map, int l, int x, int y, int val);

/// loads in tiles and links with bitmaps found in a datafile
//TEG_TILELIST *teg_loadtiles (const char *filename, const DATAFILE *data);
TEG_TILELIST *teg_loadtiles (const char *filename);

/// destroy a tilelist.
void teg_destroytiles (TEG_TILELIST* tiles);

/// draws the tilemap scrolled by xview and yview.
// draws to the current target bitmap
void teg_draw (const TEG_MAP* map, int layer, int xview, int yview);

/// draws only a single frame
// draws to the current target bitmap
void teg_draw_frame (const TEG_MAP* map, int layer, int xview, int yview, int frame);

/** idem as teg_draw, but only a part of the target bitmap will be drawn.
	x, y, w and h are relative to the target bitmap coordinates
	xview and yview are relative to the target bitmap (0,0), not to (x,y)
*/
// draws to the current target bitmap
void teg_partdraw (const TEG_MAP* map, int layer, int x, int y, int w, int h, int xview, int yview);

/// width of a map in pixels (equals width in tiles * width of a single tile)
int teg_pixelw(const TEG_MAP *map);
/// height of a map in pixels
int teg_pixelh(const TEG_MAP *map);

/// copy a part of a map to another map
void teg_mapcopy (TEG_MAP *srcmap, TEG_MAP* destmap,
	int srcl, int srcx, int srcy,
	int destl, int destx, int desty,
	int dl, int w, int h);
		
/// copy constructor, create a new map that is a deep copy of the source object
TEG_MAP *teg_create_copy (TEG_MAP *src);

/// draw a single tile, independant of the tile type
// draws to the current target bitmap
void teg_drawtile (TEG_TILELIST *tiles, int index, int x, int y);

/**
 * Draw a tilemap repeated over and over to fill the entire target bitmap
 */
void teg_draw_repeated (ALLEGRO_BITMAP *bmp, const TEG_MAP* map, int layer, int xview, int yview, int frame);
