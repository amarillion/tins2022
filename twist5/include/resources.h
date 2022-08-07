#pragma once

#include <string>
#include <map>
#include <exception>
#include <memory>
#include "bitmapProvider.h"
#include "json.h"
#include <tegel5.h>

class Tilemap;

class Anim;
struct ALLEGRO_FONT;
struct ALLEGRO_SAMPLE;
struct ALLEGRO_AUDIO_STREAM;

class FontWrapper {
public:
	virtual ALLEGRO_FONT *get(int size = 12) = 0;
};

class ResourceException: public std::exception
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

class Resources : public IBitmapProvider
{
public:
	static std::shared_ptr<Resources> newInstance();
	
	/** adds all files that match glob pattern.
	 * returns ResourceException on failure (no files found, or error while loading file).
	 */
	virtual void addFiles(const char *pattern) = 0;
	virtual void addDir(const char *dir) = 0;

	virtual Anim *getAnim (const std::string &id) = 0;
	virtual Anim *getAnimIfExists (const std::string &id) = 0;
	virtual void putAnim (const std::string &id, Anim *val) = 0;
	virtual const std::map<std::string, Anim*> &getAnims() const = 0;

	virtual const std::shared_ptr<FontWrapper> getFont (const std::string &id) = 0;

	virtual void putBitmap(const std::string &id, ALLEGRO_BITMAP *bmp) = 0;
	virtual void addSingleFile(const std::string &i) = 0;
	virtual ALLEGRO_BITMAP *getBitmap (const std::string &id) = 0;

	virtual ALLEGRO_BITMAP *getBitmapIfExists(const std::string &id) = 0;
	virtual ALLEGRO_SAMPLE *getSample (const std::string &id) = 0;
	virtual ALLEGRO_SAMPLE *getSampleIfExists (const std::string &id) = 0;
	virtual const JsonNode &getJson(const std::string &id) = 0;

	virtual void addJsonMapFile (const std::string &id, const std::string &filename, const std::string &tilesname) = 0;
	virtual void addJsonMapFile (const std::string &filename, const std::string &tilesname) = 0;

	virtual Tilemap *getJsonMap (const std::string &id) = 0;
	virtual TEG_TILELIST *getTilelist (const std::string &id) = 0;

	virtual std::string getTextFile(const std::string &id) = 0;
	virtual ALLEGRO_AUDIO_STREAM *getMusic (const std::string &id) = 0;
	virtual void addStream(const std::string &id, const std::string &filename) = 0;
	virtual ~Resources() {}

	virtual void refreshModifiedFiles() = 0;
};
