#include "setting.h"

int parse(Tag<int> tag, const char *value) {
	//TODO: error handling is somewhat impossible. Atoi returns 0 on any error, or when the string is 0
	return atoi(value);
}

bool parse(Tag<bool> tag, const char *value) {
	//TODO: error handling is somewhat impossible. Atoi returns 0 on any error, or when the string is 0
	return atoi(value);
}

float parse(Tag<float> tag, const char *value) {
	return atof(value);
}

const char* parse(Tag<const char*> tag, const char *value ) {
	return value;
}

std::string store(Tag<int> tag, int val) {
	return std::to_string(val);
}

std::string store(Tag<bool> tag, bool val) {
	return std::to_string(val ? 1 : 0);
}

std::string store(Tag<float> tag, float val) {
	return std::to_string(val);
}

std::string store(Tag<const char *> tag, const char *val) {
	return std::string(val);
}
