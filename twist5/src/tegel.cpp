#include "tegel5.h"
#include <assert.h>
#include <fstream>
#include "strutil.h"
#include <allegro5/allegro.h>

using namespace std;

#define TEGERR_OK 0
#define TEGERR_TLL_VERSION 1
#define TEGERR_MAP_VERSION 2
#define TEGERR_RESOURCE_NOT_FOUND 3
#define TEGERR_RESOURCE_FORMAT 4
#define TEGERR_HEADER_FORMAT 5
#define TEGERR_UEO_BODY 6
#define TEGERR_MEMORY 7
#define TEGERR_UEO_HEADER 8
#define TEGERR_BPP_IMPOS 9
#define TEGERR_FOPEN 10
#define TEGERR_DATAFILE 11
#define TEGERR_DATAOBJ 12
#define TEGERR_UEO_MAP 13
#define TEGERR_MAP_FORMAT 14
#define TEGERR_BITMAP_NOT_FOUND 15

const char *teg_errorlist[] = 
{
	"Everything is ok",
	"The tilelist version is not supported",
	"The map version [%i] is not supported",
	"Could not find required resource [%s] in datafile",
	"Required resource [%s] is of the wrong size or format",
	"The tilelist header does not contain required field [%s]",
	"Unexpected end of tilelist body",
	"Out of memory!",
	"Unexpected end of tilelist header",
	"Impossible number of bits per pixel",
	"file can't be opened",
	"General datafile error: %s",
	"Error with tegel datafile object: %s",
	"Map file ended unexpectedly",
	"Map file formatted incorrectly",
	"Could not load bitmap referred to by tilelist",
};


char teg_error[256] = "Everything is ok";

int _teg_installed = 0;

int teg_scan_data_line (char *line, TEG_TILE *tempframes, TEG_TILELIST *tiles /*, const DATAFILE *data */);

/**
 * Same as strncpy, but ensure null terminated
 * Some systems define strlcpy, some don't.
 * To avoid name clashes, this function is called teg_strlcpy instead.
 */
void teg_strlcpy(char * dest, const char *src, size_t len)
{
	strncpy (dest, src, len);
	if (strlen(src) > len-1) dest[len-1] = 0;
}

// grow or shrink the tilelist, preserving as much data as possible
// returns 0 when failed, 1 on success.
int teg_tilelist_resize (TEG_TILELIST *tiles, int newtilenum, int newanimsteps)
{
	// try to allocate new memory
	TEG_TILE *temp;
	assert (tiles);
	assert (newtilenum >= 0);
	assert (newanimsteps >= 0);
	temp = (TEG_TILE*)malloc(newtilenum * newanimsteps * sizeof (TEG_TILE));	
	if (temp)
	{
		int frame;
		// set mem to 0
		memset (temp, 0, newtilenum * newanimsteps * sizeof (TEG_TILE));
		// copy old values
		for (frame = 0; frame < std::min(newanimsteps, tiles->animsteps); ++frame)
		{
			memcpy ((temp + frame * newtilenum), (tiles->tiles + frame * tiles->tilenum),
				std::min (newtilenum, tiles->tilenum) * sizeof (TEG_TILE));
		}
		// free old memory
		free (tiles->tiles);
		tiles->tiles = NULL;
		// replace old with new
		tiles->tiles = temp;
		tiles->animsteps = newanimsteps;
		tiles->tilenum = newtilenum;
		return 1;
	}
	else
	{
		teg_strlcpy (teg_error, teg_errorlist[TEGERR_MEMORY], sizeof(teg_error)-1);
		return 0;		
	}
}


// read TEG_TILE
// returns 0 on failure -> error is set in teg_error
// returns 1 on success
// data returned in tempframes buffer (should be allocated and freed by caller)
// tiles is used to read animsteps and flags from. It is not changed in this function
// datafile is used to search for the bitmap / rle sprite
int teg_scan_data_line (const char *line, TEG_TILE *tempframes, TEG_TILELIST *tiles)
{
	int pos = 0;
	int scanned;
	bool lastframe = false;
	int frame;
	char buffer[256];
	int error = false;
	int flags = 0;	
	if (sscanf (line + pos, " %i%*[ \t.,:;]%n", &flags, &scanned) > 0)
	{
		pos += scanned;
	}	
	else
	{
		teg_strlcpy (teg_error, teg_errorlist[TEGERR_UEO_BODY], sizeof(teg_error)-1);
		error = true;
	}
	for (frame = 0; frame < tiles->animsteps && !lastframe && !error; )
	{	
		tempframes[frame].bmp = NULL;
		tempframes[frame].index = 0;
		tempframes[frame].flags = flags;	
		int index;
		if (sscanf (line + pos, " %d%n", &index, &scanned) > 0)
		{
			pos += scanned;
			tempframes[frame].index = index;
		}
		else
		{
			lastframe = true;
		}
		if (!error && !lastframe) frame++;
		// if we're after the first, search for a separator
		// if none is found, assume we've read the last tile-id of this line.
		if (!error) 
		{
			scanned = 0;
			sscanf (line + pos, "%*[ \t.,:;]%n", &scanned);
			if (scanned <= 0)
			{
				// no separator found
				lastframe = true;
			}
			else
			{
				pos += scanned;
			}
		}
	}
	// if there were no frames, there is a problem
	if (!error && frame == 0)
	{
		error = true;
		teg_strlcpy (teg_error, teg_errorlist[TEGERR_UEO_BODY], sizeof(teg_error)-1);
	} 
	if (!error && frame < tiles->animsteps)
	{
		// fill out the rest
		// by looping throught the ones already set
		// so if the line reads "a, b, c" and animsteps is 8, the result will be
		// a, b, c, a, b, c, a, b
		int i;
		for (i = frame; i < tiles->animsteps; ++i)
		{
			memcpy (tempframes + i, tempframes + (i % frame), sizeof (TEG_TILE));
		}			
	}
	return (error == false);
}

/** pass filename of tileset (used to find relative location of tiles bitmap) */
TEG_TILELIST *teg_loadtiles_data_v3(ifstream &f, const char *filename)
{
	TEG_TILELIST *temp;
	int error = false;
	string line;

	int i;
	int tilecount;

	// *******************************************
	// first we read the header of the tilelist
	// *******************************************
	ALLEGRO_BITMAP *obj = NULL;

	temp = (TEG_TILELIST*)malloc(sizeof (TEG_TILELIST));
	if (!temp) {
		error = true;
		strncpy (teg_error, teg_errorlist[TEGERR_MEMORY], sizeof(teg_error)-1);
	}
	if (!error)
	{
		temp->rawdata = NULL;
		temp->rawsize = 0;
		temp->filename = NULL;
		temp->largebmp = NULL;
		temp->bitmapfilename = NULL;
	}
	if (!error)
	{
		error = !getline(f, line);
		if (sscanf (line.c_str(), " tilew = %d ", &temp->tilew) != 1) {
			error = true;
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_HEADER_FORMAT], "tilew");
		}
	}
	if (!error)
	{
		error = !getline(f, line);
		if (sscanf (line.c_str(), " tileh = %d ", &temp->tileh) != 1) {
			error = true;
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_HEADER_FORMAT], "tileh");
		}
	}
	if (!error)
	{
		error = !getline(f, line);
		if (sscanf (line.c_str(), " animsteps = %d ", &temp->animsteps) != 1) {
			error = true;
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_HEADER_FORMAT], "animsteps");
		}
	}
	if (!error)
	{
		error = !getline(f, line);
		char buffer2[256];
		if (sscanf (line.c_str(), " bitmap = %128s ", buffer2) != 1) {
			error = true;
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_HEADER_FORMAT], "animsteps");
		}
		else
		{
			temp->bitmapfilename = (char*)malloc(strlen(buffer2) + 1);
			strncpy (temp->bitmapfilename, buffer2, strlen(buffer2) + 1);
		}
	}
	// temp->tilenum is no longer read from file but inferred from list length
	temp->tilenum = 0;
	tilecount = 0; // number of tiles we have encountered thus far.

	// *******************************************
	// now the body
	// *******************************************

	// read largebmp if applicable
	if (!error)
	{
		// take path relative to tll file...
		// start from tilelist filename...

		ALLEGRO_PATH *tail = al_create_path(temp->bitmapfilename);
		if (filename != NULL)
		{
			ALLEGRO_PATH * head = al_create_path(filename);
			al_rebase_path (head, tail);
			al_destroy_path(head);
		}
		obj = al_load_bitmap (al_path_cstr(tail, ALLEGRO_NATIVE_PATH_SEP));
		al_destroy_path(tail);

		if (!obj)
		{
			error = true;
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_BITMAP_NOT_FOUND], line.c_str());
		}
		if (obj != NULL)
		{
			temp->largebmp = obj;
			temp->tilenum = (al_get_bitmap_width(temp->largebmp) / temp->tilew) * (al_get_bitmap_height(temp->largebmp) / temp->tileh);
		}
		else
		{
			error = true;
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_RESOURCE_NOT_FOUND], line.c_str());
		}
	}

	// allocate memory for the tile pointers
	temp->tiles = NULL;
	if (!error)
	{
		temp->tiles = (TEG_TILE*)malloc(temp->tilenum * temp->animsteps * sizeof(TEG_TILE));
		if (temp->tiles == NULL)
		{
			error = true;
			teg_strlcpy (teg_error, teg_errorlist[TEGERR_MEMORY], sizeof(teg_error)-1);
		}
	}

	// read the tile pointers
	if (!error)
	{
		TEG_TILE* tempframes = (TEG_TILE*)malloc(temp->animsteps * sizeof (TEG_TILE));
		bool last = false;
		assert (tempframes);
		for (i = 0; !error && !last; i++)
		{
			error = !getline(f, line);
			if (line.at(0) == '!') // a "!" at the beginning of a line marks the last tile
			{
				last = true;
			}
			else
			{
				if (!teg_scan_data_line (line.c_str(), tempframes, temp /*,  data */))
				{
					error = true;
					// error message already set by teg_scan_data_line
				}
				else
				{
					// add the tempframes
					// resize if necessary
					int j;
					if (tilecount >= temp->tilenum)
					{
						// over-allocate a bit so we won't need to do this every loop
						teg_tilelist_resize (temp, temp->tilenum + 20, temp->animsteps);
					}
					// copy the data
					for (j = 0; j < temp->animsteps; ++j)
					{
						memcpy (temp->tiles + i + j * temp->tilenum, tempframes + j, sizeof (TEG_TILE));
					}
					tilecount++;
				}
			}
		}
		// now finally resize back to the number of tiles actually in use.
		if (!error) teg_tilelist_resize (temp, tilecount, temp->animsteps);
		// clean up temporary allocation
		free (tempframes);
		tempframes = nullptr;
	}

	// clean memory up if there was an error
	if (error)
	{
		if (temp)
		{
			if (temp->largebmp) al_destroy_bitmap (temp->largebmp);
			free (temp->tiles);
			temp->tiles = NULL;
			free (temp);
			temp = NULL;
		}
	}
	return temp;
}

/** pass filename of tileset (used to find relative location of tiles bitmap) */
TEG_TILELIST *teg_loadtiles_from_stream (ifstream &f, const char *filename)
{
	bool error = false;
	string line;

	int version = 0;
	
	// read TILELIST
	if (!error)
	{
		error = !getline(f, line);
		if (rtrim(line) != "TILELIST") // rtrim in case of DOS line endings on unix system!
		{
			error = true; // not right format: doesn't start with TILELIST
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_HEADER_FORMAT], "TILELIST");
		}
	}
	if (!error)
	{
		error = !getline(f, line);
		if (sscanf (line.c_str(), " version = %d ", &version) != 1)
		{
			error = true;
			snprintf (teg_error, sizeof(teg_error)-1, teg_errorlist[TEGERR_HEADER_FORMAT], "version");
		}
	}
	if (!error)
	{
		if (version < 3 || version > 3)
		{
			error = true; // not the right format version
			teg_strlcpy (teg_error, teg_errorlist[TEGERR_TLL_VERSION], sizeof(teg_error)-1);
		} 
		else if (version == 3)
		{
			return teg_loadtiles_data_v3 (f, filename);
		}
		else 
		{
			error = true; // not the right format version
			teg_strlcpy (teg_error, teg_errorlist[TEGERR_TLL_VERSION], sizeof(teg_error)-1);
		}
	}
	return NULL;	
}


void destroy_teg_tilelist (void *data)
{
	TEG_TILELIST *tiles = (TEG_TILELIST*)(data);
	teg_destroytiles (tiles);
}

void destroy_teg_map (void *data)
{
	teg_destroymap ((TEG_MAP*)data);
}

// create empty map
// set everything to 0
TEG_MAP *teg_createmap (int dl, int w, int h, TEG_TILELIST *tilelist)
{
	TEG_MAP *result = (TEG_MAP*)malloc(sizeof(TEG_MAP));
	if (result != NULL)
	{
		result->data = (int*)malloc(dl * w * h * sizeof(int));
		if (result->data != NULL)
		{
			result->dl = dl;
			result->w = w;
			result->h = h;
			result->tilelist = tilelist;
			result->tiles_filename = NULL;
			if (result->tilelist && result->tilelist->filename)
			{
				result->tiles_filename = strdup (result->tilelist->filename);
			}
			memset (result->data, 0, dl * w * h * sizeof(int));
		}
		else
		{
			// if failed to allocate room for data,
			// also destroy the allocated struct.
			free (result);
			result = NULL;
		}
	}
	return result;
}


// destroy a map
void teg_destroymap (TEG_MAP* map)
{
	if (map != NULL)
	{
		free (map->data);
		free (map->tiles_filename);
	}
	free (map);
}

// get index at a certain pos in the map
int teg_mapget (const TEG_MAP* map, int l, int x, int y)
{
	assert(map != NULL);
	assert(l >= 0 && l < map->dl);
	assert(x >= 0 && x < map->w);
	assert(y >= 0 && y < map->h);
	assert(map->data != NULL);
	return (map->data[x + map->w * (y + map->h * l)]);
}

// put index at a certain pos in the map
void teg_mapput (TEG_MAP* map, int l, int x, int y, int val)
{
	assert(map != NULL);
	assert(l >= 0 && l < map->dl);
	assert(x >= 0 && x < map->w);
	assert(y >= 0 && y < map->h);
	assert(map->data != NULL);
	map->data[x + map->w * (y + map->h * l)] = val;
}

TEG_TILELIST *teg_loadtiles (const char *filename)
{
	TEG_TILELIST *result = NULL;

	ifstream inf;
	inf.open(filename, ios::in /*,  "rt"*/); //TODO mode...
	if (!inf)
	{
		teg_strlcpy (teg_error, teg_errorlist[TEGERR_FOPEN], sizeof(teg_error)-1);
		return nullptr; // error
	}
	result = teg_loadtiles_from_stream (inf, filename);
	if (result)
	{
		assert (result->filename == NULL); // make sure that no memory for filename has been allocated yet
		int len = strlen (filename);
		result->filename = (char *)malloc(len + 1);
		teg_strlcpy (result->filename, filename, len);
	}
	inf.close();
	return result;
}

// destroy a tilelist.
void teg_destroytiles (TEG_TILELIST* tiles)
{    
	if (tiles != NULL)
	{
		free (tiles->tiles);
		free (tiles->rawdata);
		free (tiles->filename);
		if (tiles->bitmapfilename != NULL) free (tiles->bitmapfilename);
	}
	free (tiles);
}

// draws the tilemap scrolled by xview and yview.
void teg_draw (const TEG_MAP* map, int layer, int xview, int yview)
{
	assert (map != NULL);
	assert (map->tilelist != NULL);

	teg_draw_frame (map, layer, xview, yview, 0);
}

void teg_draw_frame (const TEG_MAP* map, int layer, int xview, int yview, int frame)
{
	int x, y;
	int tilex, tiley;
	int tiles_per_row = al_get_bitmap_width (map->tilelist->largebmp) / map->tilelist->tilew;
	int i;

	for (tilex = 0; tilex < map->w; tilex++)
	{
		for (tiley = 0; tiley < map->h; tiley++)
		{
			x = tilex * map->tilelist->tilew - xview;
			y = tiley * map->tilelist->tileh - yview;
			i = teg_mapget(map, layer, tilex, tiley);

			if (i >= 0 && i < map->tilelist->tilenum)
			{
				i += frame * map->tilelist->tilenum;
				teg_drawtile(map->tilelist, i, x, y);
			}
		}
	}
}

// idem as teg_draw, but only a part of the target bitmap will be drawn.
// x, y, w and h are relative to the target bitmap coordinates
// xview and yview are relative to the target bitmap (0,0), not to (x,y)
void teg_partdraw (const TEG_MAP* map, int layer, int cx, int cy, int cw, int ch, int xview, int yview)
{
	int x, y;
	int tilex, tiley;

	int ox, oy, ow, oh;
	al_get_clipping_rectangle(&ox, &oy, &ow, &oh);

	al_set_clipping_rectangle(cx, cy, cw, ch);
	for (tilex = 0; tilex < map->w; tilex++)
	{
		for (tiley = 0; tiley < map->h; tiley++)
		{
			x = tilex * map->tilelist->tilew - xview;
			y = tiley * map->tilelist->tileh - yview;

			int i = teg_mapget(map, layer, tilex, tiley);
			if (i >= 0 && i < map->tilelist->tilenum)
			{
				teg_drawtile(map->tilelist, i, x, y);
			}
		}
	}

	al_set_clipping_rectangle(ox, oy, ow, oh);
}

int teg_pixelw(const TEG_MAP *map)
{
	assert (map);
	if (!map->tilelist) return 0; else return map->w * map->tilelist->tilew;
}

int teg_pixelh(const TEG_MAP *map)
{
	assert (map);
	if (!map->tilelist) return 0; else return map->h * map->tilelist->tileh;
}

// fill up target bmp with the same map over and over
void teg_draw_repeated (ALLEGRO_BITMAP *bmp, const TEG_MAP* map, int layer, int xview, int yview, int frame)
{
	al_set_target_bitmap (bmp);
	int x, y;
	int tilex, tiley;
	int tilew, tileh;
	tileh = map->tilelist->tileh;
	tilew = map->tilelist->tilew;
	tiley = (yview / tileh) % map->h;
	while (tiley < 0) tiley += map->h;
	for (y = -(yview % tileh); y < al_get_bitmap_width(bmp); y += tileh)
	{
		tilex = (xview / tilew) % map->w;
		while (tilex < 0) tilex += map->w;
		for (x = - (xview % tilew) ; x < al_get_bitmap_width(bmp); x += tilew)
		{
//			ALLEGRO_BITMAP *s;
//			int i;
			int i = teg_mapget(map, layer, tilex, tiley);

			if (i >= 0 && i < map->tilelist->tilenum)
			{
				i += frame * map->tilelist->tilenum;
				teg_drawtile(map->tilelist, i, x, y);
			}
//
//			i = teg_mapget(map, layer, tilex, tiley);
//			if (i >= 0 && i < map->tilelist->tilenum)
//			{
//				i += frame * map->tilelist->tilenum;
//				s = map->tilelist->tiles[i].bmp;
//				if (s != NULL)
//					al_draw_bitmap (s, x, y, 0);
//			}
			tilex++;
			if (tilex >= map->w) tilex = 0;
		}
		tiley++;
		if (tiley >= map->h) tiley = 0;
	}
}

#include "tegel5.h"
#include <assert.h>
#include <stdio.h>

TEG_MAP *teg_create_copy (TEG_MAP *src)
{
	TEG_MAP* result = teg_createmap (src->dl, src->w, src->h, src->tilelist);
	teg_mapcopy (src, result,
	   0,0,0, 0,0,0,
		src->dl,
		src->w,
		src->h);
	return result;
}

/*
generic map copy function. src and dest may be the same.
part that handles overlapping regions not implemented yet.
*/
void teg_mapcopy (TEG_MAP *srcmap, TEG_MAP* destmap,
	int srcl, int srcx, int srcy,
	int destl, int destx, int desty,
	int dl, int w, int h)
{
	int l, x, y;
	for (l = 0; l < dl; l++)
		for (x = 0; x < w; x++)
			for (y = 0; y < h; y++)
				if (
					destl + l < destmap->dl &&
					destx + x < destmap->w &&
					desty + y < destmap->h &&
					destl + l >= 0 &&
					destx + x >= 0 &&
					desty + y >= 0 &&
					srcl + l < srcmap->dl &&
					srcx + x < srcmap->w &&
					srcy + y < srcmap->h &&
					srcl + l >= 0 &&
					srcx + x >= 0 &&
					srcy + y >= 0
					)
					teg_mapput (destmap,
						destl + l, destx + x, desty + y,
						teg_mapget (srcmap,
							srcl + l, srcx + x, srcy + y));
}

void teg_drawtile (TEG_TILELIST *tiles, int index, int x, int y)
{
	assert (tiles != NULL);
	assert (index >= 0);
	assert (index < (tiles->tilenum * tiles->animsteps));
	int tiles_per_row = al_get_bitmap_width(tiles->largebmp) / tiles->tilew;
	assert (tiles->largebmp != NULL);
	index = tiles->tiles[index].index;
	al_draw_bitmap_region (tiles->largebmp,
		(index % tiles_per_row) * tiles->tilew,
		(index / tiles_per_row) * tiles->tileh,
		tiles->tilew, tiles->tileh,
		x, y,
		0);
}
