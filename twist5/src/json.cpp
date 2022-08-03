#include "json.h"
#include <allegro5/allegro.h>
#include "util.h"
#include <sstream>
#include <algorithm>
#include "strutil.h"

using namespace std;

const char *NODE_TYPE_TO_STR[] = {
	"STRING_LITERAL", "ARRAY", "INT_LITERAL", "DOUBLE", "OBJECT", "BOOLEAN"
};

void JsonNode::expectNodeType(int _nodeType) const {
	if (nodeType != _nodeType) {
		throw JsonException(string_format("Expected node type %s but found %s", NODE_TYPE_TO_STR[_nodeType], NODE_TYPE_TO_STR[nodeType]));
	}
}

void JsonNode::expectObjectKey(const std::string &key) const {
	expectNodeType(JSON_OBJECT);
	if (objectValues.find(key) == objectValues.end()) {
		throw JsonException(string_format("Missing key '%s'", key.c_str()));
	}
}

vector<JsonNode> JsonNode::getArray(const string &key) {
	expectObjectKey(key);
	JsonNode child = objectValues[key];
	child.expectNodeType(JSON_ARRAY);
	return child.listValues;
}

const vector<JsonNode> &JsonNode::getArray(const string &key) const {
	expectObjectKey(key);
	const JsonNode &child = objectValues.at(key);
	child.expectNodeType(JSON_ARRAY);
	return child.listValues;
}

const vector<JsonNode> &JsonNode::asArray() const {
	expectNodeType(JSON_ARRAY);
	return listValues;
}

void JsonNode::arrayAdd(const JsonNode &item) {
	expectNodeType(JSON_ARRAY);
	listValues.push_back(item);
}

bool JsonNode::has(const std::string &key) const {
	expectNodeType(JSON_OBJECT);
	return (objectValues.find(key) != objectValues.end());
}

JsonNode &JsonNode::operator[](const string &key) {
	expectNodeType(JSON_OBJECT);
	//TODO: autovivify or not?
	return objectValues[key];
}

const JsonNode &JsonNode::operator[](const string &key) const {
	expectObjectKey(key);
	return objectValues.at(key);
}

void JsonNode::toStream(ostream &ss) {
	switch (nodeType) {
		case JSON_ARRAY: {
				ss << "[ ";
				bool first = true;
				for (auto i : listValues) {
					if(first) { first = false; } else { ss << ", "; }
					i.toStream(ss);
				}
				ss << " ]";
			}
			break;
		case JSON_OBJECT: {
				ss << "{ ";
				std::vector<std::string> keys;
				for (const auto &pair : objectValues) {
					keys.push_back(pair.first);
				}
				sort(keys.begin(), keys.end());
				bool first = true;
				for (const auto &key : keys) {
					if(first) { first = false; } else { ss << ", "; }
					ss << "\"" << key << "\": "; 
					objectValues[key].toStream(ss);
				}
				ss << " }";
			}
			break;
		case JSON_BOOLEAN:
				ss << (boolLiteral ? "true" : "false");
			break;
		case JSON_DOUBLE:
				ss << doubleLiteral;
			break;
		case JSON_INT_LITERAL:
				ss << intLiteral;
			break;
		case JSON_STRING_LITERAL:
				ss << "\"" << stringLiteral << "\"";
			break;
	}
}

std::string JsonNode::toString() {
	stringstream ss;
	toStream(ss);
	return ss.str();
}


bool JsonNode::getBool(const string &key) const {
	expectObjectKey(key);
	const JsonNode &child = objectValues.at(key);
	child.expectNodeType(JSON_BOOLEAN);
	return child.boolLiteral;
}

void JsonNode::setBool(const string &key, bool value) {
	expectNodeType(JSON_OBJECT);
	objectValues[key] = JsonNode::fromBool(value);
}

string JsonNode::getString(const string &key) const {
	expectObjectKey(key);
	const JsonNode &child = objectValues.at(key);
	child.expectNodeType(JSON_STRING_LITERAL);
	return child.stringLiteral;
}

void JsonNode::setString(const std::string &key, const std::string &value) {
	expectNodeType(JSON_OBJECT);
	objectValues[key] = JsonNode::fromString(value);
}

double JsonNode::getDouble(const std::string &key) const {
	expectObjectKey(key);
	const JsonNode &child = objectValues.at(key);
	child.expectNodeType(JSON_DOUBLE);
	return child.doubleLiteral;
}

void JsonNode::setDouble(const std::string &key, double value) {
	expectNodeType(JSON_OBJECT);
	objectValues[key] = JsonNode::fromDouble(value);
}

int JsonNode::getInt() const {
	expectNodeType(JSON_INT_LITERAL);
	return intLiteral;
}

int JsonNode::getInt(const string &key) const {
	expectObjectKey(key);
	const JsonNode &child = objectValues.at(key);
	child.expectNodeType(JSON_INT_LITERAL);
	return child.intLiteral;
}

void JsonNode::setInt(const std::string &key, int value) {
	expectNodeType(JSON_OBJECT);
	objectValues[key] = JsonNode::fromInt(value);
}

enum class TokenType { /** value that can not actually be produced by tokenizer */ DUMMY ,
		BRACE_OPEN, BRACE_CLOSE, COMMA, COLON, BRACKET_OPEN, BRACKET_CLOSE, QUOTED_STRING, KEYWORD, NUMBER, END };

const char *TOKEN_TYPE_TO_STR[(size_t)TokenType::END] = {
	"DUMMY", "BRACE_OPEN", "BRACE_CLOSE", "COMMA", "COLON", "BRACKET_OPEN", "BRACKET_CLOSE", "QUOTED_STRING",
	"KEYWORD", "NUMBER"
};

class Token {
public:
	TokenType type;
	string value;

	Token() : type(TokenType::DUMMY), value() {}
	Token(TokenType type) : type(type), value() {}
};

class JsonTokenizer {
public:
	JsonTokenizer(const string &data) : data(data) {
		next = nextToken();
	}
private:
	string data;
	size_t pos = 0;
	int lineno = 1;
	int col = 1;

	Token next;

	void advancePos() {
		if (data[pos] == '\n') {
			lineno++;
			col = 0;
		}
		col++;
		pos++;
	}

	void skipWhitespace() {
		while (true) {
			if (pos >= data.size()) break;

			int c = data[pos];
			if (c == '\r' || c == '\n' || c == '\t' || c == ' ') {
				advancePos();
			}
			else {
				break;
			}
		}
	}

	Token readSingleToken() {
		Token result;
		switch (data[pos]) {
			case ':':
				result = Token(TokenType::COLON); break;
			case ',':
				result = Token(TokenType::COMMA); break;
			case '[':
				result = Token(TokenType::BRACKET_OPEN); break;
			case ']':
				result = Token(TokenType::BRACKET_CLOSE); break;
			case '{':
				result = Token(TokenType::BRACE_OPEN); break;
			case '}':
				result = Token(TokenType::BRACE_CLOSE); break;
			default:
				throw JsonException(string_format("Unexpected token: '%c'", data[pos]));
		}
		advancePos();
		return result;
	}

	Token readQuotedString() {
		int quote = data[pos];
		advancePos();
		size_t start = pos;
		Token result = Token(TokenType::QUOTED_STRING);

		while (true) {
			if (pos >= data.size()) {
				throw JsonException("Unterminated quoted string");
			}

			int c = data[pos];
			if (c == '\r' || c == '\n') {
				throw JsonException("Newline inside quoted string not allowed");
			}
			else if (c == quote) {
				result.value = data.substr(start, pos-start);
				advancePos();
				break;
			}
			else {
				advancePos();
			}
		}
		return result;
	}

	Token readNumber() {
		int start = pos;
		Token result = Token(TokenType::NUMBER);

		while (true) {

			if (pos >= data.size()) { break; }

			int c = data[pos];
			if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == 'e' || c == 'E' || c == '.') {
				advancePos();
			}
			else {
				break;
			}
		}
		result.value = data.substr(start, pos - start);
		return result;

	}

	Token readKeyword() {
		int start = pos;
		Token result = Token(TokenType::KEYWORD);

		while (true) {

			if (pos >= data.size()) { break; }

			int c = data[pos];
			if (c >= 'a' && c <= 'z') {
				advancePos();
			}
			else {
				break;
			}
		}
		result.value = data.substr(start, pos - start);
		return result;
	}

	Token nextToken() {
		skipWhitespace();

		if (pos >= data.size()) {
			return Token(TokenType::END);
		}

		int c = data[pos];
		switch (c) {

		case ':': case ',': case '[': case ']': case '{': case '}':
			return readSingleToken(); break;
		case '"': case '\'':
			return readQuotedString(); break;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case '-': case '.':
			return readNumber(); break;
		case 't':
		case 'f':
		case 'n':
			return readKeyword(); break;
		default:
			throw JsonException(string_format("Unexpected character: %c", data[pos]));
		}
	}
public:
	TokenType peek() {
		return next.type;
	}

	Token get() {
		Token result = next;
		next = nextToken();
		return result;
	}
};

class JsonParser {
private:
	JsonTokenizer &tokenizer;

	JsonNode parseObjectNode() {
		consume(TokenType::BRACE_OPEN);

		JsonNode parent = JsonNode(JSON_OBJECT);

		while(true) {
			Token key = tokenizer.get();
			if (key.type != TokenType::QUOTED_STRING) {
				throw JsonException(
					string_format("Expected quoted string but got %s", TOKEN_TYPE_TO_STR[(size_t)key.type])
				);
			}

			consume(TokenType::COLON);

			parent[key.value] = parseNode();

			expect(TokenType::COMMA, TokenType::BRACE_CLOSE);

			TokenType type = tokenizer.peek();
			if (type == TokenType::BRACE_CLOSE) {
				break;
			}

			consume(TokenType::COMMA);
		}

		consume(TokenType::BRACE_CLOSE);
		return parent;
	}

	JsonNode parseArrayNode() {
		consume(TokenType::BRACKET_OPEN);

		JsonNode parent = JsonNode(JSON_ARRAY);

		while (true) {
			JsonNode n = parseNode();
			parent.arrayAdd(n);

			expect(TokenType::COMMA, TokenType::BRACKET_CLOSE);
			TokenType type = tokenizer.peek();
			if (type == TokenType::BRACKET_CLOSE) {
				break;
			}

			consume(TokenType::COMMA);
		}
		consume(TokenType::BRACKET_CLOSE);
		return parent;
	}

	JsonNode parseStringNode() {
		Token t = tokenizer.get();
		return JsonNode::fromString(t.value);
	}

	JsonNode parseNumberNode() {
		Token t = tokenizer.get();
		if(t.value.find('.') != string::npos) {
			return JsonNode::fromDouble(stod(t.value));
		}
		else {
			return JsonNode::fromInt(stoi(t.value));
		}
	}

	JsonNode parseBooleanNode() {
		Token t = tokenizer.get();
		if (t.value == "true") {
			return JsonNode::fromBool(true);
		}
		else if (t.value == "false") {
			return JsonNode::fromBool(false);
		}
		else {
			throw JsonException("Unknown keyword when creating a boolean node");
		}
	}

	JsonNode parseNode() {

		TokenType type = tokenizer.peek();
		if (type == TokenType::BRACE_OPEN) {
			return parseObjectNode();
		}
		else if (type == TokenType::BRACKET_OPEN) {
			return parseArrayNode();
		}
		else if (type == TokenType::QUOTED_STRING) {
			return parseStringNode();
		}
		else if (type == TokenType::NUMBER) {
			return parseNumberNode();
		}
		else if (type == TokenType::KEYWORD) {
			//TODO: could be value 'true' or 'false', but also 'null', 'undefined' etc?
			return parseBooleanNode();
		}
		throw JsonException ("Unexpected token type");
	}

public:
	JsonParser(JsonTokenizer &tokenizer) : tokenizer(tokenizer) {

	}

	void consume(TokenType type) {
		Token t = tokenizer.get();
		if (t.type != type) {
			throw JsonException(
				string_format("Unexpected token, expected: %s but got: %s", 
					TOKEN_TYPE_TO_STR[(size_t)type], 
					TOKEN_TYPE_TO_STR[(size_t)t.type])
			);
		}
	}

	void expect(TokenType opt1, TokenType opt2) {
		TokenType type = tokenizer.peek();
		if (!(type == opt1 || type == opt2)) {
			throw JsonException(
				string_format("Unexpected token, expected: %s or %s but got: %s", 
					TOKEN_TYPE_TO_STR[(size_t)opt1], 
					TOKEN_TYPE_TO_STR[(size_t)opt2], 
					TOKEN_TYPE_TO_STR[(size_t)type])
			);
		}
	}

	JsonNode parse() {
		JsonNode result = parseNode();
		consume(TokenType::END);
		return result;
	}

};

JsonNode jsonParseString(const std::string &data) {
	JsonTokenizer tokenizer = JsonTokenizer(data);
	JsonParser parser = JsonParser(tokenizer);
	return parser.parse();
}

JsonNode jsonParseFile(const std::string &fileName) {
	string result("");

	char buffer[4096];
	ALLEGRO_FILE *fp = al_fopen(fileName.c_str(), "r");
	while(al_fgets(fp, buffer, 4096))
	{
		result += buffer;
	}
	al_fclose(fp);

	return jsonParseString(result);
}
