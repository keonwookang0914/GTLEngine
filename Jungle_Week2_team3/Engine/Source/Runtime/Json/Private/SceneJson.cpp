#include "SceneJson.h"
#include "Engine/PrimitiveType.h"
#include "json.hpp"
#include <fstream>
#include <iomanip>
#include <limits>
#include <string>
#include <unordered_set>

using json = nlohmann::ordered_json;

namespace
{
    auto RoundFloat = [](float v) -> double
    { return std::round(static_cast<double>(v) * 1e5) / 1e5; };
    json FVectorToJson(const FVector &V)
    {
        return json::array({RoundFloat(V.X), RoundFloat(V.Y), RoundFloat(V.Z)});
    }


    json FRotatorToJson(const FRotator &R) 
    {
        return json::array({RoundFloat(R.Roll), RoundFloat(R.Pitch), RoundFloat(R.Yaw)}); 
    }

    bool JsonTo3Floats(const json &J, float &A, float &B, float &C)
    {
        if (!J.is_array() || J.size() != 3)
        {
            return false;
        }

        try
        {
            A = J[0].get<float>();
            B = J[1].get<float>();
            C = J[2].get<float>();
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    bool JsonToFVector(const json &J, FVector &OutV)
    {
        return JsonTo3Floats(J, OutV.X, OutV.Y, OutV.Z);
    }

    // JSON: [X축 회전, Y축 회전, Z축 회전]
    bool JsonToRotator(const json &J, FRotator &OutR)
    {
        return JsonTo3Floats(J, OutR.Roll, OutR.Pitch, OutR.Yaw);
    }
} // namespace

namespace SceneJson
{
    bool SaveScene(const FString &FilePath, const SceneRawData &Scene)
    {
        json Root;
        Root["Version"] = Scene.Version;
        Root["NextUUID"] = Scene.NextUUID;

        json ActorsJson = json::object();

        for (const ActorRawData &Actor : Scene.Actors)
        {
            if (!PrimitiveType::IsValid(Actor.Type))
            {
                return false;
            }

            const char *TypeString = PrimitiveType::ToJsonString(Actor.Type);
            if (TypeString == nullptr)
            {
                return false;
            }

            json ActorJson;
            ActorJson["Location"] = FVectorToJson(Actor.Location);
            ActorJson["Rotation"] = FRotatorToJson(Actor.Rotation);
            ActorJson["Scale"] = FVectorToJson(Actor.Scale);
            ActorJson["Type"] = TypeString;

            ActorsJson[std::to_string(Actor.UUID)] = ActorJson;
        }

        Root["Primitives"] = ActorsJson;

        std::ofstream File(FilePath);
        if (!File.is_open())
        {
            return false;
        }

        File << Root.dump(2, ' ', false);
        return File.good();
    }

    bool LoadScene(const FString &FilePath, SceneRawData &OutScene)
    {
        std::ifstream File(FilePath);
        if (!File.is_open())
        {
            return false;
        }

        json Root;
        try
        {
            File >> Root;
        }
        catch (...)
        {
            return false;
        }

        SceneRawData TempScene;

        try
        {
            if (!Root.is_object())
            {
                return false;
            }

            if (!Root.contains("Version") || !Root.contains("NextUUID") ||
                !Root.contains("Primitives"))
            {
                return false;
            }

            TempScene.Version = Root.at("Version").get<uint32>();
            TempScene.NextUUID = Root.at("NextUUID").get<uint32>();

            if (TempScene.Version != 1)
            {
                return false;
            }

            const json &ActorsJson = Root.at("Primitives");
            if (!ActorsJson.is_object())
            {
                return false;
            }

            std::unordered_set<uint32> UUIDSet;

            for (auto It = ActorsJson.begin(); It != ActorsJson.end(); ++It)
            {
                const std::string UUIDString = It.key();
                const json       &ActorJson = It.value();

                if (!ActorJson.is_object())
                {
                    return false;
                }

                if (!ActorJson.contains("Location") || !ActorJson.contains("Rotation") ||
                    !ActorJson.contains("Scale") || !ActorJson.contains("Type"))
                {
                    return false;
                }

                ActorRawData Actor;

                try
                {
                    size_t        ParsedLength = 0;
                    unsigned long ParsedValue = std::stoul(UUIDString, &ParsedLength);

                    if (ParsedLength != UUIDString.size())
                    {
                        return false;
                    }

                    if (ParsedValue >
                        static_cast<unsigned long>(std::numeric_limits<uint32>::max()))
                    {
                        return false;
                    }

                    Actor.UUID = static_cast<uint32>(ParsedValue);
                }
                catch (...)
                {
                    return false;
                }

                if (UUIDSet.find(Actor.UUID) != UUIDSet.end())
                {
                    return false;
                }
                UUIDSet.insert(Actor.UUID);

                if (!JsonToFVector(ActorJson.at("Location"), Actor.Location))
                {
                    return false;
                }

                if (!JsonToRotator(ActorJson.at("Rotation"), Actor.Rotation))
                {
                    return false;
                }

                if (!JsonToFVector(ActorJson.at("Scale"), Actor.Scale))
                {
                    return false;
                }

                FString TypeString;
                try
                {
                    TypeString = ActorJson.at("Type").get<std::string>().c_str();
                }
                catch (...)
                {
                    return false;
                }

                if (!PrimitiveType::FromJsonString(TypeString, Actor.Type))
                {
                    return false;
                }

                if (!PrimitiveType::IsValid(Actor.Type))
                {
                    return false;
                }

                if (Actor.UUID >= TempScene.NextUUID)
                {
                    return false;
                }

                TempScene.Actors.push_back(Actor);
            }
        }
        catch (...)
        {
            return false;
        }

        OutScene = TempScene;
        return true;
    }
} // namespace SceneJson