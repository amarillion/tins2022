#include "screenshot.h"
#include <allegro5/allegro.h>
#include <string.h>
#include <stdio.h>
#include "util.h"

/* Returns Zero on failure, non-zero on success. */
int take_screen_shot( char *filename )
{
	ALLEGRO_BITMAP *vis; // visible screen
	int ret = -1;     // return value


	//TODO - can we copy backbuffer this way?
	ALLEGRO_BITMAP *buffer = al_get_backbuffer(al_get_current_display());
	vis = al_create_sub_bitmap(buffer, 0, 0, al_get_bitmap_width(buffer), al_get_bitmap_height(buffer));
	if( vis != NULL )
	{
		ret = al_save_bitmap (filename, vis);
		al_destroy_bitmap( vis );
	}


	return ret == 0;
}

void screenshot()
{

	char filename[14];
	int count = 0;

	// search for a unused filename of the format ssNNNNNN.pcx
	do
	{
		sprintf( filename, "ss%06d.png", count++ );
	} while ( strlen(filename) == 12 && al_filename_exists(filename) );

	if( strlen(filename) == 12 )
	{
		take_screen_shot(filename);
	}
	else
	{
		/* Error: All filenames taken */
	}

}
