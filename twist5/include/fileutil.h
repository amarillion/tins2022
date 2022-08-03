#pragma once

#include <vector>
#include <string>
#include <allegro5/allegro.h>

/**
 * representation of a path on the file system
 *
 * Paths are stored internally as a series of components, each component usually representing a directory.
 * Paths may be relative or absolute.
 *
 * The first component may also represent a drive letter such as "C:", in which case it is regarded absolute.
 *
 * The last component may represent either a file or a directory (The working of Path is indifferent to this)
 *
 * Path should work both on linux and windows file systems.
 * Paths do not store the path separator used, this defaults to ALLEGRO_NATIVE_PATH_SEP,
 * but may be overridden on conversion from / to string.
 *
 * Path is immutable, all member functions return a copy or a newly constructed Path.
 */
struct Path
{
private:
	std::vector<std::string> pathComponents;
	bool fAbsolute;
	bool fHasDriveComponent;
public:
	Path();
	Path(const std::vector<std::string> &pathComponents, bool fAbsolute, bool hasDriveComponent);
	static Path parse(const std::string &input, char separator = ALLEGRO_NATIVE_PATH_SEP);

	/** return a new Path in the application data directory for this application,
	 * corresponding to al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH)
	 */
	static Path getUserSettingsPath();

	/*
	 * Convert ALLEGRO_PATH structure to this class
	 * TODO: at the moment, only works reliably for absolute paths, needs some work for relative paths.
	 */
	static Path fromAllegroPath(ALLEGRO_PATH *path);

	/** return a new path representing the parent directory of the current file or directory */
	Path getParentPath() const;

	/** return a new path representing the concatenation of two paths. */
	Path join (const std::string &tail, char separator = ALLEGRO_NATIVE_PATH_SEP) const;

	std::string toString(char pathSep = ALLEGRO_NATIVE_PATH_SEP) const;
	std::string getDriveComponent() const;
	bool hasDriveComponent() { return fHasDriveComponent; }
	bool isAbsolute() const { return fAbsolute; }
	bool isRelative() const { return !fAbsolute; }
	bool fileExists() const;
};
