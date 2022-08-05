#include "levelGen.h"
#include <iostream>
#include "map2d.h"
#include <sstream>
#include <map>
#include "util.h"
#include <algorithm>    // std::random_shuffle
#include "strutil.h"
#include <math.h>
#include <set>
#include "level.h"

using namespace std;

struct DirInfo {
	int dx;
	int dy;
	char shortName;
	Dir reverse;
};

map<Dir, DirInfo> DIR = {
	{ N, { 0, -1, 'N', S } },
	{ E, { 1,  0, 'E', W } },
	{ S, { 0,  1, 'S', N } },
	{ W, { -1, 0, 'W', E } },
	{ TELEPORT, { 0, 0, 'T', TELEPORT }}
};

/** short cut to check if a key exists in a given map */
//TODO: move to utility class
template<typename T, typename U>
bool hasKey(const T &aMap, const U &aKey) {
	return aMap.find(aKey) != aMap.end();
}

// essential missing functionality
// see: https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
template<typename T>
T pop(std::vector<T> &aContainer) {
	T response = aContainer[aContainer.size()-1];
	aContainer.pop_back();
	return response;
}

class Edge {
public:
	Node *src;
	Node *dest;
	Dir dir;
	char letter; // for teleporter edges...

	string toString() {
		return string_format("Edge %c from (%i, %i) to (%i, %i)", 
			DIR[dir].shortName, 
			src->x, src->y,
			dest->x, dest->y);
	}
};

bool Node::hasLink(Dir dir) {
	return hasKey(links, dir);
}
bool Node::hasLock(Dir dir) {
	return hasKey(locks, dir);
}
	
std::string Node::toString() const {
	return string_format("Node (%i, %i)", x, y);
}

string toString(const Map2D<Cell> &map) {
	stringstream result;
	
	for (size_t y = 0; y < map.getDimMY(); ++y) {
		stringstream row[4];

		for (size_t x = 0; x < map.getDimMX(); ++x) {
			auto cell = map.get(x, y);
			if (cell.isEmpty()) {
				row[0] << "▒▒▒▒";
				row[1] << "▒▒▒▒";
				row[2] << "▒▒▒▒";
				row[3] << "▒▒▒▒";
			}
			else {
				auto &node = cell.nodes[0];
				
				char pstartChar = node.pStart == 0 ? ' ' : (node.pStart == 1 ? '1' : '2');
				const char *northDoor = (node.hasLink(N) ? (node.hasLock(N) ? "X" : " ") : "▒");
				const char *westDoor = (node.hasLink(W) ? (node.hasLock(W) ? "X" : " ") : "▒");
				

				row[0] << string_format("▒▒%s▒", northDoor);
				row[1] << string_format ("▒%02i ", node.doorArea);
				// row[1] << "▒   ";
				row[2] << string_format("%s %c%c", westDoor, pstartChar, node.hasBonus ? '*' : ' ');
				row[3] << string_format("▒%c%c%c", 
					node.hasBanana ? 'b' : ' ', 
					node.hasKeycard ? 'k' : ' ', 
					node.hasLink(TELEPORT) ? node.letter : ' '
				);
			}
		}
		result << row[0].str() << "▒\n" << row[1].str() << "▒\n" << row[2].str() << "▒\n" << row[3].str() << "▒\n";
	}

	for (size_t x = 0; x < map.getDimMX(); ++x) {
		result << "▒▒▒▒";
	}
	result << "▒\n";

	return result.str();
}

/**
 * Add nodes in a few places, up to a certain num
 */
void initNodes(Map2D<Cell> &lvl, int num) {

	int x = lvl.getDimMY() / 2;
	int y = lvl.getDimMX() / 2;

	// random walk
	vector<Dir> dirs = { N, E, S, W };
	
	for (int i = 0; i < num; ++i) {
		while (!lvl.get(x, y).isEmpty()) {
			Dir dir = choice(dirs);
			int nx = x + DIR[dir].dx;
			int ny = y + DIR[dir].dy;
			if (lvl.inBounds(nx, ny)) {
				x = nx;
				y = ny;
			}
		}
		lvl.get(x, y).addNode();
	}
}

// list all potential edges from a given cell
vector<Edge> listEdges(Map2D<Cell> &lvl, Node *node) {
	vector<Edge> result;
	int x = node->x;
	int y = node->y;
	vector<Dir> dirs = { N, E, S, W };
	for (auto dir : dirs) {
		int nx = x + DIR[dir].dx;
		int ny = y + DIR[dir].dy;
		if (lvl.inBounds(nx, ny)) {
			auto &other = lvl.get(nx, ny);
			if (other.isEmpty()) continue;
			Node &otherNode = other.nodes[0];
			
			Edge e = { node, &otherNode, dir, '1' };
			result.push_back(e);
		}
	}

	return result;
}


vector<Node*> getAllNodes(Map2D<Cell> &lvl) {
	vector<Node*> result;
	for (size_t y = 0; y < lvl.getDimMY(); ++y) {
		for (size_t x = 0; x < lvl.getDimMY(); ++x) {
			if (lvl.get(x, y).isEmpty()) continue;
			result.push_back(&lvl.get(x, y).nodes[0]);
		}
	}
	return result;
}

vector<Node*> getDoorAreaNodes(Map2D<Cell> &lvl, int area) {
	vector<Node*> result;
	for (size_t y = 0; y < lvl.getDimMY(); ++y) {
		for (size_t x = 0; x < lvl.getDimMY(); ++x) {
			if (lvl.get(x, y).isEmpty()) continue;
			Node *n = &lvl.get(x, y).nodes[0];
			if (n->doorArea == area) {
				result.push_back(n);
			}
		}
	}
	return result;
}

// distribute a given item in a round-robin fashion, so that each area gets some.
void roundRobinDistribute(const vector<Node*> &allNodes, int max, void set(Node*), bool get(Node*), const vector<int> &areas) {

	// distribute items in a round-robin fashion, starting in the smallest
	size_t pos = 0;
	size_t areaPos = 1;
	int remain = max;
	size_t it = 0;

	while (remain > 0) {
		int currentArea = areas[areaPos];
		
		Node *n = allNodes[pos];
		// scan for a node of the right area, without bananas
		if (n->doorArea == currentArea && !get(n)) {
			set(n);
			remain--;
			// round robin on the areas
			areaPos = (areaPos + 1) % areas.size();
			it = 0;
		}
		
		// round robin on the nodes
		pos = (pos + 1) % allNodes.size();
		it++;
		if (it > allNodes.size()) {
			// we've wrapped around without finding any place
			// go to the next area and try again
			areaPos = (areaPos + 1) % areas.size();
			it = 0;
		}
	}

}

void placeObjects(Map2D<Cell> &lvl, int maxBananas, int maxBonus) {

	multiset<int> doorAreaCounts;
	// multimap<int, Node*> doorAreaNodes;

	auto allNodes = getAllNodes(lvl);
	for (Node *n : allNodes) {
		doorAreaCounts.insert(n->doorArea);
		// doorAreaNodes.insert(std::pair<int, Node*>(n->doorArea, n));
	}

	std::set<int> doorAreaSet(doorAreaCounts.begin(), doorAreaCounts.end());
	std::vector<int> areasOrderedByFrq(doorAreaSet.begin(), doorAreaSet.end());
	std::sort(areasOrderedByFrq.begin(), areasOrderedByFrq.end(), 
		[&](const int &i1, const int &i2) {
			return doorAreaCounts.count(i1) > doorAreaCounts.count(i2);
		}
	);

#ifdef DEBUG
	for(auto i : areasOrderedByFrq) {
		std::cout << "Area #" << i << " count: " << doorAreaCounts.count(i) << endl;
	}
#endif

	// put players in the largest area
	auto largestAreaNodes = getDoorAreaNodes(lvl, areasOrderedByFrq[0]);
	random_shuffle(largestAreaNodes.begin(), largestAreaNodes.end());
	largestAreaNodes[0]->pStart = 1;
	largestAreaNodes[1]->pStart = 2;

	random_shuffle(allNodes.begin(), allNodes.end());
	
	// sort by degree. This means dead-ends are considered first
	std::sort(allNodes.begin(), allNodes.end(), 
		[&](const Node* a, const Node *b) {
			return a->degree() < b->degree();
		}
	);

	roundRobinDistribute(
		allNodes,
		maxBananas,
		[](Node* n) { n->hasBanana = true; },
		[](Node* n) { return n->hasBanana; },
		areasOrderedByFrq
	);

	roundRobinDistribute(
		allNodes,
		maxBonus,
		[](Node* n) { n->hasBonus = true; },
		[](Node* n) { return n->hasBanana || n->hasBonus; },
		areasOrderedByFrq
	);

	random_shuffle(allNodes.begin(), allNodes.end());
	// sort by degree. This means dead-ends are considered first
	std::sort(allNodes.begin(), allNodes.end(), 
		[&](const Node* a, const Node *b) {
			return a->degree() < b->degree();
		}
	);
	
	int maxKeys = areasOrderedByFrq.size(); // put one key in each area
	roundRobinDistribute(
		allNodes,
		maxKeys,
		[](Node* n) { n->hasKeycard = true; },
		[](Node* n) { return n->hasKeycard; },
		areasOrderedByFrq
	);

}

void linkNodes(Node *src, Node *dest, Dir dir, bool reverse = true, bool doorLink = false) {
	assert(!hasKey(src->links, dir));
	src->links[dir] = dest;

	if (doorLink) {
		src->locks[dir] = true;
	}

	if (reverse) {
		linkNodes(dest, src, DIR[dir].reverse, false, doorLink);
	}
}

void convertArea(Map2D<Cell> &lvl, Node *src, Node *dest, bool convertDoors=true) {
	int srcArea = src->area;
	int srcDoorArea = src->doorArea;

	for (size_t y = 0; y < lvl.getDimMY(); ++y) {
		for (size_t x = 0; x < lvl.getDimMY(); ++x) {
			if (lvl.get(x, y).isEmpty()) continue;
			auto &node = lvl.get(x, y).nodes[0];
			if (node.area == srcArea) {
				node.area = dest->area;
				// if (convertDoors) {
				// 	node.doorArea = dest->doorArea;
				// }
			}
			if (convertDoors && node.doorArea == srcDoorArea) {
				node.doorArea = dest->doorArea;
			}
		}
	}
};

void kruskal(Map2D<Cell> &lvl, int maxDoors) {
	vector<Edge> allEdges;
	
	int doorRemain = maxDoors;
	int areaCounter = 0;

	// list all possible edges
	for (auto n : getAllNodes(lvl)) {
		n->area = areaCounter++;
		n->doorArea = n->area;
		for (auto &edge : listEdges(lvl, n)) {
			allEdges.push_back(edge);
		}
	}

	vector<Node*> teleporterNodes = getAllNodes(lvl);
	random_shuffle(teleporterNodes.begin(), teleporterNodes.end());
	char letter = 'A';
	while (teleporterNodes.size() >= 2) {
		Node *from = pop(teleporterNodes);
		Node *to = pop(teleporterNodes);
		allEdges.push_back(Edge{ from, to, TELEPORT, letter++});
	}

	// Kruskal's normally picks the lowest weights in a priority queue,
	// but for us the order really doesn't matter!
	random_shuffle(allEdges.begin(), allEdges.end());

	int countAreas = areaCounter;
	while (allEdges.size() > 0 && areaCounter > 1) {
		// pick a random edge
		// does it link two areas?
		Edge e = pop(allEdges);

		// if (countAreas == 2 && e.dir == TELEPORT) {
		// 	continue;
		// 	//TODO! there is a risk that there is nothing left...
		// }

		if (e.src->area == e.dest->area) {
			// this is now a candidate for creating a loop
		}
		else {
			cout << "Adding link: " << e.toString() << endl;
			// create an edge, update areas
			// until there is only one area
			
			bool lockDoor = (doorRemain > 0 && e.dir != TELEPORT && random(100) > 50);
			if (lockDoor) {
				linkNodes(e.src, e.dest, e.dir, true, true);
				convertArea(lvl, e.dest, e.src, false);
				// placeObjects(lvl, e.src->doorArea, e.dest->doorArea);
				doorRemain--;
			}
			else {
				linkNodes(e.src, e.dest, e.dir);
				convertArea(lvl, e.dest, e.src);
			}
			if (e.dir == TELEPORT) {
				e.src->letter = e.letter;
				e.dest->letter = e.letter;
			}
			areaCounter--;

			// print intermediate step for debugging
			// cout << endl << toString(lvl) << endl;
			// cout << endl << endl;
			// char readline;
			// cin >> readline;
		}
	}
}

void initCells(Map2D<Cell> &lvl) {
	for (size_t y = 0; y < lvl.getDimMY(); ++y) {
		for (size_t x = 0; x < lvl.getDimMY(); ++x) {
			auto &cell = lvl.get(x, y);
			cell.x = x;
			cell.y = y;
		}
	}
}

Map2D<Cell> createKruskalMaze(int roomNum) {
	
	int mapSize = sqrt(roomNum * 3);
	int maxDoors = max(1, roomNum / 5);
	int bananas = max(2, roomNum / 3);
	int bonus = max(1, roomNum / 8);

	auto level = Map2D<Cell>(mapSize, mapSize);
	initCells(level);
	
	// initialize a bunch of adjacent nodes
	initNodes(level, roomNum);

	kruskal(level, maxDoors);

	placeObjects(level, bananas, bonus);

#ifdef DEBUG
	cout << endl << toString(level) << endl;
#endif

	return level;
}