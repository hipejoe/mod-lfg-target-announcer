#include "Chat.h"
#include "CommandScript.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Group.h"
#include "LFGMgr.h"
#include "Map.h"
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"

#include <string>
#include <vector>

using namespace Acore::ChatCommands;

namespace
{
    class LfgTargetAnnouncer
    {
    public:
        static bool SendAnnouncement(
            Group* group,
            uint32 lfgDungeonId,
            uint32 mapId)
        {
            if (!group ||
                lfgDungeonId == 0 ||
                mapId == 0)
            {
                return false;
            }

            std::string message =
                LfgTargetAnnouncer::GetMessage(
                    lfgDungeonId,
                    mapId);

            if (message.empty())
            {
                return false;
            }

            LfgTargetAnnouncer::SendMessageToGroup(
                group,
                message);

            return true;
        }

    private:
        static std::string GetMessage(
            uint32 lfgDungeonId,
            uint32 mapId)
        {
            QueryResult result = WorldDatabase.Query(
                "SELECT `message` "
                "FROM `mod_lfg_target_announcer` "
                "WHERE `lfg_dungeon_id` = {} "
                "AND `map_id` = {}",
                lfgDungeonId,
                mapId);

            if (!result)
            {
                return {};
            }

            Field* fields = result->Fetch();

            std::string message =
                fields[0].Get<std::string>();

            std::vector<std::string> targetNames =
                LfgTargetAnnouncer::GetTargetNames(
                    lfgDungeonId);

            if (targetNames.empty())
            {
                return message;
            }

            message += " Required target";

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

        static std::vector<std::string> GetTargetNames(
            uint32 lfgDungeonId)
        {
            std::vector<std::string> targetNames;

            QueryResult result = WorldDatabase.Query(
                "SELECT `target_name` "
                "FROM `mod_lfg_target_announcer_target` "
                "WHERE `lfg_dungeon_id` = {} "
                "AND `required` = 1 "
                "ORDER BY `target_order`",
                lfgDungeonId);

            if (!result)
            {
                return targetNames;
            }

            do
            {
                Field* fields = result->Fetch();

                std::string targetName =
                    fields[0].Get<std::string>();

                if (!targetName.empty())
                {
                    targetNames.push_back(
                        targetName);
                }
            } while (result->NextRow());

            return targetNames;
        }

        static void SendMessageToGroup(
            Group* group,
            std::string const& message)
        {
            for (GroupReference* groupReference =
                group->GetFirstMember();
                groupReference != nullptr;
                groupReference = groupReference->next())
            {
                Player* groupMember =
                    groupReference->GetSource();

                if (!groupMember ||
                    !groupMember->IsInWorld() ||
                    !groupMember->GetSession())
                {
                    continue;
                }

                ChatHandler(groupMember->GetSession())
                    .SendSysMessage(message);
            }
        }
    };

    class LfgTargetAnnouncerScript final : public AllMapScript
    {
    public:
        LfgTargetAnnouncerScript()
            : AllMapScript("LfgTargetAnnouncerScript")
        {
        }

        void OnPlayerEnterAll(
            Map* map,
            Player* player) override
        {
            if (!sConfigMgr->GetOption<bool>(
                "LfgTargetAnnouncer.Enable",
                true))
            {
                return;
            }

            if (!map ||
                !player ||
                !map->IsDungeon() ||
                map->IsRaid())
            {
                return;
            }

            Group* group = player->GetGroup();

            if (!group ||
                !group->isLFGGroup())
            {
                return;
            }

            /*
             * Use the group leader's map-entry event to avoid sending
             * the message once for every group member entering the map.
             */
            if (group->GetLeaderGUID() != player->GetGUID())
            {
                return;
            }

            uint32 lfgDungeonId =
                sLFGMgr->GetDungeon(
                    group->GetGUID());

            if (lfgDungeonId == 0)
            {
                return;
            }

            LfgTargetAnnouncer::SendAnnouncement(
                group,
                lfgDungeonId,
                map->GetId());
        }
    };

    class LfgTargetAnnouncerCommandScript final : public CommandScript
    {
    public:
        LfgTargetAnnouncerCommandScript()
            : CommandScript(
                "LfgTargetAnnouncerCommandScript")
        {
        }

        ChatCommandTable GetCommands() const override
        {
            static ChatCommandTable commandTable =
            {
                {
                    "lfgannounce",
                    HandleLfgAnnounceCommand,
                    rbac::RBAC_PERM_COMMAND_HELP,
                    Console::No
                }
            };

            return commandTable;
        }

    private:
        static bool HandleLfgAnnounceCommand(
            ChatHandler* handler,
            char const* /*args*/)
        {
            if (!handler ||
                !handler->GetSession())
            {
                return false;
            }

            Player* player =
                handler->GetSession()->GetPlayer();

            if (!player)
            {
                return false;
            }

            if (!sConfigMgr->GetOption<bool>(
                "LfgTargetAnnouncer.Enable",
                true))
            {
                handler->SendSysMessage(
                    "|cffff0000[LFG System]:|r "
                    "LFG target announcements are disabled.");

                return true;
            }

            Map* map = player->GetMap();

            if (!map ||
                !map->IsDungeon() ||
                map->IsRaid())
            {
                handler->SendSysMessage(
                    "|cffff0000[LFG System]:|r "
                    "You are not currently in an LFG dungeon.");

                return true;
            }

            Group* group = player->GetGroup();

            if (!group ||
                !group->isLFGGroup())
            {
                handler->SendSysMessage(
                    "|cffff0000[LFG System]:|r "
                    "You are not currently in an LFG group.");

                return true;
            }

            uint32 lfgDungeonId =
                sLFGMgr->GetDungeon(
                    group->GetGUID());

            if (lfgDungeonId == 0)
            {
                handler->SendSysMessage(
                    "|cffff0000[LFG System]:|r "
                    "Unable to determine the current "
                    "LFG dungeon.");

                return true;
            }

            if (!LfgTargetAnnouncer::SendAnnouncement(
                group,
                lfgDungeonId,
                map->GetId()))
            {
                handler->SendSysMessage(
                    "|cffff0000[LFG System]:|r "
                    "No announcement is configured for "
                    "this LFG dungeon.");

                return true;
            }

            return true;
        }
    };
}

/*
 * Keep this registration function outside the anonymous namespace.
 * LfgTargetAnnouncer_loader.cpp links against this global symbol.
 */
void AddSC_mod_lfg_target_announcer()
{
    new LfgTargetAnnouncerScript();
    new LfgTargetAnnouncerCommandScript();
}
