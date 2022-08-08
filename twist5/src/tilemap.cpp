#include "tilemap.h"
#include "assert.h"
#include <tegel5.h>
#include <allegro5/allegro.h>

#include <iostream> // debugging

Tilemap::~Tilemap() {
	teg_destroymap(map);
}

Tilemap *loadTilemap (const std::string &filename, TEG_TILELIST *tiles) {

	Tilemap *result = new Tilemap();
	JsonNode node = jsonParseFile(filename);
	result->rawData = node;
	result->filename = filename;
	
	ALLEGRO_FS_ENTRY *entry = al_create_fs_entry(filename.c_str());
	result->lastModified = al_get_fs_entry_mtime(entry);
	al_destroy_fs_entry(entry);

	int w = node.getInt("width");
	int h = node.getInt("height");

	int dl = 0;
	for (auto l : node.getArray("layers")) {
		if (l.getString("type") == "tilelayer") { dl++; }
	}
	assert(dl > 0);
	result->map = teg_createmap(dl, w, h, tiles);

	int ll = 0;
	for (auto l : node.getArray("layers")) {
		if (l.getString("type") != "tilelayer") { continue; }

		auto data = l.getArray("data");
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				// substract 1 to go from 1-based index intiled to 0-based index in tegel
				int val = data[x + (y * w)].getInt() - 1;
				teg_mapput(result->map, ll, x, y, val);
			}
		}
		ll++;
	}

	return result;
}
