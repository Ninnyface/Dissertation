#include "Demo.h"
#include "../Common/GameWorld.h"
#include "../Plugins/OpenGLRendering/OGLMesh.h"
#include "../Plugins/OpenGLRendering/OGLShader.h"
#include "../Plugins/OpenGLRendering/OGLTexture.h"
#include "../Common/TextureLoader.h"
#include "../Common/GameObject.h"
#include "../Common/Camera.h"
#include <algorithm>

using namespace NCL;
using namespace CSC8503;

Demo::Demo() {
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	
	step	= 0;
	currentCost = 0;
	fuelCost = 0;
	isToggled = false;
	isMulti = false;
	InitialiseAssets();
}

void Demo::InitialiseAssets() {
	cubeMesh = new OGLMesh("cube.msh");
	cubeMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	cubeMesh->UploadToGPU();

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
	
	InitWorld();
	InitCamera();
}

Demo::~Demo() {
	delete cubeMesh;
	delete basicTex;
	delete basicShader;
	delete renderer;
	delete world;
}

void Demo::UpdateDemo(float dt) {	
	if (!isMulti) {
		if (isToggled) {
			renderer->DrawString("Number of nodes checked: " + std::to_string(step), Vector2(0, 0), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		else {
			renderer->DrawString("Current cost of path: " + std::to_string(currentCost), Vector2(0, 0), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}
	else {
		renderer->DrawString("Makespan: " + std::to_string(step), Vector2(0, 20), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		renderer->DrawString("Fuel: " + std::to_string(fuelCost), Vector2(0, 40), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		renderer->DrawString("Sum-of-costs: " + std::to_string(currentCost), Vector2(0, 0), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	}
	world->UpdateWorld(dt);
	world->GetMainCamera()->UpdateCamera(dt);
	renderer->Update(dt);
	renderer->Render();	
}

void Demo::InitCamera() {
	world->GetMainCamera()->SetNearPlane(3.0f);
	world->GetMainCamera()->SetFarPlane(4200.0f);
	world->GetMainCamera()->SetPitch(-90.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	world->GetMainCamera()->SetPosition(Vector3(45, 80, 45));
}

void Demo::swapMulti() {
	isMulti = !isMulti;
	closedList.clear();
	floors.clear();
	water.clear();
	pathNodes.clear();
	pathNodes2.clear();
	pathObjects.clear();
	pathObjects2.clear();
	cpObjects.clear();
	paths.clear();
	agentObjects.clear();
	step = 0;
	currentCost = 0;
	fuelCost = 0;
	isToggled = false;
	InitWorld();
}

void Demo::resetWorld() {
	closedList.clear();
	floors.clear();
	water.clear();
	pathNodes.clear();
	pathNodes2.clear();
	pathObjects.clear();
	pathObjects2.clear();
	cpObjects.clear();
	paths.clear();
	agentObjects.clear();
	step = 0;
	currentCost = 0;
	fuelCost = 0;
	isToggled = false;
	InitWorld();
}

void Demo::InitWorld() {
	world->ClearAndErase();
	if (isMulti) {
		Vector3 startPos(10, 0, 10);
		Vector3 endPos(80, 0, 80);
		Vector3 startPos2(80, 0, 80);
		Vector3 endPos2(10, 0, 10);
		NavigationGrid grid("MultiGrid.txt");



		NavigationPath* outPath = new NavigationPath();
		NavigationPath* outPath2 = new NavigationPath();

		float time = Window::GetTimer()->GetTimeDelta();
		bool found = grid.FindCBSPath(startPos, endPos, paths, *outPath);
		paths.push_back(outPath);
		found = grid.FindCBSPath(startPos2, endPos2, paths, *outPath2);
		paths.push_back(outPath2);
		floors = grid.GetFloors();
		water = grid.GetWater();
		
		Vector3 pos;
		while (outPath->PopWaypoint(pos)) {
			pathNodes.push_back(Vector3(pos.x, pos.y, pos.z));
		}
		
		
		while (outPath2->PopWaypoint(pos)) {
			pathNodes2.push_back(Vector3(pos.x, pos.y, pos.z));
		}

		
		
		
		for (int i = 0; i < floors.size(); i++) {
			for (int j = 0; j < pathNodes.size(); j++) {
				if (floors.at(i) == pathNodes.at(j)) {
					floors.erase(floors.begin() + i);
					i--;
					break;
				}
			}
		}
		for (int i = 0; i < floors.size(); i++) {
			for (int j = 0; j < pathNodes2.size(); j++) {
				if (floors.at(i) == pathNodes2.at(j)) {
					floors.erase(floors.begin() + i);
					i--;
					break;
				}
			}
		}
		
		agentObjects.push_back(AddAgentToWorld(Vector3(pathNodes.at(0).x, -28, pathNodes.at(0).z), Vector3(3, 3, 3), Vector4(0.5, 0.5, 1, 1)));
		agentObjects.push_back(AddAgentToWorld(Vector3(pathNodes2.at(0).x, -28, pathNodes2.at(0).z), Vector3(3, 3, 3), Vector4(0, 1, 1, 1)));

		AddFloorToWorld(Vector3(pathNodes.at(0).x, -32, pathNodes.at(0).z), Vector3(5, 1, 5), Vector4(1, 0, 1, 1));
		AddFloorToWorld(Vector3(pathNodes.at(pathNodes.size() - 1).x, -32, pathNodes.at(pathNodes.size() - 1).z), Vector3(5, 1, 5), Vector4(1, 0, 1, 1));
		AddFloorToWorld(Vector3(pathNodes2.at(0).x, -32, pathNodes2.at(0).z), Vector3(5, 1, 5), Vector4(1, 0, 1, 1));
		AddFloorToWorld(Vector3(pathNodes2.at(pathNodes2.size() - 1).x, -32, pathNodes2.at(pathNodes2.size() - 1).z), Vector3(5, 1, 5), Vector4(1, 0, 1, 1));

		bool isWater;
		for (int i = 1; i < pathNodes.size() - 1; i++) {
			isWater = false;
			for (int j = 0; j < water.size(); j++) {
				if (pathNodes.at(i) == water.at(j)) {
					pathObjects.push_back(AddWaterToWorld(Vector3(pathNodes.at(i).x, -32, pathNodes.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 1, 1)));
					pathObjects.at(i-1)->GetRenderObject()->AddPathCount(1);
					water.erase(water.begin() + j);
					isWater = true;
				}
			}
			if (!isWater) {
				pathObjects.push_back(AddFloorToWorld(Vector3(pathNodes.at(i).x, -32, pathNodes.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 0, 1)));
				pathObjects.at(i-1)->GetRenderObject()->AddPathCount(1);
				isWater = true;
			}
		}
		for (int i = 1; i < pathNodes2.size() - 1; i++) {
			isWater = false;
			for (int j = 0; j < water.size(); j++) {
				if (pathNodes2.at(i) == water.at(j)) {
					pathObjects2.push_back(AddWaterToWorld(Vector3(pathNodes2.at(i).x, -32, pathNodes2.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 1, 1)));
					pathObjects2.at(i - 1)->GetRenderObject()->AddPathCount(1);
					water.erase(water.begin() + j);
					isWater = true;
				}
			}
			if (!isWater) {
				bool created = false;
				for (int j = 1; j < pathNodes.size() - 1; j++) {
					if (pathNodes.at(j) == pathNodes2.at(i)) {
						pathObjects2.push_back(pathObjects.at(j-1));
						pathObjects.at(j - 1)->GetRenderObject()->AddPathCount(1);
						created = true;
					}
				}
				if (!created) {
					pathObjects2.push_back(AddFloorToWorld(Vector3(pathNodes2.at(i).x, -32, pathNodes2.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 0, 1)));
					pathObjects2.at(i - 1)->GetRenderObject()->AddPathCount(1);
				}
				isWater = true;
			}
		}
		for (int i = 0; i < grid.GetWalls().size(); i++) {
			AddWallToWorld(Vector3(grid.GetWalls().at(i).x, -30, grid.GetWalls().at(i).z), Vector3(5, 3, 5), Vector4(1, 1, 1, 1));
		}
		for (int i = 0; i < floors.size(); i++) {
			AddFloorToWorld(Vector3(floors.at(i).x, -32, floors.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 0, 1));
		}
		for (int i = 0; i < water.size(); i++) {
			AddWaterToWorld(Vector3(water.at(i).x, -32, water.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 1, 1));
		}
		time = Window::GetTimer()->GetTimeDelta();
		std::cout << "Time taken for algorithm to run: " + std::to_string(time) << std::endl;

	}
	else {
		
		startX = 10;
		endX = 80;
		startZ = 10;
		endZ = 80;

		Vector3 startPos(startX, 0, startZ);
		Vector3 endPos(endX, 0, endZ);
		NavigationGrid grid("Grid.txt");
		NavigationPath outPath;
		float time = Window::GetTimer()->GetTimeDelta();
		bool found = grid.FindPath(startPos, endPos, outPath);
		if (found) {
			floors = grid.GetFloors();
			water = grid.GetWater();

			Vector3 pos;
			while (outPath.PopWaypoint(pos)) {
				pathNodes.push_back(Vector3(pos.x, pos.y, pos.z));
			}

			for (int i = 0; i < floors.size(); i++) {
				for (int j = 0; j < pathNodes.size(); j++) {
					if (floors.at(i) == pathNodes.at(j)) {
						floors.erase(floors.begin() + i);
						i--;
						break;
					}
				}
			}

			AddFloorToWorld(Vector3(startX, -32, startZ), Vector3(5, 1, 5), Vector4(1, 0, 1, 1));
			AddFloorToWorld(Vector3(endX, -32, endZ), Vector3(5, 1, 5), Vector4(1, 0, 1, 1));

			bool isWater;
			for (int i = 1; i < pathNodes.size() - 1; i++) {
				isWater = false;
				for (int j = 0; j < water.size(); j++) {
					if (pathNodes.at(i) == water.at(j)) {
						pathObjects.push_back(AddWaterToWorld(Vector3(pathNodes.at(i).x, -32, pathNodes.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 1, 1)));
						water.erase(water.begin() + j);
						isWater = true;
					}
				}
				if (!isWater) {
					pathObjects.push_back(AddFloorToWorld(Vector3(pathNodes.at(i).x, -32, pathNodes.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 0, 1)));
					isWater = true;
				}
			}

			for (int j = 0; j < grid.GetClosedList().size(); j++) {
				for (int i = 0; i < floors.size(); i++) {
					if (floors.at(i) == grid.GetClosedList().at(j)->position) {
						closedList.push_back(AddClosedListToWorld(Vector3(floors.at(i).x, -32, floors.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 0, 1)));

						floors.erase(floors.begin() + i);

						break;
					}
				}
			}
			for (int i = 0; i < grid.GetWalls().size(); i++) {
				AddWallToWorld(Vector3(grid.GetWalls().at(i).x, -30, grid.GetWalls().at(i).z), Vector3(5, 3, 5), Vector4(1, 1, 1, 1));
			}
			for (int i = 0; i < floors.size(); i++) {
				AddFloorToWorld(Vector3(floors.at(i).x, -32, floors.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 0, 1));
			}
			for (int i = 0; i < water.size(); i++) {
				AddWaterToWorld(Vector3(water.at(i).x, -32, water.at(i).z), Vector3(5, 1, 5), Vector4(0, 0, 1, 1));
			}
			for (int i = 0; i < closedList.size(); i++) {
				cpObjects.push_back(closedList.at(i));
			}
			for (int i = 0; i < grid.GetClosedList().size(); i++) {
				for (int j = 0; j < pathObjects.size(); j++) {
					if (grid.GetClosedList().at(i)->position.x == pathObjects.at(j)->GetTransform().GetWorldPosition().x &&
						grid.GetClosedList().at(i)->position.z == pathObjects.at(j)->GetTransform().GetWorldPosition().z) {
						if (i > cpObjects.size()) {
							cpObjects.emplace(cpObjects.end(), pathObjects.at(j));
							break;
						}
						else {
							cpObjects.emplace(cpObjects.begin() + i - 1, pathObjects.at(j));
							break;
						}
					}
				}
			}
			time = Window::GetTimer()->GetTimeDelta();
			std::cout << "Time taken for algorithm to run: " + std::to_string(time) << std::endl;
		}
		else {
			std::cout << "The map or path are invalid." << std::endl;
		}
	}
}

void Demo::nextStep() {
	if (!isToggled) {
		if (!isMulti) {
			if (step < pathObjects.size()) {
				currentCost = 0;
				pathObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				for (int i = 0; i < step + 1; i++) {
					if (pathObjects.at(i)->GetName() == "Water") {
						currentCost += 10;
					}
					else {
						currentCost += 1;
					}
				}
				step++;
			}
		}
		else {
			currentCost = 0;
			if (step < pathObjects.size()) {
				pathObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			}
			if (step < pathObjects2.size()) {
				pathObjects2.at(step)->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
			}	
			for (int i = 0; i < step + 1; i++) {
				if (i < pathObjects.size()) {
					if (pathObjects.at(i)->GetName() == "Water") {
						currentCost += 10;
					}
					else {
						currentCost += 1;
					}
				}
				if (i == pathObjects.size()) {
					currentCost += 1;
				}
				if (i < pathObjects2.size()) {
					if (pathObjects2.at(i)->GetName() == "Water") {
						currentCost += 10;
					}
					else {
						currentCost += 1;
					}
				}
				if (i == pathObjects2.size()) {
					currentCost += 1;
				}
			}
			if (step < pathNodes.size() || step < pathNodes2.size()) {
				if (pathNodes.size() - 1 > step) {
					if (pathNodes.at(step) != pathNodes.at(step + 1)) {
						fuelCost += 1;
					}
				}
				if (pathNodes2.size() - 1 > step) {
					if (pathNodes2.at(step) != pathNodes2.at(step + 1)) {
						fuelCost += 1;
					}
				}
			}
			if (step < pathObjects.size() + 1 || step < pathObjects2.size() + 1) {
				step++;
			}
		}
	}
	else {	
		if (step < cpObjects.size()) {
			currentCost = 0;
			if (cpObjects.at(step)->GetName() == "ClosedList") {
				cpObjects.at(step)->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
			}
			else {
				cpObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			}

			step++;
		}
	}
	if (isMulti) {
		if (step < pathNodes.size()) {
			agentObjects.at(0)->GetTransform().SetWorldPosition(Vector3(pathNodes.at(step).x,
																	agentObjects.at(0)->GetRenderObject()->GetTransform()->GetWorldPosition().y,
																	pathNodes.at(step).z));
		}
		if (step < pathNodes2.size()) {
			agentObjects.at(1)->GetTransform().SetWorldPosition(Vector3(pathNodes2.at(step).x,
																		agentObjects.at(1)->GetRenderObject()->GetTransform()->GetWorldPosition().y,
																		pathNodes2.at(step).z));
		}
	}
}

void Demo::previousStep() {
	if (!isToggled) {
		if (!isMulti) {
			if (step > 0) {
				currentCost = 0;
				step--;
				if (pathObjects.at(step)->GetName() == "Water") {
					pathObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
				}
				else {
					pathObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
				}
				for (int i = 0; i < step; i++) {
					if (pathObjects.at(i)->GetName() == "Water") {
						currentCost += 10;
					}
					else {
						currentCost += 1;
					}
				}
			}
		}
		else {
			if (step > 0) {
				currentCost = 0;
				step--;
				if (step < pathObjects.size()) {
					if (pathObjects.at(step)->GetName() == "Water") {
						pathObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
					}
					else {
						pathObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
					}
				}
				if (step < pathObjects2.size()) {
					if(pathObjects2.size() > pathObjects.size() && pathObjects2.at(step)->GetRenderObject()->GetPathCount() > 1){
						pathObjects2.at(step)->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
					}
					else if (pathObjects2.at(step)->GetName() == "Water") {
						pathObjects2.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
					}
					else {
						pathObjects2.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
					}
				}
				for (int i = 0; i < step; i++) {
					if (i < pathObjects.size()) {
						if (pathObjects.at(i)->GetName() == "Water") {
							currentCost += 10;
						}
						else {
							currentCost += 1;
						}
					}
					if (i < pathObjects2.size()) {
						if (pathObjects2.at(i)->GetName() == "Water") {
							currentCost += 10;
						}
						else {
							currentCost += 1;
						}
					}
				}
			}
			if (step > 0) {
				if (pathNodes.size() - 1 > step) {
					if (pathNodes.at(step) != pathNodes.at(step + 1)) {
						fuelCost -= 1;
					}
				}
			}
			if (step > 0) {
				if (pathNodes2.size() - 1 > step) {
					if (pathNodes2.at(step) != pathNodes2.at(step + 1)) {
						fuelCost -= 1;
					}
				}
			}
			if (step == 0 && fuelCost != 0) {
				fuelCost = 0;
			}
		}
	}
	else {
		if (step > 0) {
			currentCost = 0;
			step--;
			if (cpObjects.at(step)->GetName() == "Water") {
				cpObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
			}
			else {
				cpObjects.at(step)->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
			}			
		}
	}
	if (isMulti) {
		if (step < pathNodes.size()) {
			agentObjects.at(0)->GetTransform().SetWorldPosition(Vector3(pathNodes.at(step).x,
				agentObjects.at(0)->GetRenderObject()->GetTransform()->GetWorldPosition().y,
				pathNodes.at(step).z));
		}
		if (step < pathNodes2.size()) {
			agentObjects.at(1)->GetTransform().SetWorldPosition(Vector3(pathNodes2.at(step).x,
				agentObjects.at(1)->GetRenderObject()->GetTransform()->GetWorldPosition().y,
				pathNodes2.at(step).z));
		}
	}
}

void Demo::toggleWorking() {
	if (!isMulti) {
		int correctStep = 0;
		int tempStep = step;
		bool pathFound = false;
		if (isToggled) {
			while (!pathFound) {
				for (int i = 0; i < pathObjects.size(); i++) {
					if (step == 0) {
						correctStep = 0;
						pathFound = true;
						break;
					}
					else if (cpObjects.at(step - 1) == pathObjects.at(i)) {
						correctStep = i + 1;
						pathFound = true;
						break;
					}
				}
				step--;
			}
			step = tempStep;
			while (step > 0) {
				previousStep();
			}
			isToggled = !isToggled;
			while (step != correctStep) {
				nextStep();
			}
		}
		else {
			while (!pathFound) {
				for (int i = 0; i < cpObjects.size(); i++) {
					if (step == 0) {
						correctStep = 0;
						pathFound = true;
						break;
					}
					else if (cpObjects.at(i) == pathObjects.at(step - 1)) {
						correctStep = i + 1;
						pathFound = true;
						break;
					}
				}
				step++;
			}
			step = tempStep;
			while (step > 0) {
				previousStep();
			}
			isToggled = !isToggled;
			while (step != correctStep) {
				nextStep();
			}
		}
	}
}

GameObject* Demo::AddWallToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour) {
	GameObject* wall = new GameObject("Wall");

	wall->GetTransform().SetWorldPosition(position);
	wall->GetTransform().SetWorldScale(dimensions);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->GetRenderObject()->SetColour(colour);

	world->AddGameObject(wall);

	return wall;
}

GameObject* Demo::AddAgentToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour) {
	GameObject* wall = new GameObject("Agent");

	wall->GetTransform().SetWorldPosition(position);
	wall->GetTransform().SetWorldScale(dimensions);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->GetRenderObject()->SetColour(colour);

	world->AddGameObject(wall);

	return wall;
}

GameObject* Demo::AddClosedListToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour) {
	GameObject* wall = new GameObject("ClosedList");

	wall->GetTransform().SetWorldPosition(position);
	wall->GetTransform().SetWorldScale(dimensions);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->GetRenderObject()->SetColour(colour);

	world->AddGameObject(wall);

	return wall;
}

GameObject* Demo::AddWaterToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour) {
	GameObject* water = new GameObject("Water");

	water->GetTransform().SetWorldPosition(position);
	water->GetTransform().SetWorldScale(dimensions);

	water->SetRenderObject(new RenderObject(&water->GetTransform(), cubeMesh, basicTex, basicShader));
	water->GetRenderObject()->SetColour(colour);

	world->AddGameObject(water);

	return water;
}

GameObject* Demo::AddFloorToWorld(const Vector3& position, Vector3 dimensions, Vector4 colour) {
	GameObject* floor = new GameObject("Floor");

	floor->GetTransform().SetWorldPosition(position);
	floor->GetTransform().SetWorldScale(dimensions);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(colour);

	world->AddGameObject(floor);

	return floor;
}