#include "workerthread.h"
#include <allegro5/allegro.h>

#include <iostream>
using namespace std;

void *call_worker_run(ALLEGRO_THREAD *thread, void *arg)
{
	// Runs on worker thread.
	WorkerThread *worker = (WorkerThread*)arg;
	worker->run();
	worker->setDone(true);
	return nullptr;
}

WorkerThread::WorkerThread() {
	mutex = al_create_mutex();
}

WorkerThread::~WorkerThread() {
	if (thr != nullptr) {
		cerr << "WorkerThread destructor called before it was completed.\nMake sure to call waitForCompletion()" << endl;
	}
	al_destroy_mutex(mutex);
}

/** thread safe */
void WorkerThread::setDone(bool value) {
	al_lock_mutex(mutex);
	done = value;
	al_unlock_mutex(mutex);
}

/** thread safe */
void WorkerThread::setStarted(bool value)
{
	al_lock_mutex(mutex);
	started = value;
	al_unlock_mutex(mutex);
}

void WorkerThread::setInterrupted(bool value)
{
	al_lock_mutex(mutex);
	interrupted = value;
	al_unlock_mutex(mutex);
}

bool WorkerThread::isInterrupted() {
	bool result;
	al_lock_mutex(mutex);
	result = interrupted;
	al_unlock_mutex(mutex);
	return result;
}

/** thread safe */
bool WorkerThread::isDone()
{
	bool result;
	al_lock_mutex(mutex);
	result = done;
	al_unlock_mutex(mutex);
	return result;
}

/** thread safe */
bool WorkerThread::isStarted()
{
	bool result;
	al_lock_mutex(mutex);
	result = started;
	al_unlock_mutex(mutex);
	return result;
}

void WorkerThread::interruptAndWait() {
	if (thr == nullptr) return;

	// set the flag that hopefully the other thread will listen to...
	setInterrupted(true);

	// perform additional custom action to stop the thread
	onInterrupt();

	// now wait for it all to finish
	al_destroy_thread(thr); // implicit join
	thr = nullptr;
}

void WorkerThread::waitForCompletion() {
	if (thr != nullptr) {
		al_destroy_thread(thr); // implicit join!
		thr = nullptr;
	}
}

int WorkerThread::start() {
	thr = al_create_thread (call_worker_run, this);
	al_start_thread(thr);
	setStarted(true);
	return 0;
}
