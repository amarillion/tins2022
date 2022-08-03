#include "fileutil.h"
#include <allegro5/allegro.h>
#include "strutil.h"
#include <fstream>

using namespace std;

/*
 *
 * TODO: needs work to detect drives from UNC
	The "drive letter" is only used on Windows, and is usually a string like "c:", but
	may be something like "\\Computer Name" in the case of UNC (Uniform Naming Convention) syntax.
 */
bool looksLikeDrive(const string &item)
{
	return (
			item.length() == 2 &&
			item[1] == ':' &&
			item[0] >= 'A' && item[0] <= 'Z'
	);
}

Path Path::parse(const string &s, char delim)
{
	bool isAbsolute = false;
	bool hasDriveComponent = false;
	vector<string> result;

	std::stringstream ss;
	ss.str(s);
	std::string item;

	bool first = true;
	while (std::getline(ss, item, delim))
	{
		if (first)
		{
			if (item == "") // i.e. starts with slash
			{
				isAbsolute = true;
			}

			if (looksLikeDrive(item))
			{
				isAbsolute = true;
				hasDriveComponent = true;
			}

			first = false;
		}

		if (item != "")
			result.push_back (item);
	}

	return Path(result, isAbsolute, hasDriveComponent);
}

Path Path::fromAllegroPath(ALLEGRO_PATH *path)
{
	Path result;

	result.fAbsolute = true; //TODO: not sure how to detect relative paths!

	const char *drive = al_get_path_drive(path);
	if (strlen(drive) > 0)
	{
		result.pathComponents.push_back(string(drive));
		result.fHasDriveComponent = true;
	}

	int num = al_get_path_num_components(path);
	for (int i = 0; i < num; ++i)
	{
		const char *component = al_get_path_component(path, i);
		result.pathComponents.push_back(string(component));
	}

	return result;
}

Path Path::getUserSettingsPath()
{
	ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	Path result = fromAllegroPath(path);
	al_destroy_path(path);
	return result;
}

Path::Path() : pathComponents(), fAbsolute(false), fHasDriveComponent(false) {}

Path::Path(const vector<string> &data, bool _fAbsolute, bool _hasDriveComponent) : pathComponents(data), fAbsolute(_fAbsolute), fHasDriveComponent(_hasDriveComponent) {}

Path Path::getParentPath() const
{
	if (pathComponents.size() == 0) return *this;

	auto first = pathComponents.begin();
	auto last = pathComponents.end()-1;
	return Path(vector<string>(first, last), fAbsolute, fHasDriveComponent);
}

Path Path::join (const std::string &tail, char sep) const
{
	Path tailPath = Path::parse(tail, sep);
	if (tailPath.isAbsolute())
	{
		throw exception(); // Tail component can not be absolute
	}

	auto result = pathComponents;
	result.insert(result.end(), tailPath.pathComponents.begin(), tailPath.pathComponents.end());
	return Path(result, fAbsolute, fHasDriveComponent);
}

std::string Path::toString(char pathSep) const
{
	std::stringstream ss;
	bool first = true;
	auto it = pathComponents.begin();

	if (it == pathComponents.end()) return "";

	if (fAbsolute)
	{
		if (fHasDriveComponent)
		{
			ss << (*it);
			it++;
		}
		ss << pathSep;
	}

	while (it != pathComponents.end())
	{
		if (first)
		{
			first = false;

		}
		else
		{
			ss << pathSep;
		}
		ss << (*it);
		it++;
	}
	return ss.str();
}

string Path::getDriveComponent() const
{
	if (fHasDriveComponent) { return pathComponents[0]; } else return "";
}


bool Path::fileExists() const
{
	// see discussion here: http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
	return std::ifstream(toString()).good();
}
