#include "NavigationGrid.h"
#include "../Common/Assets.h"
#include <fstream>

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE		= 0;
const int RIGHT_NODE	= 1;
const int TOP_NODE		= 2;
const int BOTTOM_NODE	= 3;

const char WALL_NODE	= 'x';
const char FLOOR_NODE	= '.';
const char WATER_NODE	= '!';

NavigationGrid::NavigationGrid()	{
	nodeSize	= 0;
	gridWidth	= 0;
	gridHeight	= 0;
	allNodes	= nullptr;
}

NavigationGrid::NavigationGrid(const std::string&filename) : NavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new GridNode[gridWidth * gridHeight];

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3(x * gridWidth, 0, y * gridHeight);
		}
	}
	
	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];		
			if (n.type == 'x') {
				walls.push_back(Vector3(x*nodeSize, 0, y*nodeSize));
			}
			if (n.type == '.') {
				floors.push_back(Vector3(x*nodeSize, 0, y*nodeSize));
			}
			if (n.type == '!') {
				water.push_back(Vector3(x*nodeSize, 0, y*nodeSize));
			}
			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}
			for (int i = 0; i < 4; ++i) {
				if (n.connected[i]) {
					if (n.connected[i]->type == '.') {
						n.costs[i]		= 10;
					}
					if (n.connected[i]->type == '!') {
						n.costs[i]		= 100;
					}
					if (n.connected[i]->type == 'x') {
						n.connected[i]  = nullptr; //actually a wall, disconnect!
					}
				}
			}
		}	
	}
}

NavigationGrid::~NavigationGrid()	{
	delete[] allNodes;
}

bool NavigationGrid::FindCBSPath(const Vector3& from, const Vector3& to, std::vector<NavigationPath*> existingPaths, NavigationPath& outPath) {
	int fromX = (from.x / nodeSize);
	int fromZ = (from.z / nodeSize);

	int toX = (to.x / nodeSize);
	int toZ = (to.z / nodeSize);

	if (fromX < 0 || fromX > gridWidth - 1 ||
		fromZ < 0 || fromZ > gridHeight - 1) {
		return false; // outside of map region !	
	}

	if (toX < 0 || toX > gridWidth - 1 ||
		toZ < 0 || toZ > gridHeight - 1) {
		return false; // outside of map region !
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode = &allNodes[(toZ * gridWidth) + toX];

	std::vector < GridNode * > openList;
	std::vector < GridNode * > closedList;

	openList.emplace_back(startNode);
	startNode->f = 0;
	startNode->g = 0;
	startNode->parent = nullptr;
	
	GridNode * currentBestNode = nullptr;

	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);

		if (currentBestNode == endNode) {// we ’ve found the path !
			savedClosedList = closedList;
			savedOpenList = openList;
			GridNode * node = endNode;
			std::vector<GridNode*> nodes;
			while (node != nullptr) {
				outPath.PushWaypoint(node->position);		
				nodes.emplace(nodes.begin(), node);
				node = node->parent; // Build up the waypoints	
			}
			paths.push_back(nodes);
			for (int i = 0; i < existingPaths.size(); i++) {
				for (int j = 0; j < existingPaths.at(i)->GetWaypoints().size(); j++) {			
					if (j <= outPath.GetWaypoints().size()) {
						if (existingPaths.at(i)->GetWaypoints().at(j) == outPath.GetWaypoints().at(j)) {
							
							ResolveConflict(outPath, *existingPaths.at(i), i, j);
							break;
						}
						
					}	
				}
			}

			return true;

		}
		else {
			for (int i = 0; i < 4; ++i) {
				GridNode * neighbour = currentBestNode->connected[i];
				if (!neighbour) { // might not be connected ...
					continue;

				}
				bool inClosed = NodeInList(neighbour, closedList);
				if (inClosed) {
					continue; // already discarded this neighbour ...

				}

				float h = Heuristic(neighbour, endNode);
				float g = currentBestNode->g + currentBestNode->costs[i];
				float f = h + g;

				bool inOpen = NodeInList(neighbour, openList);

				if (!inOpen) { // first time we ’ve seen this neighbour
					openList.emplace_back(neighbour);

				}
				// might be a better route to this node !
				if (!inOpen || f < neighbour->f) {
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}		
			closedList.emplace_back(currentBestNode);
		}
	}
	return false;
}

void NavigationGrid::ResolveConflict(NavigationPath& pathB, NavigationPath& pathA, int index, int count) {
	std::vector<GridNode*> a = paths.at(index);
	std::vector<GridNode*> b = paths.at(paths.size() - 1);

	GridNode* aNode = a.at(0);
	GridNode* bNode = b.at(count);

	GridNode* newANode = aNode;
	GridNode* newBNode = bNode;

	for (int i = 0; i < b.size(); i++) {
		if (bNode == a.at(i)) {
			aNode = a.at(i - 1);
		}
	}
	if (count != 0) {
		bNode = b.at(count - 1);
	}
	std::vector<GridNode*> newANodes = FindBestNodes(aNode);
	std::vector<GridNode*> newBNodes = FindBestNodes(bNode);
	int bestIndex = 0;
	int aCost = 0;
	int bCost = 0;

	for (int i = 0; i < newANodes.size(); i++) {
		for (int j = 0; j < newBNodes.size(); j++) {
			aCost = 0;
			bCost = 0;
			if (newANodes.at(i)->position != newBNodes.at(j)->position && !(i == newANodes.size() - 1 && j == newBNodes.size() - 1)) {
				for (int k = 0; k < count; k++) {
					if (newANodes.at(i)->position == a.at(k)->position) {
						aCost += 2;
					}
				}	
				if (newANodes.at(i)->type == '.') {
					aCost += 1;
				}
				if (newANodes.at(i)->type == '!') {
					aCost += 10;
				}
				for (int k = 0; k < count; k++) {
					if (newBNodes.at(j)->position == b.at(k)->position) {
						bCost += 2;
					}
				}
				if (newBNodes.at(j)->type == '.') {
					bCost += 1;
				}
				if (newBNodes.at(j)->type == '!') {
					bCost += 10;
				}
				if (bestIndex == 0 || aCost + bCost < bestIndex) {
					bestIndex = aCost + bCost;
					newANode = newANodes.at(i);
					newBNode = newBNodes.at(j);
				}
			}
		}
	}
	if (bestIndex > 2) {
		ResolveConflict(pathB, pathA, index, count + 1);
	}
	else {
		if (newANode->position == a.at(count)->position) {
			pathB.GetWaypoints().insert(pathB.GetWaypoints().begin() + count + 1, newBNode->position);
			int a = 0;
			bool foundA = false;
			bool foundB = false;
			for (int i = 0; i < pathA.GetWaypoints().size(); i++) {
				if (pathA.GetWaypoints().at(i) == bNode->position) {
					foundB = true;
				}
				if (pathA.GetWaypoints().at(i) == aNode->position) {
					foundA = true;
				}
				if (foundA && foundB) {
					break;
				}
				if (foundA || foundB) {
					a++;
				}

			}
			for (int i = 0; i < a - 1; i++) {
				pathB.GetWaypoints().insert(pathB.GetWaypoints().begin() + count + 1, pathB.GetWaypoints().at(count + 1));
			}
			pathB.GetWaypoints().insert(pathB.GetWaypoints().begin() + count + 1, pathB.GetWaypoints().at(count + 3));

		}
		else {
			pathA.GetWaypoints().insert(pathA.GetWaypoints().begin() + count + 1, newANode->position);
			int a = 0;
			bool foundA = false;
			bool foundB = false;
			for (int i = 0; i < pathB.GetWaypoints().size(); i++) {
				if (pathB.GetWaypoints().at(i) == bNode->position) {
					foundB = true;
				}
				if (pathB.GetWaypoints().at(i) == aNode->position) {
					foundA = true;
				}
				if (foundA || foundB) {
					a++;
				}
				if (foundA && foundB) {
					break;
				}
				
			}

			for (int i = 0; i <= a+1; i++) {
				pathA.GetWaypoints().insert(pathA.GetWaypoints().begin() + count + 1, pathA.GetWaypoints().at(count + 1));
			}
			pathA.GetWaypoints().insert(pathA.GetWaypoints().begin() + count + 1, pathA.GetWaypoints().at(count + 3 + (a+1)));
		}
	}
}

std::vector<GridNode*> NavigationGrid::FindBestNodes(GridNode* node){
	std::vector<GridNode*> possibilities;

	for (int i = 0; i <= 4; i++) {
		if (i == 4) {
			possibilities.push_back(node);
		}
		else if (node->connected[i] != nullptr) {
			possibilities.push_back(node->connected[i]);
		}
	}
	return possibilities;
}


bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	int fromX = (from.x / nodeSize);
	int fromZ = (from.z / nodeSize);
	
	int toX = (to.x / nodeSize);
	int toZ = (to.z / nodeSize);
	
	if (fromX < 0 || fromX > gridWidth - 1 ||
		fromZ < 0 || fromZ > gridHeight - 1) {
		return false; // outside of map region !	
	}
	
	if (toX < 0 || toX > gridWidth - 1 ||
		toZ < 0 || toZ > gridHeight - 1) {
		return false; // outside of map region !
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode = &allNodes[(toZ * gridWidth) + toX];
	
	std::vector < GridNode * > openList;
	std::vector < GridNode * > closedList;

	openList.emplace_back(startNode);
	startNode -> f = 0;
	startNode -> g = 0;
	startNode -> parent = nullptr;
	
	GridNode * currentBestNode = nullptr;
	
	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);
		
		if (currentBestNode == endNode) {// we ’ve found the path !
			savedClosedList = closedList;
			savedOpenList = openList;
			GridNode * node = endNode;
			while (node != nullptr) {
				outPath.PushWaypoint(node -> position);
				node = node -> parent; // Build up the waypoints
				
			}
			return true;
			
		}
		else {
			for (int i = 0; i < 4; ++i) {
				GridNode * neighbour = currentBestNode -> connected[i];
				if (!neighbour) { // might not be connected ...
					continue;
					
				}
				bool inClosed = NodeInList(neighbour, closedList);
				if (inClosed) {
					continue; // already discarded this neighbour ...
					
				}
				
				float h = Heuristic(neighbour, endNode);
				float g = currentBestNode -> g + currentBestNode -> costs[i];
				float f = h /*+ g*/;
				
				bool inOpen = NodeInList(neighbour, openList);
				
				if (!inOpen) { // first time we ’ve seen this neighbour
					openList.emplace_back(neighbour);
					
				}
				// might be a better route to this node !
				if (!inOpen || f < neighbour -> f) {
					neighbour -> parent = currentBestNode;
					neighbour -> f = f;
					neighbour -> g = g;
				}
	
			}
			closedList.emplace_back(currentBestNode);
			
		}
	
	}
	return false;
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
	std::vector < GridNode * >::iterator i =
		std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

GridNode*  NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator bestI = list.begin();
	
	GridNode * bestNode = *list.begin();
	
	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i) -> f < bestNode -> f) {
			bestNode = (*i);
			bestI = i;	
		}	
	}
	list.erase(bestI);
	return bestNode;
}

float NavigationGrid::Heuristic(GridNode* n1, GridNode* n2) const {
	return (n1 -> position - n2 -> position).Length();
}