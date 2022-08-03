#include "util.h"
#include <allegro5/allegro.h>

#ifndef ALLEGRO_ANDROID
#include <allegro5/allegro_native_dialog.h>
#endif

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <cstdio>
#include <string>
#include <vector>
#include "strutil.h"
#include <iostream>

using namespace std;

int random (int i)
{
	return rand() % i;
}

void log(const char* msg, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, sizeof(buf), msg, ap);
	va_end (ap);

	// log to stderr
	cerr << buf << endl;
}

void allegro_message(const char *msg, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, sizeof(buf), msg, ap);
	va_end (ap);

	// log to stderr
	cerr << buf << endl;
#ifndef ALLEGRO_ANDROID
	al_show_native_message_box (al_get_current_display(), "error", "error", buf, NULL, ALLEGRO_MESSAGEBOX_ERROR);
#endif
}

string get_config_string(ALLEGRO_CONFIG *config, const char *section, const char *key, const char *defaultValue)
{
	const char *value = al_get_config_value (config, section, key);
	string result = defaultValue;
	if (value != NULL)
	{
		return string (value); //TODO - error handling
	}
	else
	{
		return string (defaultValue);
	}
}

int get_config_int(ALLEGRO_CONFIG *config, const char *section, const char *key, int defaultValue)
{
	const char *value = al_get_config_value (config, section, key);
	int result = defaultValue;
	if (value != NULL)
	{
		result = atoi(value); //TODO - error handling
	}
	return result;
}

void set_config_int(ALLEGRO_CONFIG *config, const char *section, const char *key, int value)
{
	char buffer[256];
	snprintf (buffer, sizeof(buffer), "%i", value);
	al_set_config_value (config, section, key, buffer);
}

void expandGlobHelper (string glob, string base, vector<string> &result);

/**
 * Expand a unix GLOB, such as '~' or '*.java' into a list of files.
 * only files or directories that actually exist are returned. If there are no matches,
 * returns an empty list.
 *
 * <p>
 * TODO only tested on unix, not likely to work well on Windows.
 *
 * @param glob the glob pattern
 * @param baseDir if the glob is relative, it will be taken relative to this Directory. If it is absolute, this parameter has no effect.
 *        If baseDir is null, the current directory will be used.
 * @return list of files or directories, or an empty list if there are no matches.
 */
void glob (const string &globVal, const string &baseDir, vector<string> &result)
{
	string base;
	string globStr = globVal;
	if (globStr[0] == '/')
	{
		base = "/";
		globStr = globStr.substr(1); //TODO substr
	}
	//TODO
//	else if (glob == ("~") || glob.startsWith ("~/"))
//	{
//		base = new File (System.getProperty("user.home"));
//		glob = glob.substring(1);
//	}
	else
	{
		// relative - use supplied base Directory (if it isn't null)
//		base = baseDir == null ? new File(".") : baseDir;  //TODO
		base = baseDir;
	}

	expandGlobHelper (globStr, base, result);
}

/** expand glob relative to current directory */
void glob (const string &globVal, vector<string> &result)
{
	string baseDir = ".";
	glob (globVal, baseDir, result);
}

/**
 * Helper for expandGlob.
 * <br>
 * Tests if a given file name matches a given glob pattern with * or ?.
 * Does not handle directories.
 */
bool localGlobMatches(const string &text, const string &globVal)
{
	string globStr = globVal;

	string rest = ""; bool restValid = false;

	string::size_type pos = globStr.find('*');
	if (pos != string::npos) {
		rest = globStr.substr(pos + 1); //TODO substr
		restValid = true;
		globStr = globStr.substr(0, pos); //TODO substr
	}

	if (globStr.length() > text.length())
		return false;

	// handle the part up to the first *
	for (string::size_type i = 0; i < globStr.length(); i++)
		if (globStr[i] != '?'
				&& !equalsIgnoreCase (globStr.substr(i, i + 1), text.substr(i, i + 1))) //TODO substr 2x
			return false;

	// recurse for the part after the first *, if any
	if (!restValid) {
		return globStr.length() == text.length();
	} else {
		for (string::size_type i = globStr.length(); i <= text.length(); i++) {
			if (localGlobMatches(text.substr(i), rest)) //TODO substr
				return true;
		}
		return false;
	}
	return false;
}

/**
 * Helper for expandGlob.
 * <br>
 * get a list of files matching a glob in a directory. Does not traverse directories.
 */
void getLocalMatches (string remain, string base, vector<string> &result)
{
	if (remain == ".")
	{
		result.push_back (base);
	}
	//TODO
//	else if (remain == "..")
//	{
//		result.push_back (base.getAbsoluteFile().getParentFile());
//	}
	else if (contains(remain, "*") || contains (remain, "?"))
	{
		// look for directories in base matching
		//TODO
		vector<string> temp;
		listDir(base, temp);

		for (vector<string>::iterator child = temp.begin(); child != temp.end(); ++child)
		{
			if (localGlobMatches (*child /* .getName() TODO */, remain))
			{
				result.push_back (*child);
			}
		}

	}
	else
	{
		//TODO: path separator?
		string f = base + "/" + remain;
		if (al_filename_exists(f.c_str()))
		{
			result.push_back(f);
		}
	}
}

/**
 * list contents of dir, ignoring any wildcard patterns
 */
void listDir(const string &dir, vector<string> &result)
{
	ALLEGRO_FS_ENTRY *entry = al_create_fs_entry(dir.c_str());

	if (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) {
		ALLEGRO_FS_ENTRY *next;

		if (!al_open_directory(entry))
		{
			return; // directory doesn't exist, nothing added to result.
		}


		while (1) {
			next = al_read_directory(entry);
			if (!next)
				break;

			const char *name = al_get_fs_entry_name(next);
			result.push_back (string(name));

			al_destroy_fs_entry(next);
		}
		al_close_directory(entry);
	}

	al_destroy_fs_entry(entry);

}

bool isDirectory (string &path)
{

	ALLEGRO_FS_ENTRY *e = al_create_fs_entry(path.c_str());
	uint32_t flags = al_get_fs_entry_mode(e);
	al_destroy_fs_entry(e);
	return (flags | ALLEGRO_FILEMODE_ISDIR) > 0;
}

void expandGlobHelper (string glob, string base, vector<string> &result)
{
	// deal with sequences of slashes...
	while (startsWith ("/", glob))
	{
		glob = glob.substr(1); // substr OK
	}

	// if nothing remains, then we only select the current directory.
	if (glob == "")
	{
		result.push_back (base);
	}

	string::size_type pos = glob.find ('/');
	if (pos != string::npos)
	{
		string dir = glob.substr (0, pos); // substr OK
		glob = glob.substr (pos); //TODO: pos..length
		vector<string> temp;
		getLocalMatches (dir, base, temp);
		for (vector<string>::iterator f = temp.begin(); f != temp.end(); ++f)
		{
			if (isDirectory(*f))
			{
				expandGlobHelper(glob, *f, result);
			}
		}
	}
	else
	{
		getLocalMatches(glob, base, result);
	}
}

void clip_with_transform(float cx1, float cy1, float cx2, float cy2)
{
    const ALLEGRO_TRANSFORM *trans = al_get_current_transform();
    al_transform_coordinates (trans, &cx1, &cy1);
    al_transform_coordinates (trans, &cx2, &cy2);
    al_set_clipping_rectangle (cx1, cy1, cx2, cy2);
}

string generateRandomId() {
	// NB: buffer must be 17 or longer
	char buffer[255];
	char *pos = buffer;
	for (int i = 0; i < 4; ++i) {
		// format hex to string, C-style 
		sprintf(pos, "%04x", rand() % 0xFFFF);
		pos += 4;
	}
	return string(buffer);
}

int pickWeightedRandom(vector<int> const &actionWeights)
{
	int sum = 0;
	for (int val : actionWeights)
	{
		assert(val >= 0);
		sum += val;
	}

	// make a weighted random pick
	int choice = rand() % sum;
	int pickedAction = 0;
	for (size_t i = 0; i < actionWeights.size(); ++i)
	{
		if (choice > actionWeights[i])
		{
			choice -= actionWeights[i];
		}
		else
		{
			pickedAction = i;
			break;
		}
	}
	return pickedAction;
}
