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

            if (!group || !group->isLFGGroup())
            {
                return;
            }

            if (group->GetLeaderGUID() != player->GetGUID())
            {
                return;
            }

            uint32 lfgDungeonId = sLFGMgr->GetDungeon(
                group->GetGUID());

            if (lfgDungeonId == 0)
            {
                return;
            }

            QueryResult result = WorldDatabase.Query(
                "SELECT `message` "
                "FROM `mod_lfg_target_announcer` "
                "WHERE `lfg_dungeon_id` = {} "
                "AND `map_id` = {}",
                lfgDungeonId,
                map->GetId());

            if (!result)
            {
                return;
            }

            std::string message =
                result->Fetch()[0].Get<std::string>();

            if (message.empty())
            {
                return;
            }

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

void AddSC_mod_lfg_target_announcer()
{
    new LfgTargetAnnouncerScript();
}
