#include "Chat.h"
#include "DatabaseEnv.h"
#include "Group.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Config.h"

#include <string>

class LfgTargetAnnouncerScript : public AllMapScript
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

        std::string message = this->GetMessage(
            map->GetId());

        if (message.empty())
        {
            return;
        }

        this->SendMessageToGroup(
            group,
            message);
    }

private:
    std::string GetMessage(uint32 mapId) const
    {
        QueryResult result = WorldDatabase.Query(
            "SELECT `message` "
            "FROM `mod_lfg_target_announcer` "
            "WHERE `map_id` = {}",
            mapId);

        if (!result)
        {
            return {};
        }

        Field* fields = result->Fetch();

        return fields[0].Get<std::string>();
    }

    void SendMessageToGroup(
        Group* group,
        std::string const& message) const
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

void AddSC_mod_lfg_target_announcer()
{
    new LfgTargetAnnouncerScript();
}
