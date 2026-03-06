#include "LevelLoader.h"
#include <rapidjson/document.h>
#include "Math.h"
#include <SDL2/SDL.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "Actor.h"
#include "MeshComponent.h"
#include "Block.h"
#include "Player.h"
#include "Game.h"
#include "Prop.h"
#include "PortalGun.h"
#include "Door.h"
#include "EnergyLauncher.h"
#include "EnergyCatcher.h"
#include "EnergyGlass.h"
#include "EnergyCube.h"
#include "TurretBase.h"
#include "VOTrigger.h"

namespace
{
	// Helper functions to get other types
	bool GetFloatFromJSON(const rapidjson::Value& inObject, const char* inProperty,
						  float& outFloat);
	bool GetIntFromJSON(const rapidjson::Value& inObject, const char* inProperty, int& outInt);
	bool GetStringFromJSON(const rapidjson::Value& inObject, const char* inProperty,
						   std::string& outStr);
	bool GetBoolFromJSON(const rapidjson::Value& inObject, const char* inProperty, bool& outBool);
	bool GetVectorFromJSON(const rapidjson::Value& inObject, const char* inProperty,
						   Vector3& outVector);
	bool GetQuaternionFromJSON(const rapidjson::Value& inObject, const char* inProperty,
							   Quaternion& outQuat);
	bool GetStringArrayFromJSON(const rapidjson::Value& inObject, const char* inProperty,
								std::vector<std::string>& outArray);
} // namespace

void LoadActor(const rapidjson::Value& actorValue, Game* game, Actor* parent)
{
	if (actorValue.IsObject())
	{

		// Lookup actor type
		std::string type = actorValue["type"].GetString();
		Actor* actor = nullptr;

		if (type == "Block")
		{
			Block* block = new Block(game);
			actor = block;
		}
		else if (type == "Player")
		{
			Vector3 initialPos;
			GetVectorFromJSON(actorValue, "pos", initialPos);
			bool hasGun = false;
			GetBoolFromJSON(actorValue, "gun", hasGun);

			// Handle construction of a player!
			Player* player = new Player(game);
			player->SetInitialPos(initialPos);
			if (hasGun)
			{
				player->GiveGun();
			}

			actor = player;

			game->SetPlayer(player);
		}
		// Add else ifs for other actor types
		else if (type == "Prop")
		{
			std::string meshName;
			GetStringFromJSON(actorValue, "mesh", meshName);

			bool usesAlpha = false;
			GetBoolFromJSON(actorValue, "alpha", usesAlpha);

			bool usesCollision = false;
			GetBoolFromJSON(actorValue, "collision", usesCollision);

			Prop* prop = new Prop(game, meshName, usesAlpha, usesCollision);
			actor = prop;
		}
		else if (type == "PortalGun")
		{
			PortalGun* portalGun = new PortalGun(game);
			actor = portalGun;

			game->SetPortalGun(portalGun);
		}
		else if (type == "Door")
		{
			// "name" (required; string) - specifies a unique name for that door
			std::string doorName;
			GetStringFromJSON(actorValue, "name", doorName);

			Door* door = new Door(game, doorName);
			actor = door;
		}
		else if (type == "EnergyLauncher")
		{
			// "cooldown" (optional; float) - specifies how frequently the energy launcher shoots a pellet.
			// If this is not specified, the cooldown for the EnergyLauncher should default to 1.5f.
			float pelletCooldown = 1.5f;
			GetFloatFromJSON(actorValue, "cooldown", pelletCooldown);

			// "door" (optional; string) - which door name this energy launcher is associated with.
			// If the door is open, the energy launcher will not fire any more pellets
			std::string doorName;
			GetStringFromJSON(actorValue, "door", doorName);

			EnergyLauncher* energyLauncher = new EnergyLauncher(game, pelletCooldown, doorName);
			actor = energyLauncher;
		}
		else if (type == "EnergyCatcher")
		{
			// "door" (optional; string) - which door name this energy catcher is associated with.
			// If the energy catcher catches a pellet, it will “open” the corresponding door
			std::string doorName;
			GetStringFromJSON(actorValue, "door", doorName);

			EnergyCatcher* energyCatcher = new EnergyCatcher(game, doorName);
			actor = energyCatcher;
		}
		else if (type == "EnergyGlass")
		{
			EnergyGlass* energyGlass = new EnergyGlass(game);
			actor = energyGlass;
		}
		else if (type == "EnergyCube")
		{
			EnergyCube* energyCube = new EnergyCube(game);
			actor = energyCube;
		}
		else if (type == "Turret")
		{
			actor = new TurretBase(game);
		}
		else if (type == "VOTrigger")
		{
			std::string doorName;
			GetStringFromJSON(actorValue, "door", doorName);

			std::string nextLevel;
			GetStringFromJSON(actorValue, "level", nextLevel);

			std::vector<std::string> sounds;
			GetStringArrayFromJSON(actorValue, "sounds", sounds);

			std::vector<std::string> subtitles;
			GetStringArrayFromJSON(actorValue, "subtitles", subtitles);

			actor = new VOTrigger(game, doorName, nextLevel, sounds, subtitles);
		}

		// Set properties of actor
		if (actor)
		{
			Vector3 pos;
			if (GetVectorFromJSON(actorValue, "pos", pos))
			{
				actor->SetPosition(pos);
			}

			float scale = 1.0f;
			Vector3 scaleV;
			if (GetFloatFromJSON(actorValue, "scale", scale))
			{
				actor->SetScale(scale);
			}
			else if (GetVectorFromJSON(actorValue, "scale", scaleV))
			{
				actor->SetScale(scaleV);
			}

			float rot = 0.0f;
			if (GetFloatFromJSON(actorValue, "rot", rot))
			{
				actor->SetRotation(rot);
			}

			Quaternion q;
			if (GetQuaternionFromJSON(actorValue, "quat", q))
			{
				// TODO: Set actor's quaternion member to q
				actor->SetQuat(q);
			}

			int textureIdx = 0;
			if (GetIntFromJSON(actorValue, "texture", textureIdx))
			{
				MeshComponent* mesh = actor->GetComponent<MeshComponent>();
				if (mesh)
				{
					mesh->SetTextureIndex(static_cast<int>(textureIdx));
				}
			}

			// Calculate world transform once
			actor->CalcWorldTransform();

			// See if we have any children
			auto childIter = actorValue.FindMember("children");
			if (childIter != actorValue.MemberEnd())
			{
				auto& childArr = childIter->value;
				if (childArr.IsArray())
				{
					for (rapidjson::SizeType i = 0; i < childArr.Size(); i++)
					{
						if (childArr[i].IsObject())
						{
							LoadActor(childArr[i], game, actor);
						}
					}
				}
			}
		}
	}
}

// Sanjay: This is suboptimal
#if defined(__APPLE__) || defined(__clang_analyzer__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

bool LevelLoader::Load(class Game* game, const std::string& fileName)
{
	std::ifstream file(fileName);

	if (!file.is_open())
	{
		SDL_Log("Level file %s not found", fileName.c_str());
		return false;
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	std::string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		SDL_Log("Level file %s is not valid JSON", fileName.c_str());
		return false;
	}

	// Loop through "actors" array
	const rapidjson::Value& actors = doc["actors"];
	if (actors.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < actors.Size(); i++)
		{
			// This index should be an object
			LoadActor(actors[i], game, nullptr);
		}
	}

	return true;
}

namespace
{

	bool GetFloatFromJSON(const rapidjson::Value& inObject, const char* inProperty, float& outFloat)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsFloat())
		{
			return false;
		}

		outFloat = property.GetFloat();
		return true;
	}

	bool GetIntFromJSON(const rapidjson::Value& inObject, const char* inProperty, int& outInt)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsInt())
		{
			return false;
		}

		outInt = property.GetInt();
		return true;
	}

	bool GetStringFromJSON(const rapidjson::Value& inObject, const char* inProperty,
						   std::string& outStr)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsString())
		{
			return false;
		}

		outStr = property.GetString();
		return true;
	}

	bool GetBoolFromJSON(const rapidjson::Value& inObject, const char* inProperty, bool& outBool)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsBool())
		{
			return false;
		}

		outBool = property.GetBool();
		return true;
	}

	bool GetVectorFromJSON(const rapidjson::Value& inObject, const char* inProperty,
						   Vector3& outVector)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsArray() || property.Size() != 3)
		{
			return false;
		}

		for (rapidjson::SizeType i = 0; i < 3; i++)
		{
			if (!property[i].IsFloat())
			{
				return false;
			}
		}

		outVector.x = property[0].GetFloat();
		outVector.y = property[1].GetFloat();
		outVector.z = property[2].GetFloat();

		return true;
	}

	bool GetQuaternionFromJSON(const rapidjson::Value& inObject, const char* inProperty,
							   Quaternion& outQuat)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsArray() || property.Size() != 4)
		{
			return false;
		}

		for (rapidjson::SizeType i = 0; i < 4; i++)
		{
			if (!property[i].IsFloat())
			{
				return false;
			}
		}

		outQuat.x = property[0].GetFloat();
		outQuat.y = property[1].GetFloat();
		outQuat.z = property[2].GetFloat();
		outQuat.w = property[3].GetFloat();

		return true;
	}

	bool GetStringArrayFromJSON(const rapidjson::Value& inObject, const char* inProperty,
								std::vector<std::string>& outArray)
	{
		outArray.clear();

		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsArray())
		{
			return false;
		}

		for (rapidjson::SizeType i = 0; i < property.Size(); i++)
		{
			if (!property[i].IsString())
			{
				return false;
			}
			outArray.emplace_back(property[i].GetString());
		}

		return true;
	}
} // namespace

#if defined(__APPLE__) || defined(__clang_analyzer__)
#pragma clang diagnostic pop
#endif
