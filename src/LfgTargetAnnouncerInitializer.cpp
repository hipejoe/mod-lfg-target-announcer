#include "ScriptMgr.h"

/*
 * SQL owns schema creation and initial data population.
 *
 * Base schema:
 *
 * data/sql/db-world/base/
 *     mod_lfg_target_announcer.sql
 *
 * One-time data updates:
 *
 * data/sql/db-world/updates/
 *     2026_08_15_00_lfg_dbc_reference.sql
 *     2026_08_15_01_mod_lfg_target_announcer_from_dbc.sql
 *
 * This module contains runtime LFG target announcement behavior.
 */
void AddSC_mod_lfg_target_announcer()
{
}
