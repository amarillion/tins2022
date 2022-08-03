#include "metrics.h"
#include "json.h"
#include "download.h"
#include "mainloop.h"
#include "util.h"
#include <ctime> /* time, time_t, difftime */

// for debugging only:
#include <iostream>

using namespace std;

#define METRICS_SERVER_HOST "https://www.helixsoft.nl"
// #define METRICS_SERVER_HOST "http://localhost:8000"
#define METRICS_URL METRICS_SERVER_HOST "/metrics/"

class MetricsImpl : public Metrics {
	
private:
	string gameId;
	string userId;
	string sessionId;
	string version;
	time_t startTime;
	vector<shared_ptr<WorkerThread>> workers;
	bool isDone = false;
	bool metricsEnabled = true;
public:
	MetricsImpl(const string &gameId, const string &version) {
		ALLEGRO_CONFIG *config = MainLoop::getMainLoop()->getConfig();
		cout << "Metrics enabled: " << metricsEnabled << endl;
		userId = MainLoop::getMainLoop()->getUserId();
		this->gameId = gameId;
		this->version = version; // read from package.json. 
		sessionId = generateRandomId();
		startTime = time(0);
	}

	~MetricsImpl() {
		assert (isDone); // must call done() before destructor.
	}

	void done() override {
		for (auto worker : workers) {
			worker->interruptAndWait();
		}
		isDone = true;
	}

	void logHelper(double secondsElapsed, const string &event) {
		if (!metricsEnabled) return;
		auto data = compileData(secondsElapsed, event);
		auto worker = postWorker(METRICS_URL, data.toString());
		worker->start();
		workers.push_back(worker);
	}

	JsonNode compileData(double secondsElapsed, const string &event) {
		JsonNode record(JSON_OBJECT);
		record.setString("event", event);
		record.setString("user", userId);
		record.setString("session", sessionId);
		record.setString("game", gameId);
		record.setString("version", version);
		record.setDouble("secondsElapsed", secondsElapsed);
		JsonNode result(JSON_ARRAY);
		result.arrayAdd(record);
		return result;
	}

	virtual void logSessionStart() override {
		logHelper (0, "Start session");	
	}

	virtual void logAchievement(const string &achievementId) override {
		double secondsElapsed = difftime(time(0), startTime);
		logHelper(secondsElapsed, achievementId);

	}
	virtual void logSessionClose() override {
		double secondsElapsed = difftime(time(0), startTime);
		logHelper(secondsElapsed, "Session closed");
	}

};

shared_ptr<Metrics> Metrics::newInstance(const string &gameId, const string &current_version_str)
{
	shared_ptr<Metrics> result = make_shared<MetricsImpl>(gameId, current_version_str);
	return result;
}
