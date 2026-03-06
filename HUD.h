#pragma once
#include "UIComponent.h"
#include <string>

class HUD : public UIComponent
{
public:
	HUD(class Actor* owner);
	~HUD();

	void Draw(class Shader* shader) override;

	void ShowSubtitle(std::string text);

	void Update(float deltaTime) override;

	void PlayerTakeDamage(float angle);

	void SetPlayerDead() { mPlayerDead = true; }

private:
	class Font* mFont;

	class Texture* mSubtitleTexture = nullptr;
	class Texture* mSubtitleShadowTexture = nullptr;

	class Texture* mDamageIndicatorTexture;
	float mDamageIndicatorAngle = 0.0f;
	const float DAMAGE_INDICATOR_TIME = 1.5f;
	float mDamageIndicatorTimer = 0.0f;

	bool mPlayerDead = false;
};