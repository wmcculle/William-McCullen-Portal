#pragma once
#include "Component.h"
#include "Math.h"

class CameraComponent : public Component
{
public:
	CameraComponent(class Actor* owner);

	void Update(float deltaTime) override;

	float GetPitchSpeed() const { return mPitchSpeed; }
	void SetPitchSpeed(float pitchSpeed) { mPitchSpeed = pitchSpeed; }

	float GetPitchAngle() const { return mPitchAngle; }
	void ResetPitchAngle() { mPitchAngle = 0.0f; }

private:
	float mTargetOffset = 50.0f;

	float mPitchAngle = 0.0f;
	float mPitchSpeed = 0.0f;
};