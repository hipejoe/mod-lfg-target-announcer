#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "Log.h"
#include "ScriptMgr.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    struct LfgTargetDefinition
    {
        uint32 LfgDungeonId;
        std::vector<uint32> CreatureEntries;
        std::vector<std::string> EventTargets;
    };

    /*
     * Add the exact LFG dungeon IDs and completion targets here.
     *
     * Creature names are not stored here. They are loaded from
     * creature_template during the initial database population.
     */
    std::vector<LfgTargetDefinition> const LfgTargetDefinitions =
    {
        // Utgarde Keep - Normal
        {
            163,
            {
                23954 // Ingvar the Plunderer
            },
            {}
        },

        // Utgarde Keep - Heroic
        {
            242,
            {
                23954 // Ingvar the Plunderer
            },
            {}
        },

        // Additional examples:
        //
        // {
        //     lfgDungeonId,
        //     {
        //         creatureEntry1,
        //         creatureEntry2
        //     },
        //     {
        //         "Complete the required event"
        //     }
        // }
    };

    class LfgTargetAnnouncerInitializer
    {
    public:
        static void Populate()
        {
            if (!LfgTargetAnnouncerInitializer::IsTableEmpty())
            {
                LOG_INFO(
                    "module",
                    "LFG target announcer table already contains data; "
                    "initial population skipped.");

                return;
            }

            LOG_INFO(
                "module",
                "Populating `mod_lfg_target_announcer`.");

            std::unordered_map<uint32, LfgTargetDefinition const*>
                targetDefinitions =
                LfgTargetAnnouncerInitializer::CreateDefinitionMap();

            uint32 insertedCount = 0;

            for (LFGDungeonEntry const* dungeon : sLFGDungeonStore)
            {
                if (!LfgTargetAnnouncerInitializer::ShouldIncludeDungeon(
                    dungeon))
                {
                    continue;
                }

                auto definitionIterator =
                    targetDefinitions.find(dungeon->ID);

                if (definitionIterator ==
                    targetDefinitions.end())
                {
                    LOG_WARN(
                        "module",
                        "No target definition exists for LFG dungeon ID {} "
                        "({}). The entry was not inserted.",
                        dungeon->ID,
                        dungeon->Name[LOCALE_enUS]);

                    continue;
                }

                if (LfgTargetAnnouncerInitializer::InsertDungeon(
                    dungeon,
                    *definitionIterator->second))
                {
                    ++insertedCount;
                }
            }

            LOG_INFO(
                "module",
                "Inserted {} LFG target announcer entries.",
                insertedCount);
        }

    private:
        static bool IsTableEmpty()
        {
            QueryResult result = WorldDatabase.Query(
                "SELECT 1 "
                "FROM `mod_lfg_target_announcer` "
                "LIMIT 1");

            return !result;
        }

        static bool ShouldIncludeDungeon(
            LFGDungeonEntry const* dungeon)
        {
            if (!dungeon)
            {
                return false;
            }

            if (dungeon->MapID == 0)
            {
                return false;
            }

            return dungeon->TypeID == lfg::LFG_TYPE_DUNGEON ||
                dungeon->TypeID == lfg::LFG_TYPE_HEROIC;
        }

        static std::unordered_map<
            uint32,
            LfgTargetDefinition const*> CreateDefinitionMap()
        {
            std::unordered_map<
                uint32,
                LfgTargetDefinition const*> definitions;

            for (LfgTargetDefinition const& definition :
                LfgTargetDefinitions)
            {
                definitions.emplace(
                    definition.LfgDungeonId,
                    &definition);
            }

            return definitions;
        }

        static bool InsertDungeon(
            LFGDungeonEntry const* dungeon,
            LfgTargetDefinition const& definition)
        {
            std::string dungeonName =
                dungeon->Name[LOCALE_enUS];

            if (dungeonName.empty())
            {
                dungeonName =
                    "LFG Dungeon " +
                    std::to_string(dungeon->ID);
            }

            std::vector<std::string> targetNames =
                LfgTargetAnnouncerInitializer::GetCreatureNames(
                    definition.CreatureEntries);

            targetNames.insert(
                targetNames.end(),
                definition.EventTargets.begin(),
                definition.EventTargets.end());

            if (targetNames.empty())
            {
                LOG_WARN(
                    "module",
                    "No valid completion targets were found for LFG "
                    "dungeon ID {} ({}).",
                    dungeon->ID,
                    dungeonName);

                return false;
            }

            std::string message =
                LfgTargetAnnouncerInitializer::BuildMessage(
                    dungeonName,
                    targetNames);

            std::string comment =
                LfgTargetAnnouncerInitializer::BuildComment(
                    dungeon,
                    dungeonName);

            WorldDatabase.EscapeString(message);
            WorldDatabase.EscapeString(comment);

            WorldDatabase.Execute(
                "INSERT INTO `mod_lfg_target_announcer` "
                "("
                "`lfg_dungeon_id`, "
                "`map_id`, "
                "`message`, "
                "`comment`"
                ") "
                "VALUES ({}, {}, '{}', '{}')",
                dungeon->ID,
                dungeon->MapID,
                message,
                comment);

            return true;
        }

        static std::vector<std::string> GetCreatureNames(
            std::vector<uint32> const& creatureEntries)
        {
            std::vector<std::string> targetNames;

            for (uint32 creatureEntry : creatureEntries)
            {
                QueryResult result = WorldDatabase.Query(
                    "SELECT `name` "
                    "FROM `creature_template` "
                    "WHERE `entry` = {}",
                    creatureEntry);

                if (!result)
                {
                    LOG_ERROR(
                        "module",
                        "Creature entry {} does not exist in "
                        "`creature_template`.",
                        creatureEntry);

                    continue;
                }

                std::string creatureName =
                    result->Fetch()[0].Get<std::string>();

                if (!creatureName.empty())
                {
                    targetNames.push_back(
                        creatureName);
                }
            }

            return targetNames;
        }

        static std::string BuildMessage(
            std::string const& dungeonName,
            std::vector<std::string> const& targetNames)
        {
            std::string message =
                "|cff00ff00[LFG System]:|r To complete " +
                dungeonName +
                ", complete the following requirement";

            if (targetNames.size() != 1)
            {
                message += "s";
            }

            message += ": ";

            for (std::size_t index = 0;
                index < targetNames.size();
                ++index)
            {
                if (index > 0)
                {
                    if (index == targetNames.size() - 1)
                    {
                        message += " and ";
                    }
                    else
                    {
                        message += ", ";
                    }
                }

                message += "|cffffff00";
                message += targetNames[index];
                message += "|r";
            }

            message += ".";

            return message;
        }

        static std::string BuildComment(
            LFGDungeonEntry const* dungeon,
            std::string const& dungeonName)
        {
            std::string difficulty;

            if (dungeon->TypeID == lfg::LFG_TYPE_HEROIC)
            {
                difficulty = "Heroic";
            }
            else
            {
                difficulty = "Normal";
            }

            return dungeonName +
                " - " +
                difficulty +
                " - LFG ID " +
                std::to_string(dungeon->ID);
        }
    };

    class LfgTargetAnnouncerWorldScript final : public WorldScript
    {
    public:
        LfgTargetAnnouncerWorldScript()
            : WorldScript("LfgTargetAnnouncerWorldScript")
        {
        }

        void OnStartup() override
        {
            LfgTargetAnnouncerInitializer::Populate();
        }
    };
}

void AddSC_mod_lfg_target_announcer_initializer()
{
    new LfgTargetAnnouncerWorldScript();
}
