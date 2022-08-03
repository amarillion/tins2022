#pragma once

#include <memory>
#include "state.h"

class UpdateCheckerImpl;

#ifndef UPDATE_SERVER_URL
#define UPDATE_SERVER_URL "http://www.helixsoft.nl"
//#define UPDATE_SERVER_URL "http://localhost:8000"
#endif


class UpdateChecker : public State
{
public:
	virtual void done() = 0;
	virtual void start_check_thread(const std::string &game, const std::string &lang) = 0;

    // factory
    static std::shared_ptr<UpdateChecker> newInstance(const std::string &cacheDir, const std::string &current_version_str, int nextAction);
	virtual std::string const className() const override { return "UpdateChecker"; }
};


class Version
{
private:
	std::string versionStr;
	int major = 0;
	int minor = 0;
public:
	const std::string &getVersionString() const { return versionStr; };
	int getMajor() { return major; }
	int getMinor() { return minor; }

	explicit Version(const std::string &versionStr);
	bool operator>(const Version &that);
	bool operator<(const Version &that);
};
