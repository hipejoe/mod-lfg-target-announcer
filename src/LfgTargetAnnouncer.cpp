#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Group.h"
#include "LFGMgr.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"

#include <string>

namespace
{
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
                sLFGMgr->GetDungeon(group->GetGUID());

            if (lfgDungeonId == 0)
            {
                return;
            }

            std::string message =
                LfgTargetAnnouncerScript::GetMessage(
                    lfgDungeonId,
                    map->GetId());

            if (message.empty())
            {
                return;
            }

            LfgTargetAnnouncerScript::SendMessageToGroup(
                group,
                message);
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

            return fields[0].Get<std::string>();
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
}

/*
 * Keep this registration function outside the anonymous namespace.
 * LfgTargetAnnouncer_loader.cpp links against this global symbol.
 */
void AddSC_mod_lfg_target_announcer()
{
    new LfgTargetAnnouncerScript();
}
