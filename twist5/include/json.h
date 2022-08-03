#pragma once

#include <map>
#include <string>
#include <vector>
#include "deprecated.h"

enum { JSON_STRING_LITERAL, JSON_ARRAY, JSON_INT_LITERAL, JSON_DOUBLE, JSON_OBJECT, JSON_BOOLEAN };

class JsonException: public std::exception {
private:
	std::string msg;
public:
	JsonException(const std::string &msg) : msg(msg) {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}
};

class JsonNode {
private:
	int nodeType;

	union {
		int intLiteral;
		double doubleLiteral;
		bool boolLiteral;
	};

	std::string stringLiteral;
	std::map<std::string, JsonNode> objectValues;
	std::vector<JsonNode> listValues;

	void expectObjectKey(const std::string &key) const;
	void expectNodeType(int nodeType) const;
public:
	JsonNode () : nodeType(-1) {}

	JsonNode (int type) : nodeType(type) {}

	static JsonNode fromInt(int val) {
		JsonNode result(JSON_INT_LITERAL);
		result.intLiteral = val;
		return result;
	}

	static JsonNode fromBool(bool val) {
		JsonNode result(JSON_BOOLEAN);
		result.boolLiteral = val;
		return result;
	}

	static JsonNode fromString(const std::string &val) {
		JsonNode result(JSON_STRING_LITERAL);
		result.stringLiteral = val;
		return result;
	}

	static JsonNode fromDouble(double val) {
		JsonNode result(JSON_DOUBLE);
		result.doubleLiteral = val;
		return result;
	}

	double getDouble(const std::string &key) const;
	void setDouble(const std::string &key, double value);
	
	int getInt() const;
	int getInt(const std::string &key) const;
	void setInt(const std::string &key, int val);

	bool getBool(const std::string &key) const;
	void setBool(const std::string &key, bool value);

	std::string getString(const std::string &key) const;
	void setString(const std::string &key, const std::string &value);
	
	std::vector<JsonNode> getArray(const std::string &key);
	const std::vector<JsonNode> &getArray(const std::string &key) const;
	
	void arrayAdd(const JsonNode &item);
	const std::vector<JsonNode> &asArray() const;

	bool has(const std::string &key) const;
	JsonNode &operator[](const std::string &key);
	const JsonNode &operator[](const std::string &key) const;
	
	// render Json body recursively to string
	std::string toString();
	
	void toStream(std::ostream &os);
};

JsonNode jsonParseFile(const std::string &fileName);
JsonNode jsonParseString(const std::string &data);
