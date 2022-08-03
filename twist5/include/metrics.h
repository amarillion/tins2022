#pragma once

#include <string>
#include <memory>

class Metrics {
public:
	virtual void logSessionStart() = 0;
	virtual void logAchievement(const std::string &achievementId) = 0;
	virtual void logSessionClose() = 0;
	virtual void done() = 0;
    static std::shared_ptr<Metrics> newInstance(const std::string &gameId, const std::string &current_version_str);
};
