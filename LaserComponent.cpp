#include "LaserComponent.h"
#include "Shader.h"
#include "Mesh.h"
#include "Actor.h"
#include "Game.h"
#include "Renderer.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Portal.h"

LaserComponent::LaserComponent(Actor* owner)
: MeshComponent(owner, true)
{
	SetMesh(GetGame()->GetRenderer()->GetMesh("Assets/Meshes/Laser.gpmesh"));
}

void LaserComponent::Update(float deltaTime)
{
	if (mIsDisabled)
	{
		return;
	}

	// Clear out the line segment vector
	mLineSegments.clear();
	// Create the first line segment and insert it into the vector
	mLineSegments.push_back(
		LineSegment(mOwner->GetWorldPosition(),
					mOwner->GetWorldPosition() + mOwner->GetWorldForward() * 350.0f));

	// Test for collisions
	CastInfo outInfo;
	if (SegmentCast(GetGame()->GetActors(), mLineSegments.back(), outInfo, mIgnoreActor))
	{
		mLastHitActor = outInfo.mActor;
		mLineSegments.back().mEnd = outInfo.mPoint;

		// If the first line segment’s SegmentCast hits a portal, you need to create a second line segment that
		// represents the part of the laser that’s on the other side of the portal
		// You can only create the second line segment if the portal collision occurs and there is both an orange and blue portal in the world
		Portal* entryPortal = dynamic_cast<Portal*>(outInfo.mActor);
		if (entryPortal != nullptr && GetGame()->GetBluePortal() != nullptr &&
			GetGame()->GetOrangePortal() != nullptr)
		{
			// Create an additional segment and insert it into the vector, if needed (based on portals)

			Vector3 initialDirection = mLineSegments.back().mStart - mLineSegments.back().mEnd;
			initialDirection.Normalize();

			Portal* outPortal = nullptr;
			if (entryPortal == GetGame()->GetBluePortal())
			{
				outPortal = GetGame()->GetOrangePortal();
			}
			else
			{
				outPortal = GetGame()->GetBluePortal();
			}

			Vector3 outPortalDirection = OutPortalDirection(initialDirection, outPortal);

			Vector3 startPosition = outPortal->GetPosition() + outPortalDirection * 0.5f;

			mLineSegments.push_back(
				LineSegment(startPosition, startPosition + outPortalDirection * 350.0f));

			// This second line segment also needs to test for collisions and stop if there is a collision
			if (SegmentCast(GetGame()->GetActors(), mLineSegments.back(), outInfo, outPortal))
			{
				mLastHitActor = outInfo.mActor;
				mLineSegments.back().mEnd = outInfo.mPoint;
			}
			else
			{
				mLastHitActor = nullptr;
			}
		}
	}
	else
	{
		mLastHitActor = nullptr;
	}

	// Make sure it doesn't crash if portal is deleted
	if (dynamic_cast<Portal*>(mLastHitActor))
	{
		mLastHitActor = nullptr;
	}
}

void LaserComponent::Draw(Shader* shader)
{
	if (mMesh)
	{
		for (LineSegment lineSegment : mLineSegments)
		{
			// Set the world transform
			shader->SetMatrixUniform("uWorldTransform", CalcLaserTransform(lineSegment));
			// Set the active texture
			Texture* t = mMesh->GetTexture(mTextureIndex);
			if (t)
			{
				t->SetActive();
			}
			// Set the mesh's vertex array as active
			VertexArray* va = mMesh->GetVertexArray();
			va->SetActive();
			// Draw
			glDrawElements(GL_TRIANGLES, va->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
		}
	}
}

Matrix4 LaserComponent::CalcLaserTransform(LineSegment lineSegment)
{
	// World = Scale * Rotation * Translation

	// Scale Matrix

	Matrix4 mWorldTransform = Matrix4::CreateScale(lineSegment.Length(), 1.0f, 1.0f);

	// Rotation Matrix

	Quaternion laserQuaternion;

	Vector3 originalFacing = Vector3::UnitX;
	Vector3 desiredFacing = lineSegment.mStart - lineSegment.mEnd;
	desiredFacing.Normalize();

	float dotProduct = Vector3::Dot(originalFacing, desiredFacing);

	// If originalFacing and desiredFacing are collinear and facing in the same direction
	if (dotProduct == 1.0f)
	{
		// No rotation
		laserQuaternion = Quaternion::Identity;
	}
	// If originalFacing and desiredFacing are collinear and facing in the opposite direction
	else if (dotProduct == -1.0f)
	{
		// yaw of 180 degrees
		laserQuaternion = Quaternion(Vector3::UnitZ, Math::Pi);
	}
	else
	{
		Vector3 axisOfRotation = Vector3::Cross(originalFacing, desiredFacing);
		axisOfRotation.Normalize();

		// Find the angle of rotation (the angle between originalFacing and desiredFacing)
		float angleOfRotation = Math::Acos(dotProduct);

		laserQuaternion = Quaternion(axisOfRotation, angleOfRotation);
	}

	mWorldTransform *= Matrix4::CreateFromQuaternion(laserQuaternion);

	// Translation Matrix

	mWorldTransform *= Matrix4::CreateTranslation(lineSegment.PointOnSegment(0.5f));

	return mWorldTransform;
}

Vector3 LaserComponent::OutPortalDirection(Vector3 initialDirection, Portal* oppositePortal)
{
	// 2. Get the inverse world transform matrix of the “in” portal. (This gives a matrix that can transform from
	// world space into the object space of the “in” portal)
	Matrix4 inverseWorldTransformMatrix = GetOwner()->GetWorldTransform();
	inverseWorldTransformMatrix.Invert();

	// 3. Use Vector3::Transform to transform (1) by (2), making sure to pass in 0.0f for the third parameter.
	// (The direction vector is now in object space of the “in” portal)
	Vector3 transform = Vector3::Transform(initialDirection, inverseWorldTransformMatrix, 0.0f);

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

	// 7. Normalize the vector from (6) to get the correct “out” portal direction vector
	outTransform.Normalize();

	return outTransform;
}

// If it’s disabled you should clear out the laser vector and not create line segments in Update anymore
void LaserComponent::SetDisabled()
{
	mIsDisabled = true;
	mLineSegments.clear();
}