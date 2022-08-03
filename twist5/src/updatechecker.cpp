#include <string>
#include "updatechecker.h"
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include "color.h"
#include <allegro5/allegro.h>
#include "text.h"
#include "download.h"
#include "strutil.h"
#include "fileutil.h"
#include <map>
#include <fstream>
#include "DrawStrategy.h"

using namespace std;

// TODO
const string cachebase = "page.out";

Version::Version(const string &versionStr)
{
	this->versionStr = versionStr;
	vector<string> fields = split(versionStr, '.');

	try
	{
		if (fields.size() > 0) major = stoi(fields[0]);
	} catch (const invalid_argument &e) { /* ignore */ }

	try
	{
		if (fields.size() > 1) minor = stoi(fields[1]);
	} catch (const invalid_argument &e) { /* ignore */ }

}

bool Version::operator>(const Version &that)
{
	return (this->major > that.major) ||
			(this->major == that.major &&
			this->minor > that.minor);
}

bool Version::operator<(const Version &that)
{
	return (this->major < that.major) ||
			(this->major == that.major &&
			this->minor < that.minor);
}

/**
 * Update information, read from cache or obtained from server.
 */
class UpdateInfo
{
private:
	Version latestVersion;
	Version currentVersion;
	bool newVersionAvailable;
	map<string, string> properties;
	string cacheFile;
public:
	const Version &getLatestVersion() { return latestVersion; }
	const Version &getCurrentVersion() { return currentVersion; }
	UpdateInfo(Version currentVersion, const string &cacheFile) : latestVersion("0.0.unknown"), currentVersion(currentVersion), newVersionAvailable(false), properties(), cacheFile(cacheFile) {}

	void reload() {
		// read cached properties file
		ifstream infile(cacheFile.c_str());

		int lineno = 0;
		string line;
		while (getline(infile, line))
		{
			lineno++;
			line = trim(line);
			vector<string> fields = split(line, '=');

			if (fields.size() == 2)
			{
				string &key = fields[0];
				string &value = fields[1];
				properties[key] = value;
			}
			else
			{
				// TODO
				cout << "couldn't parse " << line << endl;
			}
		}

		latestVersion = Version(properties["latestversion"]);
	}

	bool isNewerVersionAvailable() {
		return latestVersion > currentVersion;
	}
};

class UpdateCheckerImpl : public UpdateChecker
{
private:
	UpdateInfo info;
	string cacheDir;
	shared_ptr<WorkerThread> worker;
	bool inited = false;
	int nextAction;
public:

	static string getCacheFile(const string &cacheDir)
	{
		Path result = Path::parse(cacheDir).join(cachebase);
		return result.toString();
	}

	UpdateCheckerImpl(const string &cacheDir, const string &version_str, int nextAction) : info(Version(version_str), getCacheFile(cacheDir)), cacheDir(cacheDir), nextAction(nextAction)
	{
	}

	void buildGui()
	{
		add(ClearScreen::build(BLUE).get());
		add(Text::build(WHITE, "Visit\nhttp://www.helixgames.nl\nfor more awesome free games!")
			.layout(Layout::LEFT_TOP_RIGHT_H, 0, 50, 0, 200).get());

		if (info.isNewerVersionAvailable())
		{
			add(Text::build(ORANGE, "Version " + info.getLatestVersion().getVersionString() + " is now available!\n\nGo to http://www.helixgames.nl\nand download the update for free!")
				.layout(Layout::LEFT_TOP_RIGHT_H, 0, 160, 0, 200).get());
		}

		add(Text::build(WHITE, "press any key to continue")
			.layout(Layout::LEFT_BOTTOM_RIGHT_H, 0, 50, 0, 50).get());

	}

	virtual void onFocus() override
	{
		State::onFocus();
		if (!inited)
		{
			// refresh data if download has completed by now, otherwise rely on cached data
			if (worker->isDone())
			{
				info.reload();
			}

			buildGui();
			inited = true;
		}
	}

	virtual void handleEvent(ALLEGRO_EVENT &event) override
	{
		Container::handleEvent(event);

		if (event.type == ALLEGRO_EVENT_KEY_DOWN || event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{
			pushMsg(nextAction);
		}
	}

	virtual void start_check_thread(const string &game, const string &lang) override
	{
		stringstream urlss;
		urlss << UPDATE_SERVER_URL << "/updatecheck?software=" << game << "&version=" << info.getCurrentVersion().getVersionString() << "&lang=" << lang << "&platform=" << ALLEGRO_PLATFORM_STR;
		string url = urlss.str();
		worker = downloadWorker(url, getCacheFile(cacheDir));
		worker->start();
	}

	virtual void done() override
	{
		worker->interruptAndWait(); // signal that downloading should stop even if it's not completed			
	}

	virtual ~UpdateCheckerImpl() {
		done();
	}
};

shared_ptr<UpdateChecker> UpdateChecker::newInstance(const std::string &cacheDir, const std::string &current_version_str, int nextAction)
{
	shared_ptr<UpdateChecker> result = make_shared<UpdateCheckerImpl>(cacheDir, current_version_str, nextAction);
	return result;
}
