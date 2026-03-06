#include "HUD.h"
#include "Actor.h"
#include "Font.h"
#include "Texture.h"
#include "Game.h"
#include "Renderer.h"

HUD::HUD(Actor* owner)
: UIComponent(owner)
{
	mFont = new Font();
	mFont->Load("Assets/Inconsolata-Regular.ttf");

	mDamageIndicatorTexture =
		GetGame()->GetRenderer()->GetTexture("Assets/Textures/UI/DamageIndicator.png");
}

HUD::~HUD()
{
	mFont->Unload();
	delete mFont;
}

void HUD::Draw(class Shader* shader)
{
	if (mSubtitleTexture != nullptr)
	{
		// Calculate the Vector2 screen position for the subtitles:
		// The x-component is 0
		// The y-component is -325.0f + mSubtitleTexture->GetHeight() / 2.0f
		Vector2 screenPosition = Vector2(0.0f, -325.0f + mSubtitleTexture->GetHeight() / 2.0f);

		// The trick to add the shadow is just that right before you draw the subtitle texture, first draw the
		// subtitle shadow texture but at the calculated position plus an offset of (2.0f, -2.0f).
		DrawTexture(shader, mSubtitleShadowTexture, screenPosition + Vector2(2.0f, -2.0f));

		// Draw the subtitle texture
		DrawTexture(shader, mSubtitleTexture, screenPosition);
	}

	// If the damage indicator time is > 0, call DrawTexture with the following parameters:
	if (mDamageIndicatorTimer > 0.0f)
	{
		DrawTexture(shader, mDamageIndicatorTexture, Vector2::Zero, 1.0f, mDamageIndicatorAngle);
	}

	if (mPlayerDead)
	{
		// When the player dies the HUD shows the "Assets/Textures/UI/DamageOverlay.png" texture on screen
		DrawTexture(shader,
					GetGame()->GetRenderer()->GetTexture("Assets/Textures/UI/DamageOverlay.png"));
	}
}

void HUD::ShowSubtitle(std::string text)
{
	// If the subtitle texture pointer is not null
	if (mSubtitleTexture != nullptr)
	{
		// Call Unload on both the subtitle and subtitle shadow textures, delete both, and set both pointers to null.
		mSubtitleTexture->Unload();
		mSubtitleShadowTexture->Unload();

		delete mSubtitleTexture;
		delete mSubtitleShadowTexture;

		mSubtitleTexture = nullptr;
		mSubtitleShadowTexture = nullptr;
	}

	// If the text to show isn’t empty:
	if (!text.empty())
	{
		// Concatenate "GLaDOS: " to the start of the text to show
		text = "GLaDOS: " + text;

		// Call RenderText on the font once with the text from (a), passing in Color::LightGreen and saving the
		// texture in the subtitle texture
		mSubtitleTexture = mFont->RenderText(text, Color::LightGreen);

		// Call RenderText a second time with the same text from (a), this time passing in Color::Black and
		// saving it in the subtitle shadow texture
		mSubtitleShadowTexture = mFont->RenderText(text, Color::Black);
	}
}

void HUD::PlayerTakeDamage(float angle)
{
	mDamageIndicatorAngle = angle;
	mDamageIndicatorTimer = DAMAGE_INDICATOR_TIME;
}

void HUD::Update(float deltaTime)
{
	if (mDamageIndicatorTimer > 0.0f)
	{
		mDamageIndicatorTimer -= deltaTime;
	}
}