#pragma once
#include "NavigationGrid.h"
#include "GameTechRenderer.h"
#include "../Common/GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class Demo {
		public:
			Demo();
			~Demo();

			void UpdateDemo(float dt);
			void nextStep();
			void previousStep();
			void toggleWorking();
			void swapMulti();
			void resetWorld();

		protected:
			void InitialiseAssets();
			void InitCamera();

			GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour);
			GameObject* AddAgentToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour);
			GameObject* AddWaterToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour);
			GameObject* AddFloorToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour);
			GameObject* AddClosedListToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour);

			void InitWorld();

			GameWorld* world;
			GameTechRenderer* renderer;

			OGLMesh* cubeMesh = nullptr;
			OGLTexture* basicTex = nullptr;
			OGLShader*	basicShader = nullptr;

			
			vector<GameObject*> closedList;
			vector<Vector3> floors;
			vector<Vector3> water;
			vector<Vector3> pathNodes;
			vector<Vector3> pathNodes2;
			vector<GameObject*> agentObjects;
			vector<GameObject*> pathObjects;
			vector<GameObject*> pathObjects2;
			vector<GameObject*> cpObjects;
			vector<NavigationPath*> paths;

			bool isMulti;
			bool isToggled;
			int fuelCost;
			int step;
			int currentCost;
			int startX;
			int endX;
			int startZ;
			int endZ;
		};
	}
}