#include "../Common/Window.h"
#include "NavigationGrid.h"
#include "Demo.h"

using namespace NCL;
using namespace CSC8503;
using namespace std;

int main() {

	Window*w = Window::CreateGameWindow("Pathfinding demo", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}


	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);
	
	Demo* d = new Demo();

	float timeout = 0;

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDelta() / 1000.0f;	
		timeout += dt;
		d->UpdateDemo(dt);

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT) && timeout > 0.1) {
			d->nextStep();
			timeout = 0;
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT) && timeout > 0.1) {
			d->previousStep();
			timeout = 0;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_V)) {
			d->toggleWorking();
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_M)) {
			d->swapMulti();
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_R)) {
			d->resetWorld();
		}
	
	}
	Window::DestroyGameWindow();
	return 0;
}