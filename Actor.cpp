#include "Actor.h"
#include "Game.h"
#include "Component.h"
#include <algorithm>

Actor::Actor(Game* game, Actor* parent)
: mGame(game)
, mState(ActorState::Active)
, mPosition(Vector3::Zero)
, mScale(Vector3(1.0f, 1.0f, 1.0f))
, mRotation(0.0f)
, mParent(parent)
{
	if (mParent == nullptr)
	{
		mGame->AddActor(this); // add itself to the game
	}
	else
	{
		mParent->AddChild(this);
	}
}

Actor::~Actor()
{
	mGame->GetAudio()->RemoveActor(this);

	// Delete each child
	while (!mChildren.empty())
	{
		delete mChildren.back();
	}

	if (mParent == nullptr)
	{
		mGame->RemoveActor(this); // remove itself from the game
	}
	else
	{
		mParent->RemoveChild(this);
	}

	// Loop over all its components and call delete on each
	for (auto component : mComponents)
	{
		delete component;
	}

	mComponents.clear(); // clear the vector of components
}

void Actor::Update(float deltaTime)
{
	CalcWorldTransform();

	if (GetState() == ActorState::Active)
	{
		// 1. Loop over all its components and call Update on each of them
		for (auto component : mComponents)
		{
			component->Update(deltaTime);
		}

		// 2. After the loop, call the OnUpdate member function.
		// The OnUpdate member function is overridable by subclasses, so it?s where custom update logic can get added for a child class of Actor.
		OnUpdate(deltaTime);
	}

	CalcWorldTransform();

	// Update each child
	for (auto child : mChildren)
	{
		child->Update(deltaTime);
	}
}

void Actor::OnUpdate(float deltaTime)
{
}

void Actor::ProcessInput(const Uint8* keyState, Uint32 mouseButtons, const Vector2& relativeMouse)
{
	if (GetState() == ActorState::Active)
	{
		for (auto component : mComponents)
		{
			component->ProcessInput(keyState, mouseButtons, relativeMouse);
		}

		OnProcessInput(keyState, mouseButtons, relativeMouse);
	}
}

void Actor::OnProcessInput(const Uint8* keyState, Uint32 mouseButtons, const Vector2& relativeMouse)
{
}

void Actor::AddComponent(Component* c)
{
	mComponents.emplace_back(c);
	std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) {
		return a->GetUpdateOrder() < b->GetUpdateOrder();
	});
}

// Return the forward direction vector of the actor.
// Remember that since +y is down in SDL, negate the y component of the equation.
Vector3 Actor::GetForward() const
{
	return Vector3(cos(GetRotation()), sin(GetRotation()), 0.0f);
}

void Actor::CalcWorldTransform()
{
	// World = Scale * Rotation * Translation
	mWorldTransform = Matrix4::CreateScale(mScale) * Matrix4::CreateRotationZ(mRotation) *
					  Matrix4::CreateRotationX(mRollAngle) * Matrix4::CreateFromQuaternion(mQuat) *
					  Matrix4::CreateTranslation(mPosition);

	// If there's a parent, do additional calculation to turn mWorldTransform into the actor's actual world transform
	if (mParent != nullptr)
	{
		if (mInheritScale)
		{
			mWorldTransform *= mParent->GetWorldTransform();
		}
		else
		{
			mWorldTransform *= mParent->GetWorldRotTrans();
		}
	}
}

Vector3 Actor::GetRight() const
{
	float rightAngle = mRotation + Math::PiOver2;
	return Vector3(cos(rightAngle), sin(rightAngle), 0.0f);
}

Vector3 Actor::GetQuatForward() const
{
	Vector3 fwd = Vector3::Transform(Vector3::UnitX, GetQuat());
	fwd.Normalize();
	return fwd;
}

// This calculates and returns a temporary matrix that uses the same formula as the world transform, except without the scale matrix.
// This is so that children have the choice of whether they’ll inherit the parent’s scale or not.
Matrix4 Actor::GetWorldRotTrans() const
{
	// World = Rotation * Translation
	Matrix4 worldRotTransform =
		Matrix4::CreateRotationZ(mRotation) * Matrix4::CreateRotationX(mRollAngle) *
		Matrix4::CreateFromQuaternion(mQuat) * Matrix4::CreateTranslation(mPosition);

	if (mParent)
	{
		worldRotTransform *= mParent->GetWorldRotTrans();
	}

	return worldRotTransform;
}

Vector3 Actor::GetWorldPosition() const
{
	return mWorldTransform.GetTranslation();
}

Vector3 Actor::GetWorldForward() const
{
	return mWorldTransform.GetXAxis();
}

void Actor::AddChild(Actor* actor)
{
	mChildren.emplace_back(actor);
}

void Actor::RemoveChild(Actor* actor)
{
	auto iter = std::find(mChildren.begin(), mChildren.end(), actor);
	if (iter != mChildren.end())
	{
		mChildren.erase(iter);
	}
}