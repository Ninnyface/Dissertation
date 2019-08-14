#pragma once
#include "NavigationMap.h"
#include <string>
#include <vector>
namespace NCL {
	namespace CSC8503 {
		struct GridNode {
			GridNode* parent;

			GridNode* connected[4];
			int		  costs[4];

			Vector3		position;

			float f;
			float g;

			int type;

			GridNode() {
				for (int i = 0; i < 4; ++i) {
					connected[i] = nullptr;
					costs[i] = 0;
				}
				f = 0;
				g = 0;
				type = 0;
				parent = nullptr;
			}
			~GridNode() {	}
		};

		class NavigationGrid : public NavigationMap	{
		public:
			NavigationGrid();
			NavigationGrid(const std::string&filename);
			~NavigationGrid();

			std::vector<Vector3> GetWalls() {
				return walls;
			}
			std::vector<Vector3> GetFloors() {
				return floors;
			}
			std::vector<Vector3> GetWater() {
				return water;
			}
			std::vector<GridNode*> GetOpenList() {
				return savedOpenList;
			}
			std::vector<GridNode*> GetClosedList() {
				return savedClosedList;
			}

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
			bool FindCBSPath(const Vector3& from, const Vector3& to, std::vector<NavigationPath*> existingPaths, NavigationPath& outPath) override;
				
		protected:
			std::vector<GridNode*>	FindBestNodes(GridNode* node);
			bool		NodeInList(GridNode* n, std::vector<GridNode*>& list) const;
			GridNode*	RemoveBestNode(std::vector<GridNode*>& list) const;
			float		Heuristic(GridNode* hNode, GridNode* endNode) const;
			void		ResolveConflict(NavigationPath& a, NavigationPath& b, int index, int count);
			int nodeSize;
			int gridWidth;
			int gridHeight;
			
			std::vector < GridNode * > savedOpenList;
			std::vector < GridNode * > savedClosedList;
			std::vector<std::vector<GridNode*>> paths;
			std::vector<Vector3> walls;
			std::vector<Vector3> floors;
			std::vector<Vector3> water;

			GridNode* allNodes;
		};
	}
}