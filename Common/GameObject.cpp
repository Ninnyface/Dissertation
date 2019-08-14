#include "GameObject.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName)	{
	name			= objectName;
	isActive		= true;
	renderObject	= nullptr;
}

GameObject::~GameObject()	{
	delete renderObject;
}

