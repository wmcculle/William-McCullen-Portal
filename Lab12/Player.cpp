#include "Player.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "PlayerMove.h"
#include "CameraComponent.h"
#include "PortalGun.h"
#include "PlayerMesh.h"
#include "HealthComponent.h"
#include "HUD.h"
#include "Random.h"

Player::Player(Game* game)
: Actor(game)
{
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(50.0f, 100.0f, 50.0f);
	mPlayerMove = new PlayerMove(this);
	mCameraComponent = new CameraComponent(this);
	mHealthComponent = new HealthComponent(this);

	mHUD = new HUD(this);

	mDeathSounds.push_back("Glados-PlayerDead1.ogg");
	mDeathSounds.push_back("Glados-PlayerDead2.ogg");
	mDeathSounds.push_back("Glados-PlayerDead3.ogg");
	mDeathSounds.push_back("Glados-PlayerDead4.ogg");

	mDeathSubtitles.push_back("Congratulations! The test is now over.");
	mDeathSubtitles.push_back(
		"Thank you for participating in this Aperture Science computer-aided enrichment activity.");
	mDeathSubtitles.push_back("Goodbye.");
	mDeathSubtitles.push_back("You're not a good person. You know that, right?");

	// For the player’s HealthComponent, set the mOnDeath callback so that it forces the game to reload the level.
	mHealthComponent->SetOnDeath([this] {
		int rand = Random::GetIntRange(0, 3);
		mDeathSound = mGame->GetAudio()->PlaySound(mDeathSounds[rand]);
		mGame->GetPlayer()->GetHUD()->ShowSubtitle(mDeathSubtitles[rand]);

		// When the player dies the HUD shows the "Assets/Textures/UI/DamageOverlay.png" texture on screen
		mHUD->SetPlayerDead();
	});

	mHealthComponent->SetOnDamage([this](const Vector3& location) {
		// Get the vector from the player to the location (that you get in to the callback). Set the z
		// component to 0 and normalize the vector.
		Vector3 playerToLocation = location - GetPosition();
		playerToLocation.z = 0.0f;
		playerToLocation.Normalize();

		// Get the forward vector of the player, set the z component to 0 and normalize the vector
		Vector3 playerForward = GetForward();
		playerForward.z = 0.0f;
		playerForward.Normalize();

		// Calculate the angle between (2) and (1), using the cross product between them to figure out if it’s
		// positive or negative
		float dot = Vector3::Dot(playerForward, playerToLocation);

		Vector3 cross = Vector3::Cross(playerForward, playerToLocation);

		// Call the PlayerTakeDamage function on the HUD, passing in the angle
		if (cross.z >= 0)
		{
			mHUD->PlayerTakeDamage(-Math::Acos(dot));
		}
		else
		{
			mHUD->PlayerTakeDamage(Math::Acos(dot));
		}
	});
}

void Player::OnUpdate(float deltaTime)
{
	CheckPortalGunCollision();
}

void Player::CheckPortalGunCollision()
{
	// When the player intersects with the PortalGun
	if (!HasGun() && GetGame()->GetPortalGun() != nullptr &&
		GetComponent<CollisionComponent>()->Intersect(
			GetGame()->GetPortalGun()->GetComponent<CollisionComponent>()))
	{
		// Destroy the PortalGun
		GetGame()->GetPortalGun()->SetState(ActorState::Destroy);
		// Give the player the gun
		GiveGun();
	}
}

void Player::GiveGun()
{
	mHasGun = true;
	new PlayerMesh(GetGame());
}