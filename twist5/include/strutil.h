#pragma once

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string>
#include <vector>
#include <sstream>

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

static inline bool contains (const std::string &haystack, const std::string &substring)
{
	return haystack.find(substring) != std::string::npos;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

static inline std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> result;
	std::stringstream ss;
	ss.str(s);
	std::string item;

	while (std::getline(ss, item, delim))
	{
		result.push_back (item);
	}
	return result;
}

static inline std::string join(const std::vector<std::string> &elts, const std::string &sep)
{
	std::stringstream ss;
	bool first = true;
	for (auto elt : elts)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			ss << sep;
		}
		ss << elt;
	}
	return ss.str();
}

// check if argument starts with prefix
static inline bool startsWith(const std::string &prefix, const std::string &argument)
{
	return (argument.substr(0, prefix.size()) == prefix);
}

static inline std::string toUpper(const std::string &str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

static inline std::string toLower(const std::string &str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

static inline bool equalsIgnoreCase (const std::string &one, const std::string &other)
{
	return toUpper (one) == toUpper (other);
}

// source: http://stackoverflow.com/a/26310318/3306
template<typename... Args>
std::string string_format(const char* fmt, Args... args)
{
    size_t size = std::snprintf(nullptr, 0, fmt, args...);
    std::string buf;
    buf.reserve(size + 1);
    buf.resize(size);
    std::snprintf(&buf[0], size + 1, fmt, args...);
    return buf;
}
