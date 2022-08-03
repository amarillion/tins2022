#pragma once

#include <string>
#include <allegro5/allegro.h>

template <class T>
struct Tag {};

int parse(Tag<int> tag, const char *value);
bool parse(Tag<bool> tag, const char *value);
float parse(Tag<float> tag, const char *value);
const char* parse(Tag<const char*> tag, const char *value);

std::string store(Tag<int> tag, int val);
std::string store(Tag<bool> tag, bool val);
std::string store(Tag<float> tag, float val);
std::string store(Tag<const char*> tag, const char* val);

template <class T>
T get_config(Tag<T>, ALLEGRO_CONFIG *config, const char *section, const char *key, T defaultValue) {
	const char *value = al_get_config_value (config, section, key);
	T result;
	if (value != nullptr) {
		result = parse(Tag<T>{}, value); //TODO - error handling
	} 
	else {
		result = defaultValue;
	}
	return result;
}

template <class T>
void set_config(Tag<T>, ALLEGRO_CONFIG *config, const char *section, const char *key, T val) {
	std::string data = store(Tag<T>{}, val);
	al_set_config_value(config, section, key, data.c_str());
}

template <class T, class ConfigProvider>
class Setting {
private:
	ConfigProvider *parent;
	const char *section;
	const char *key;
	T defaultValue;
public:
	Setting(ConfigProvider *parent, const char *section, const char *key, T defaultValue) :
		parent(parent), section(section),
		key(key), defaultValue(defaultValue) {}

	T get() {
		ALLEGRO_CONFIG *config = parent->getConfig();
		assert(config);
		return get_config(Tag<T>{}, config, section, key, defaultValue);
	};

	void set(T val) {
		ALLEGRO_CONFIG *config = parent->getConfig();
		assert(config);
		set_config(Tag<T>{}, config, section, key, val);
	}
};

