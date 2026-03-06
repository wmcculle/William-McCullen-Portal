#include "Portal.h"
#include "Game.h"
#include "PortalMeshComponent.h"
#include "CollisionComponent.h"
#include "Renderer.h"
#include "Player.h"

Portal::Portal(Game* game, int textureIndex)
: Actor(game)
{
	mPortalMeshComponent = new PortalMeshComponent(this);
	mPortalMeshComponent->SetTextureIndex(textureIndex);

	mCollisionComponent = new CollisionComponent(this);
}

void Portal::OnUpdate(float deltaTime)
{
	if (this == mGame->GetBluePortal())
	{
		mGame->GetRenderer()->GetBluePortal().mView = CalcViewMatrix(mGame->GetOrangePortal());
	}
	else
	{
		mGame->GetRenderer()->GetOrangePortal().mView = CalcViewMatrix(mGame->GetBluePortal());
	}
}

Matrix4 Portal::CalcViewMatrixBasic(Portal* oppositePortal)
{
	if (oppositePortal != nullptr)
	{
		// Calculate the correct basic portal view matrix
		Vector3 eye = oppositePortal->GetPosition() - oppositePortal->GetQuatForward() * 5.0f;
		Vector3 target = oppositePortal->GetPosition() + oppositePortal->GetQuatForward() * 5.0f;
		Vector3 up = oppositePortal->GetWorldTransform().GetZAxis();

		return Matrix4::CreateLookAt(eye, target, up);
	}

	// return if there is no matching opposite portal
	return Matrix4::CreateScale(0.0f);
}

Matrix4 Portal::CalcViewMatrix(Portal* oppositePortal)
{
	// “In” portal is this instance of the Portal object
	// “Out” portal is the “opposite portal” that is the portal you’d exit at if you travelled through this

	// If the “out” portal is null, then still return a zeroed scale matrix
	if (oppositePortal == nullptr)
	{
		return Matrix4::CreateScale(0.0f);
	}

	// If the “in” portal is facing directly up or down, just fall back to using CalcViewMatrixBasic to calculate
	// the portal view, as our complex view will not work in that case.
	if (Math::Abs(GetQuatForward().z) == 1)
	{
		return CalcViewMatrixBasic(oppositePortal);
	}

	// Otherwise, use the complex portal calculations to calculate the portal
	return OutPortalDirection(oppositePortal);
}

Matrix4 Portal::OutPortalDirection(Portal* oppositePortal)
{
	// Calculate the vector from the player to the “in” portal
	Vector3 playerToInPortal = this->GetPosition() - mGame->GetPlayer()->GetPosition();

	// If this vector’s length is 0, you should return the matrix that you saved in the member variable
	// that was the valid view matrix you calculated on the last frame
	if (playerToInPortal.Length() == 0.0f)
	{
		return mViewMatrix;
	}
	// Otherwise, normalize the player to “in” vector. (This vector is expressed in world space)
	else
	{
		playerToInPortal.Normalize();
	}

	// 2. Get the inverse world transform matrix of the “in” portal. (This gives a matrix that can transform from
	// world space into the object space of the “in” portal)
	Matrix4 inverseWorldTransformMatrix = GetWorldTransform();
	inverseWorldTransformMatrix.Invert();

	// 3. Use Vector3::Transform to transform (1) by (2), making sure to pass in 0.0f for the third parameter.
	// (The direction vector is now in object space of the “in” portal)
	Vector3 transform = Vector3::Transform(playerToInPortal, inverseWorldTransformMatrix, 0.0f);

	// 4. Create a Z rotation matrix rotating by Math::Pi
	Matrix4 zRotationMatrix = Matrix4::CreateRotationZ(Math::Pi);

	// 5. Use Vector3::Transform to transform (3) by (4), passing in 0.0f for the third parameter. (This rotates by
	// Pi about the Z so it turns around “behind” the portal)
	Vector3 turnTransform = Vector3::Transform(transform, zRotationMatrix, 0.0f);

	// 6. Use Vector3::Transform to transform (5) by the “out” portal’s world transform matrix, passing in 0.0f
	// for the third parameter. (This will transform the direction vector back into world space, but now
	// accounting for the transform of the “out” portal)
	Vector3 outTransform = Vector3::Transform(turnTransform, oppositePortal->GetWorldTransform(),
											  0.0f);

	// 7. Normalize the outTransform vector to get the correct “out” portal direction vector
	outTransform.Normalize();

	// 8. Once you have the correct “out” portal direction vector, the rest of the look at matrix is the same
	// idea as the basic portal:
	Vector3 eye = oppositePortal->GetPosition() - outTransform * 5.0f;
	Vector3 target = oppositePortal->GetPosition() + outTransform * 5.0f;
	Vector3 up = oppositePortal->GetWorldTransform().GetZAxis();

	// Save the view matrix in a member variable so that if you need to, I can return the view matrix
	// from last frame.
	mViewMatrix = Matrix4::CreateLookAt(eye, target, up);

	// Then, in CalcViewMatrix, after you calculate the the look at matrix in the “complex”, case:
	// 1. Set the z component of the “out” forward vector to 0.0f
	outTransform.z = 0.0f;

	// 2. Normalize the vector
	outTransform.Normalize();

	// 3. Use the technique you used in part 1 to calculate a yaw angle given a vector, and save that in the
	// “out” yaw variable
	float dot = Vector3::Dot(Vector3::UnitX, outTransform);

	// Cross product to decide if you should turn left or right.
	// Our game's coordinate system is left-handed, so we should use the "left hand rule" instead

	// Calculate forward × normalized v
	Vector3 cross = Vector3::Cross(Vector3::UnitX, outTransform);

	// By left hand rule:
	// Positive z means clockwise rotation (turn right)
	if (cross.z >= 0)
	{
		mOutYaw = Math::Acos(dot);
	}
	// Negative z means counter-clockwise rotation(turn left)
	else
	{
		mOutYaw = -Math::Acos(dot);
	}

	return mViewMatrix;
}