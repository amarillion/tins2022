#include <assert.h>
#include "resources.h"
#include "color.h"
#include "dom.h"
#include "util.h"
#include "strutil.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include "mainloop.h"
#include "json.h"
#include "anim.h"
#include "tilemap.h"

using namespace std;
using namespace xdom;

class ResourceException: public exception
{
private:
	std::string msg;
public:
	ResourceException(const std::string &msg) : msg(msg) {}
	virtual const char* what() const throw()
	{
		return msg.c_str();
	}
};

class BitmapFontWrapper : public FontWrapper {
private:
	ALLEGRO_FONT *wrapped;
public:
	BitmapFontWrapper(ALLEGRO_FONT *wrapped) : wrapped(wrapped) {}
	virtual ALLEGRO_FONT *get(int size = 12) override {
		return wrapped;
	}
	virtual ~BitmapFontWrapper() {
		al_destroy_font(wrapped);
	}
};

class TtfWrapper : public FontWrapper {
private:
	std::map<int, ALLEGRO_FONT*> fonts {};
	std::string fname;
public:
	TtfWrapper(const std::string &fname): fname(fname) {}

	virtual ALLEGRO_FONT *get(int size = 12) override {
		if (fonts.find(size) == fonts.end()) {
			ALLEGRO_FONT *font = al_load_font(fname.c_str(), size, 0);
			if (!font) {
				throw ResourceException(string_format("error loading TTF '%s' with size %i", fname.c_str(), size));
			}
			fonts[size] = font;
		}
		return fonts[size];
	}
	virtual ~TtfWrapper() {
		for(auto &pair: fonts) {
			al_destroy_font(pair.second);
		}
	}
};

class ResourcesImpl : public Resources
{
private:
	char errorMsg[1024];
	std::map <std::string, ALLEGRO_BITMAP*> sprites; // seperately loaded bitmaps
	std::map <std::string, Anim*> animlist;
	std::map <std::string, std::shared_ptr<FontWrapper>> fonts;

	std::map <std::string, Tilemap*> jsonMaps; // seperately loaded maps
	std::map <std::string, TEG_TILELIST*> tilelists;

	std::map <std::string, ALLEGRO_SAMPLE *> samples;
	std::map <std::string, ALLEGRO_AUDIO_STREAM*> duhlist;
	std::map <std::string, std::string> textFiles;
	std::map <std::string, JsonNode> jsonFiles;
public:

	virtual void putAnim (const string &id, Anim *val)
	{
		animlist.insert (pair<string, Anim*>(id, val));
	}
	virtual const std::map<std::string, Anim*> &getAnims() const override {
		return animlist;
	}

	virtual Anim *getAnimIfExists (const string &id) override
	{
		if (animlist.find(id) == animlist.end()) return nullptr; else return animlist[id];
	}

	virtual Anim *getAnim (const string &id) override
	{
		if (animlist.find (id) == animlist.end())
		{
			allegro_message ("Couldn't find Animation '%s'", id.c_str());
			exit(1);
		}
		else
		{
			return animlist[id];
		}
	}

	virtual const char *getErrorMsg() const override {
		return errorMsg;
	}

	virtual ALLEGRO_SAMPLE *getSampleIfExists(const std::string &id) override
	{
		if (samples.find(id) == samples.end())
			return NULL;
		else
			return samples[id];
	}

	virtual ALLEGRO_BITMAP *getBitmapIfExists(const std::string &id)  override { 
		if (sprites.find(id) == sprites.end()) return NULL; else return sprites[id]; 
	}

	virtual void putBitmap(const string &id, ALLEGRO_BITMAP *bmp) override {
		sprites.insert (pair<string, ALLEGRO_BITMAP *>(id, bmp));
	}

	// TODO: utility function?
	void split_path (const string &in, string &basename, string &extension)
	{
		size_t start = in.find_last_of ("\\/");
		if (start == string::npos) start = 0;
		else start += 1;

		size_t end = in.find_last_of ('.');
		if (end == string::npos) end = in.length();

		extension = in.substr (end + 1);
		basename = in.substr(start, end-start);

	}

	// returns 0 on failure, and sets error message.
	virtual int addSingleFile(const string &i) override
	{
		string basename;
		string extension;

		split_path (i, basename, extension);

		// must have an extension, to be able to recognize file type.
		//TODO: error message?
		if (extension == "") return 0;
		else if (equalsIgnoreCase (extension, "tll"))
		{
			TEG_TILELIST *temp = teg_loadtiles(i.c_str());
			if (!temp)
			{
				snprintf(errorMsg, sizeof(errorMsg), "error load TLL %s with error %s", i.c_str(), teg_error);
				return 0;
			}
			else
			{
				tilelists[basename] = temp;
			}
		}
		else if (equalsIgnoreCase (extension, "ttf"))
		{
			fonts[basename] = make_shared<TtfWrapper>(i);
		}
		else if (equalsIgnoreCase (extension, "tga"))
		{
			ALLEGRO_FONT *temp;
			if (!(temp = al_load_bitmap_font(i.c_str())))
			{
				snprintf(errorMsg, sizeof(errorMsg), "error load AlFont %s", i.c_str());
				return 0;
			} else {
				fonts[basename] = make_shared<BitmapFontWrapper>(temp);
			}
		}
		else if (equalsIgnoreCase (extension, "mod") || equalsIgnoreCase (extension, "xm"))
		{
			ALLEGRO_AUDIO_STREAM *temp;
			if (!(temp = al_load_audio_stream (i.c_str(), 4, 2048))) //TODO: correct values for al_load_audio_stream
			{
				snprintf(errorMsg, sizeof(errorMsg), "error loading DUMB %s", i.c_str());
				return 0;
			}
			else {
				al_set_audio_stream_playmode(temp, ALLEGRO_PLAYMODE_LOOP );
				assert (duhlist.find(basename) == duhlist.end()); // fails if you overload the same id.
				duhlist[basename] = temp;
			}
		}
		else if (equalsIgnoreCase  (extension, "ogg"))
		{
           ALLEGRO_SAMPLE *sample_data = NULL;
           sample_data = al_load_sample(i.c_str());
           if (!sample_data)
           {
        	   log ("error loading OGG %s", i.c_str());
        	    //TODO: write to log but don't quit. Sound is not essential.
//				snprintf(errorMsg, sizeof(errorMsg), "error loading OGG %s", i.c_str());
//				return 0;
           }
           else
           {
        	   samples[basename] = sample_data;
           }
		}
		else if (equalsIgnoreCase (extension, "bmp") || equalsIgnoreCase (extension, "png"))
		{
			ALLEGRO_BITMAP *bmp;
			bmp = al_load_bitmap (i.c_str());
			if (!bmp)
			{
				snprintf(errorMsg, sizeof(errorMsg), "error loading BMP/PNG %s", i.c_str());
				return 0;
			}
			else
			{
				assert (al_get_bitmap_width(bmp) > 0);
				assert (al_get_bitmap_height(bmp) > 0);
				sprites[basename] = bmp;
			}
		}
		else if (equalsIgnoreCase (extension, "wav"))
		{
			ALLEGRO_SAMPLE *wav;
			wav = al_load_sample (i.c_str());
			if (!wav)
			{
				snprintf(errorMsg, sizeof(errorMsg), "error loading WAV %s", i.c_str());
				return 0;
			}
			else
			{
				samples[basename] = wav;
			}
		}
		else if (equalsIgnoreCase (extension, "json"))
		{
			try {
				jsonFiles[basename] = jsonParseFile(i);
			} catch (const JsonException &ex) {
				snprintf(errorMsg, sizeof(errorMsg), "error loading JSON %s", i.c_str());
				return 0;
			}
		}
		else if (equalsIgnoreCase (extension, "xml"))
		{
			DomNode node = xmlParseFile(i);

			vector<DomNode>::iterator n;
			for (n = node.children.begin(); n != node.children.end(); ++n)
			{
				if (n->name == "anim")
					Anim::loadFromXml (*n, this, animlist);
			}
		}
		else if (equalsIgnoreCase (extension, "glsl")) {
			string result("");

			char buffer[4096];
			ALLEGRO_FILE *fp = al_fopen(i.c_str(), "r");
			while(al_fgets(fp, buffer, 4096)) {
				result += buffer;
			}
			al_fclose(fp);

			textFiles[basename] = result;
		}

		return 1;
	}

	virtual int addDir(const char *dir) override
	{
		//TODO: use listDir function from util.cpp

		ALLEGRO_FS_ENTRY *entry = al_create_fs_entry(dir);

		if (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) {
			ALLEGRO_FS_ENTRY *next;

			al_open_directory(entry);

			while (1) {
				next = al_read_directory(entry);
				if (!next)
					break;

				string name(al_get_fs_entry_name(next));
				addSingleFile (name);

				al_destroy_fs_entry(next);
			}
			al_close_directory(entry);
		}

		al_destroy_fs_entry(entry);

		return 0;
	}

	virtual int addFiles(const char *pattern) override
	{
		vector<string> filenames;
		glob (pattern, filenames);
		if (filenames.size() == 0) {
			snprintf(errorMsg, sizeof(errorMsg), "No files matching pattern %s", pattern);
			return 0;
		}
		for (string &i : filenames)
		{
			int result = addSingleFile (i);
			if (result == 0) return 0; // error while loading a file.
		}
		return 1; // success
	}

	//TODO: rename to getMusic
	virtual ALLEGRO_AUDIO_STREAM *getMusic (const string &id) override
	{
	    if (duhlist.find (id) == duhlist.end())
	    {
			throw (ResourceException(string_format("Couldn't find DUH (Music) '%s'", id.c_str())));
	    }
	    return duhlist[id];
	}

	virtual ~ResourcesImpl()
	{
		{
			map<string, ALLEGRO_BITMAP*>::iterator i;
			for (i = sprites.begin(); i != sprites.end(); i++)
			{
				al_destroy_bitmap (i->second);
			}
		}
		{
			map<string, Anim*>::iterator i;
			for (i = animlist.begin(); i != animlist.end(); i++)
			{
				delete (i->second);
			}
		}
		{
			map<string, ALLEGRO_AUDIO_STREAM*>::iterator i;
			for (i = duhlist.begin(); i != duhlist.end(); i++)
			{
				al_destroy_audio_stream(i->second);
			}
		}
			{
				map<string, TEG_TILELIST*>::iterator i;
				for (i = tilelists.begin(); i != tilelists.end(); i++)
				{
					teg_destroytiles (i->second);
				}
			}
			{
				map<string, ALLEGRO_SAMPLE *>::iterator i;
				for (i = samples.begin(); i != samples.end(); i++)
				{
					al_destroy_sample (i->second);
				}
			}

	}

	virtual string getTextFile (const string &id) override {
		if (textFiles.find (id) == textFiles.end()) {
			throw(ResourceException(string_format("Couldn't find Text file '%s'", id.c_str())));
		}

		return textFiles[id];
	}

	virtual const JsonNode &getJson(const std::string &id) override {
		if (jsonFiles.find(id) == jsonFiles.end()) {
			throw(ResourceException(string_format("Couldn't find JSON file '%s'", id.c_str())));
		}

		return jsonFiles[id];
	}

	virtual ALLEGRO_BITMAP *getBitmap (const string &id) override
	{
		if (sprites.find (id) == sprites.end())
		{
			throw(ResourceException(string_format("Couldn't find Bitmap sprite '%s'", id.c_str())));
		}

		return sprites[id];
	}

	virtual const shared_ptr<FontWrapper> getFont(const string &id) override {
		if (fonts.find (id) == fonts.end())
		{
			// auto-vivicate fixed font...
			if (id == "builtin_font") {
				ALLEGRO_FONT* ff = al_create_builtin_font();
				auto result = make_shared<BitmapFontWrapper>(ff);
				fonts["builtin_font"] = result;
				return result;
			}
			else
			{
				throw(ResourceException(string_format("Couldn't find FONT '%s'", id.c_str())));
			}
		}
		else
			return fonts[id];
	}

	virtual Tilemap *getJsonMap (const string &id) override {
		if (jsonMaps.find(id) == jsonMaps.end()) {
			allegro_message ("Couldn't find MAP '%s'", id.c_str());
			exit(1);
		}
		return jsonMaps[id];
	}

	virtual TEG_TILELIST *getTilelist (const string &id) override {
		if (tilelists.find (id) == tilelists.end()) {
			allegro_message ("Couldn't find TILELIST '%s'", id.c_str());
			exit(1);
		}

		return tilelists[id];
	}

	virtual int addJsonMapFile(const string &filename, const string &tilesname) override {
		string basename;
		string extension;
		split_path (filename, basename, extension);
		return addJsonMapFile(basename, filename, tilesname);
	}

	virtual int addJsonMapFile(const string &id, const string &filename, const string &tilesname) override {
		TEG_TILELIST *tiles = getTilelist (tilesname);
		if (!tiles)
		{
			snprintf(errorMsg, sizeof(errorMsg), "Could not find tiles named [%s]", tilesname.c_str());
			return 0;
		}

		Tilemap *result = loadTilemap(filename, tiles);

		jsonMaps[id] = result;
		return 1;
	}

	virtual int addStream(const string &id, const string &filename) override {
		ALLEGRO_AUDIO_STREAM *temp;
		if (!(temp = al_load_audio_stream (filename.c_str(), 4, 2048))) //TODO: correct values for al_load_audio_stream
		{
			snprintf(errorMsg, sizeof(errorMsg), "error loading Stream %s", id.c_str());
			return 0;
		}
		else {
			al_set_audio_stream_playmode(temp, ALLEGRO_PLAYMODE_LOOP );
			// al_set_audio_stream_playing(temp, false); //TODO???
			// al_attach_audio_stream_to_mixer(temp, al_get_default_mixer()); // TODO?
			assert (duhlist.find(id) == duhlist.end()); // fails if you overload the same id.
			duhlist[id] = temp;
		}
		
		return 1;
	}

	virtual ALLEGRO_SAMPLE *getSample (const string &id) override
	{
		if (samples.find (id) == samples.end())
		{
			throw (ResourceException(string_format("Couldn't find SAMPLE '%s'", id.c_str())));
		}

		return samples[id];
	}

};

shared_ptr<Resources> Resources::newInstance() {
	return make_shared <ResourcesImpl>();
}