#include "data.h"

void DataWrapper::FireEvent (int code)
{
	// notify all listeners of event
	for (auto i : listeners)
	{
		i->changed (code);
	}

	for (auto i : listenerFunctions) {
		i.second(code);
	}
}

void DataWrapper::AddListener (DataListener* listener)
{
	listeners.push_back (listener);
}

void DataWrapper::RemoveListener (DataListener *listener)
{
	listeners.remove (listener);
}

void DataWrapper::RemoveListener (int handle) {
	auto it = listenerFunctions.find(handle);
	if (it != listenerFunctions.end()) {
		listenerFunctions.erase (it);
	}
}

int DataWrapper::AddListener (const DataListenerFunction &listener)
{
	int result = nextFunctionHandle;
	listenerFunctions[nextFunctionHandle] = listener;
	nextFunctionHandle++;
	return result;
}

void ListWrapper::FireEvent (int code, int pos)
{
	// notify all listeners of event
	for (auto i : listenerFunctions) {
		i.second(code, pos);
	}
}

void ListWrapper::RemoveListener (int handle) {
	auto it = listenerFunctions.find(handle);
	if (it != listenerFunctions.end()) {
		listenerFunctions.erase (it);
	}
}

int ListWrapper::AddListener (const ListListenerFunction &listener)
{
	int result = nextFunctionHandle;
	listenerFunctions[nextFunctionHandle] = listener;
	nextFunctionHandle++;
	return result;
}
