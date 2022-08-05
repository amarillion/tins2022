#ifndef _LEVEL_GEN_H_
#define _LEVEL_GEN_H_

#include <map2d.h>
#include <vector>
#include <map>
#include <string>

enum Dir { N, E, S, W, TELEPORT };

struct Node {
	int area;
	int doorArea;
	int x, y;
	std::map<Dir, Node*> links;
	std::map<Dir, bool> locks;

	char letter; // for teleporter edges...
	bool hasLink(Dir dir);
	bool hasLock(Dir dir);

	int pStart = 0;
	bool hasBanana = false;
	bool hasBonus = false;
	bool hasKeycard = false;

	int degree() const {
		return links.size();
	}

	std::string toString() const;

};

class Cell {
public:
	int x;
	int y;
	std::vector<Node> nodes;
	int roomType;
	int objects;

	bool isEmpty() const {
		return nodes.empty();
	}

	void addNode() {
		Node n;
		n.x = x;
		n.y = y;
		nodes.push_back(n);
	}
};

Map2D<Cell> createKruskalMaze(int levelNum);
std::vector<Node*> getAllNodes(Map2D<Cell> &lvl);

#endif