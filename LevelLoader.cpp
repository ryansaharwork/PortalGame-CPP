#include "LevelLoader.h"
#include <rapidjson/document.h>
#include "Math.h"
#include <SDL3/SDL.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "Actor.h"
#include "Block.h"
#include "Door.h"
#include "EnergyCatcher.h"
#include "EnergyLauncher.h"
#include "EnergyGlass.h"
#include "EnergyCube.h"
#include "MeshComponent.h"
#include "Game.h"
#include "Player.h"
#include "PortalGun.h"
#include "Prop.h"
#include "TurretBase.h"
#include "VOTrigger.h"

namespace
{
	// Helper functions to get other types
	// NOLINTBEGIN
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
	// NOLINTEND
} // namespace

void LoadActor(const rapidjson::Value& actorValue, Actor* parent)
{
	if (actorValue.IsObject())
	{
		// Lookup actor type
		std::string type = actorValue["type"].GetString();
		Actor* actor = nullptr;

		if (type == "Block")
		{
			Block* block = gGame.CreateActor<Block>();
			actor = block;
		}

		else if (type == "Player")
		{
			Player* player = gGame.CreateActor<Player>();
			actor = player;
			gGame.SetPlayer(player);

			// Store player's initial pos
			Vector3 initialPos;
			if (GetVectorFromJSON(actorValue, "pos", initialPos))
			{
				player->SetInitialPos(initialPos);
			}

			// Start with portal gun if "gun" is true
			if (actorValue.HasMember("gun") && actorValue["gun"].IsBool() &&
				actorValue["gun"].GetBool())
			{
				player->GiveGun();
			}
		}

		else if (type == "Prop")
		{
			// Default properties
			bool usesAlpha = false;
			bool hasCollision = false;
			std::string meshName;

			// Load properties (optional)
			GetBoolFromJSON(actorValue, "alpha", usesAlpha);
			GetBoolFromJSON(actorValue, "collision", hasCollision);
			GetStringFromJSON(actorValue, "mesh", meshName);

			// Create prop, set as curr actor
			Prop* prop = gGame.CreateActor<Prop>();
			actor = prop;

			// Apply optional properties
			prop->SetMesh(meshName, usesAlpha);
			if (hasCollision)
			{
				prop->EnableCollision();
			}
		}

		else if (type == "PortalGun")
		{
			PortalGun* portalGun = gGame.CreateActor<PortalGun>();
			actor = portalGun;
		}

		else if (type == "Door")
		{
			Door* door = gGame.CreateActor<Door>();
			actor = door;

			std::string name;
			if (GetStringFromJSON(actorValue, "name", name))
			{
				door->SetName(name);
				gGame.RegisterDoor(name, door);
			}
		}

		else if (type == "EnergyLauncher")
		{
			EnergyLauncher* launcher = gGame.CreateActor<EnergyLauncher>();
			actor = launcher;

			std::string doorName;
			if (GetStringFromJSON(actorValue, "door", doorName))
			{
				launcher->SetDoorName(doorName);
			}

			float cooldown = 1.5f;
			if (GetFloatFromJSON(actorValue, "cooldown", cooldown))
			{
				launcher->SetCooldown(cooldown);
			}
		}

		else if (type == "EnergyCatcher")
		{
			EnergyCatcher* catcher = gGame.CreateActor<EnergyCatcher>();
			actor = catcher;

			std::string doorName;
			if (GetStringFromJSON(actorValue, "door", doorName))
			{
				catcher->SetDoorName(doorName);
			}
		}

		else if (type == "EnergyGlass")
		{
			EnergyGlass* glass = gGame.CreateActor<EnergyGlass>();
			actor = glass;
		}

		else if (type == "EnergyCube")
		{
			EnergyCube* cube = gGame.CreateActor<EnergyCube>();
			actor = cube;
		}

		else if (type == "Turret")
		{
			TurretBase* turret = gGame.CreateActor<TurretBase>();
			actor = turret;
		}

		else if (type == "VOTrigger")
		{
			VOTrigger* voTrigger = gGame.CreateActor<VOTrigger>();
			actor = voTrigger;

			std::string doorName;
			if (GetStringFromJSON(actorValue, "door", doorName))
			{
				voTrigger->SetDoorName(doorName);
			}

			std::string nextLevel;
			if (GetStringFromJSON(actorValue, "level", nextLevel))
			{
				voTrigger->SetNextLevel(nextLevel);
			}

			std::vector<std::string> sounds;
			if (GetStringArrayFromJSON(actorValue, "sounds", sounds))
			{
				voTrigger->SetSounds(sounds);
			}

			std::vector<std::string> subtitles;
			if (GetStringArrayFromJSON(actorValue, "subtitles", subtitles))
			{
				voTrigger->SetSubtitles(subtitles);
			}
		}

		// Set properties of actor
		if (actor)
		{
			Vector3 pos;
			if (GetVectorFromJSON(actorValue, "pos", pos))
			{
				actor->GetTransform().SetPosition(pos);
			}

			float scale = 1.0f;
			Vector3 scaleV;
			if (GetFloatFromJSON(actorValue, "scale", scale))
			{
				actor->GetTransform().SetScale(scale);
			}
			else if (GetVectorFromJSON(actorValue, "scale", scaleV))
			{
				actor->GetTransform().SetScale(scaleV);
			}

			float rot = 0.0f;
			if (GetFloatFromJSON(actorValue, "rot", rot))
			{
				actor->GetTransform().SetRotation(rot);
			}

			Quaternion q;
			if (GetQuaternionFromJSON(actorValue, "quat", q))
			{
				actor->GetTransform().SetQuat(q);
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
							LoadActor(childArr[i], actor);
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

bool LevelLoader::Load(const std::string& fileName)
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
			LoadActor(actors[i], nullptr);
		}
	}

	return true;
}

// NOLINTBEGIN
namespace
{
	[[maybe_unused]] bool GetFloatFromJSON(const rapidjson::Value& inObject, const char* inProperty,
										   float& outFloat)
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

	[[maybe_unused]] bool GetIntFromJSON(const rapidjson::Value& inObject, const char* inProperty,
										 int& outInt)
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

	[[maybe_unused]] bool GetStringFromJSON(const rapidjson::Value& inObject,
											const char* inProperty, std::string& outStr)
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

	[[maybe_unused]] bool GetBoolFromJSON(const rapidjson::Value& inObject, const char* inProperty,
										  bool& outBool)
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

	[[maybe_unused]] bool GetVectorFromJSON(const rapidjson::Value& inObject,
											const char* inProperty, Vector3& outVector)
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

	[[maybe_unused]] bool GetQuaternionFromJSON(const rapidjson::Value& inObject,
												const char* inProperty, Quaternion& outQuat)
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

		outQuat.Set(property[0].GetFloat(), property[1].GetFloat(), property[2].GetFloat(),
					property[3].GetFloat());

		return true;
	}

	[[maybe_unused]] bool GetStringArrayFromJSON(const rapidjson::Value& inObject,
												 const char* inProperty,
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
// NOLINTEND

#if defined(__APPLE__) || defined(__clang_analyzer__)
#pragma clang diagnostic pop
#endif
