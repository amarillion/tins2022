#pragma once

#include <iostream>

struct ALLEGRO_MUTEX;
struct ALLEGRO_THREAD;

class WorkerThread {
private:
	ALLEGRO_MUTEX *mutex = nullptr;
	ALLEGRO_THREAD *thr = nullptr;
	bool started = false; // flag protected by mutex 
	bool done = false; // flag protected by mutex
	bool interrupted = false; // flag protected by mutex
	friend void *call_worker_run(ALLEGRO_THREAD *thread, void *arg);
public:
	WorkerThread();
	virtual ~WorkerThread(); // implicit join thread.
protected:
	void setDone(bool value);
	void setStarted(bool value);
	void setInterrupted(bool value);
public:
	/** returns true if the worker thread has completed, false otherwise */
	bool isDone();
	bool isInterrupted();
	bool isStarted();

	/* starts the worker thread and calls run() on the other thread */
	int start();

	/**
	 * wait for the worker thread to finish to completion
	 */
	void waitForCompletion();

	/* 
		May be called by GUI thread to attempt to stop worker thread.
		Sets the should stop flag on the worker thread, 
		and also calls onInterrupt() to peform additional custom actions.
		Then waits for the thread to finish.
	*/
	void interruptAndWait();

protected:
	/* 
		override, but do not call directly! This will be called after start() on other thread.
		If possible, should call isInterrupted() to find out if an interrupt is requested.
	*/
	virtual void run() = 0;

	/* 
		override if you want to perform additional action to attempt to interrupt the other thread 
	 */
	virtual void onInterrupt() {};
};
