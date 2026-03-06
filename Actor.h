#pragma once
#include <vector>
#include <SDL2/SDL_stdinc.h>
#include "Math.h"
#include <string>

enum class ActorState
{
	Active,
	Paused,
	Destroy
};

class Actor
{
public:
	Actor(class Game* game, Actor* parent = nullptr);
	virtual ~Actor();

	// Update function called from Game (not overridable)
	void Update(float deltaTime);
	// ProcessInput function called from Game (not overridable)
	void ProcessInput(const Uint8* keyState, Uint32 mouseButtons, const Vector2& relativeMouse);

	// Getters/setters
	const Vector3& GetPosition() const { return mPosition; }
	void SetPosition(const Vector3& pos) { mPosition = pos; }

	const Vector3& GetScale() const { return mScale; }
	void SetScale(float scale) { mScale = Vector3(scale, scale, scale); }
	void SetScale(const Vector3& scale) { mScale = scale; }

	float GetRotation() const { return mRotation; }
	void SetRotation(float rotation) { mRotation = rotation; }

	ActorState GetState() const { return mState; }
	void SetState(ActorState state) { mState = state; }

	class Game* GetGame() { return mGame; }

	// Returns component of type T, or null if doesn't exist
	template <typename T>
	T* GetComponent() const
	{
		for (auto c : mComponents)
		{
			T* t = dynamic_cast<T*>(c);
			if (t != nullptr)
			{
				return t;
			}
		}

		return nullptr;
	}

	// Before you can implement Update, create a function in Actor called GetForward that returns the forward
	// direction vector of the actor. Since the game is 2D, you can compute this using the unit circle equation
	// covered in class. Remember that since +y is down in SDL, negate the y component of the equation.
	Vector3 GetForward() const;

	const Matrix4& GetWorldTransform() const { return mWorldTransform; }

	void CalcWorldTransform();

	float mRollAngle = 0.0f;

	void SetQuat(const Quaternion& quat) { mQuat = quat; }
	const Quaternion& GetQuat() const { return mQuat; }

	Vector3 GetRight() const;

	Vector3 GetQuatForward() const;

	Actor* GetParent() const { return mParent; }

	Vector3 GetWorldPosition() const;
	Vector3 GetWorldForward() const;

protected:
	// Any actor-specific update code (overridable)
	virtual void OnUpdate(float deltaTime);
	// Any actor-specific update code (overridable)
	virtual void OnProcessInput(const Uint8* keyState, Uint32 mouseButtons,
								const Vector2& relativeMouse);

	class Game* mGame;
	// Actor's state
	ActorState mState;

	// Transform
	Vector3 mPosition;
	Vector3 mScale;
	float mRotation;

	// Components
	std::vector<class Component*> mComponents;

private:
	friend class Component;
	// Adds component to Actor (this is automatically called
	// in the component constructor)
	void AddComponent(class Component* c);

	Matrix4 mWorldTransform;

	Quaternion mQuat;

	Matrix4 GetWorldRotTrans() const;

	Actor* mParent;
	std::vector<Actor*> mChildren;
	bool mInheritScale = false;

	void AddChild(Actor* actor);
	void RemoveChild(Actor* actor);
};
