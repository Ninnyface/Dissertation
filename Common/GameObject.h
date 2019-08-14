#pragma once
#include "Transform.h"

#include "RenderObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {

		class GameObject	{
		public:
			GameObject(string name = "");
			~GameObject();

			bool IsActive() const {
				return isActive;
			}

			const Transform& GetConstTransform() const {
				return transform;
			}

			Transform& GetTransform() {
				return transform;
			}

			void SetTransform(Transform transform) {
				this->transform = transform;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}


			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			const string& GetName() const {
				return name;
			}

			void SetName(string name) {
				this->name = name;
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				
				//std::cout << "OnCollisionEnd event occured!\n";
			}


		protected:
			Transform			transform;

			RenderObject*		renderObject;

			bool	isActive;
			string	name;
		};
	}
}

