/*
 * Copyright (C) 2012-2014 JadeCore <http://www.pandashan.com/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "throne_of_thunder.h"
#include "Vehicle.h"
#include "ThreatManager.h"

enum eSpells
{
    // Gara'Jal's Soul
    SPELL_LINGERING_PRESENCE             = 136467,
    SPELL_POSSESSED                      = 136442,
    // Frost King Malakk
    SPELL_FRIGID_ASSAULT                 = 136904,
    SPELL_FRIGID_ASSAULT_STUN            = 136910,
    SPELL_BITING_COLD                    = 136917,
    SPELL_BITING_COLD_AURA               = 136992,
    SPELL_FROSTBITE                      = 136990,
    SPELL_FROSTBITE_PERIODIC             = 136922,
    SPELL_FROSTBITE_AURA                 = 137575,
    // Kaz'Ra Jin
    SPELL_RECKLESS_CHARGE_AREATRIGGER    = 138026,
    SPELL_RECKLESS_CHARGE_ROLLING        = 137117, // aura
    SPELL_RECKLESS_CHARGE_MOVEMENT       = 137131,
    SPELL_RECKLESS_CHARGE_DAMAGE         = 137133,
    SPELL_RECKLESS_CHARGE_KNOCK_BACK     = 137122,
    SPELL_OVERLOAD                       = 137149,
    SPELL_GENERIC_STUN                   = 135781,
    // Sul
    SPELL_SAND_BOLT                      = 136189,
    SPELL_QUICKSAND                      = 136521,
    SPELL_SAND_PERIODIC_DMG              = 136860,
    SPELL_SAND_VISUAL                    = 136851,
    SPELL_ENSNARED                       = 136878,
    SPELL_ENTRAPPED                      = 136857,
    SPELL_SANDSTORM                      = 136894,
    SPELL_SANDSTORM_DUMMY                = 136895,
    SPELL_FORTIFIED                      = 136864,
    // High Priestress Mar'Li
    SPELL_WRATH_OF_THE_LOA_BLESSED       = 137344,
    SPELL_WRATH_OF_THE_LOA_SHADOW        = 137347,
    SPELL_BLESSED_LOA_SPIRIT_SUMMON      = 137200,
    SPELL_SHADOWED_LOA_SPIRIT_SUMMONED   = 137351,
    SPELL_SHADOWED_GIFT                  = 137407,
    SPELL_MARKED_SOUL                    = 137359,
    SPELL_DARK_POWER                     = 136507,
    // Heroic Mode
    SPELL_SOUL_FRAGMENT                  = 137641,
    SPELL_SOUL_FRAGMENT_SWITCH           = 137643,
    SPELL_DISCHARGE                      = 137166,
    SPELL_BODY_HEAT                      = 137084,
    SPELL_CHILLED_TO_THE_BONE            = 137085,
    SPELL_TREACHEROUS_GROUND             = 137614,
    SPELL_TREACHEROUS_GROUND_RESIZE      = 137629,
    SPELL_TWISTED_FATE_PRINCIPAL         = 137891,
    SPELL_TWISTED_FATE_SECOND_PRINCIPAL  = 137962,
    SPELL_TWISTED_FATE_LINK_VISUAL       = 137967,
    SPELL_TWISTED_FATE_PERIODIC          = 137986,
    SPELL_SHADOW_VISUAL                  = 32395
};

enum eEvents
{
    EVENT_RECKLESS_CHARGE                        = 1,
    EVENT_RECKLESS_CHARGE_AREATRIGGER            = 2,
    EVENT_RECKLESS_CHARGE_AREATRIGGER_DMG        = 3,
    EVENT_FRIGID_ASSAULT                         = 4,
    EVENT_LINGERING_PRESENCE_MALAKK              = 5,
    EVENT_LINGERING_PRESENCE_KAZRA_JIN           = 6,
    EVENT_LINGERING_PRESENCE_HIGH_PRIESTRESS     = 7,
    EVENT_LINGERING_PRESENCE_SUL_THE_SANDCRAWLER = 8,
    EVENT_BITING_COLD                            = 9,
    EVENT_FROSTBITE                              = 10,
    EVENT_SAND_BOLT                              = 11,
    EVENT_QUICKSAND                              = 12,
    EVENT_SANDSTORM                              = 13,
    EVENT_WRATH_OF_THE_LOA_BLESSED               = 14,
    EVENT_WRATH_OF_THE_LOA_SHADOW                = 15,
    EVENT_BLESSED_LOA_SPIRIT_SUMMON              = 16,
    EVENT_HEAL_WEAKER_TROLL                      = 17,
    EVENT_SHADOWED_LOA_SPIRIT_SUMMON             = 18,
    EVENT_OS_PLAYER                              = 19,
    EVENT_DARK_POWER                             = 20,
    EVENT_SOUL_FRAGMENT                          = 21,
    EVENT_TWISTED_FATE                           = 22,
    EVENT_TWISTED_FATE_SECOND                    = 23,
    EVENT_ENSNARED                               = 24
};

enum eSays
{
};

enum eActions
{
    ACTION_SCHEDULE_AT_DMG                         = 1,
    ACTION_SCHEDULE_FRIGID_ASSAULT                 = 2,
    ACTION_SCHEDULE_POSSESSION                     = 3,
    ACTION_SCHEDULE_FROSTBITE                      = 4,
    ACTION_SCHEDULE_SANDSTROM                      = 5,
    ACTION_SANDSTORM                               = 6,
    ACTION_SCHEDULE_WRATH_OF_THE_LOA_SHADOW        = 7,
    ACTION_SCHEDULE_SHADOWED_SPIRIT_SPAWN          = 8,
    ACTION_SOUL_FRAGMENT                           = 9,
    ACTION_TREACHEROUS_GROUND                      = 10
};

enum eDatas
{
    DATA_LINGERING_PRESENCE_COEF,
    DATA_SPELL_DMG_MULTIPLIER
};

uint32 entries[4] = {NPC_KAZRA_JIN, NPC_SUL_THE_SANDCRAWLER, NPC_FROST_KING_MALAKK, NPC_HIGH_PRIESTRESS_MAR_LI};
uint32 possessed[4] = {0, 0, 0, 0};

void StartFight(InstanceScript* instance, Creature* me, Unit* /*target*/)
{
    if (!instance)
        return;

    if (!instance->CheckRequiredBosses(DATA_CONCIL_OF_ELDERS))
    {
        if (me->GetAI())
            me->AI()->EnterEvadeMode();

        return;
    }

    if (instance->GetBossState(DATA_CONCIL_OF_ELDERS) == IN_PROGRESS)
        return; // Prevent recursive calls

    instance->SetBossState(DATA_CONCIL_OF_ELDERS, IN_PROGRESS);
    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

    uint32 mobEntries[4] = {NPC_KAZRA_JIN, NPC_SUL_THE_SANDCRAWLER, NPC_HIGH_PRIESTRESS_MAR_LI, NPC_FROST_KING_MALAKK};

    for (uint32 entry : mobEntries)
        if (Creature* boss = instance->instance->GetCreature(instance->GetData64(entry)))
            boss->SetInCombatWithZone();

    if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
        if (garaJalSoul->GetAI())
            garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
}

bool isAlonePossessed(InstanceScript* instance)
{
    uint32 bossEntries[4] = {NPC_FROST_KING_MALAKK, NPC_HIGH_PRIESTRESS_MAR_LI, NPC_SUL_THE_SANDCRAWLER, NPC_KAZRA_JIN};
    for (uint32 entry : bossEntries)
        if (Creature* boss = instance->instance->GetCreature(instance->GetData64(entry)))
            if (boss->HasAura(SPELL_POSSESSED))
                return false;

    return true;
}

// Gara'Jal's Soul - 69182
class npc_gara_jal_s_soul : public CreatureScript
{
    public:
        npc_gara_jal_s_soul() : CreatureScript("npc_gara_jal_s_soul") { }

        struct npc_gara_jal_s_soulAI : public ScriptedAI
        {
            npc_gara_jal_s_soulAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            EventMap events;
            InstanceScript* pInstance;
            uint32 targetFaction;

            void Reset()
            {
                targetFaction = 0;
                events.Reset();
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_SCHEDULE_POSSESSION)
                {
                    uint8 i = urand(0, 3);

                    while (entries[i] == possessed[i])
                        i = urand(0, 3);

                    switch (entries[i])
                    {
                        case NPC_KAZRA_JIN:
                        events.ScheduleEvent(EVENT_LINGERING_PRESENCE_KAZRA_JIN, 1000);
                            break;
                        case NPC_FROST_KING_MALAKK:
                        events.ScheduleEvent(EVENT_LINGERING_PRESENCE_MALAKK, 1000);
                            break;
                        case NPC_HIGH_PRIESTRESS_MAR_LI:
                        events.ScheduleEvent(EVENT_LINGERING_PRESENCE_HIGH_PRIESTRESS, 1000);
                            break;
                        case NPC_SUL_THE_SANDCRAWLER:
                        events.ScheduleEvent(EVENT_LINGERING_PRESENCE_SUL_THE_SANDCRAWLER, 1000);
                            break;
                        default:
                            break;
                    }

                    possessed[i] = entries[i];

                    bool full = true;
                    uint8 u = 0;

                    while (full && u < 4)
                    {
                        if (possessed[u] == 0)
                            full = false;
                        u++;
                    }

                    if (full)
                        for (uint32 entry : possessed)
                            entry = 0;

                }

                else if (action == ACTION_SOUL_FRAGMENT)
                    events.ScheduleEvent(EVENT_SOUL_FRAGMENT, 1000);
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_LINGERING_PRESENCE_MALAKK:
                        if (Creature* malakk = pInstance->instance->GetCreature(pInstance->GetData64(NPC_FROST_KING_MALAKK)))
                        {
                            if (isAlonePossessed(malakk->GetInstanceScript()))
                                me->AddAura(SPELL_POSSESSED, malakk);

                            if (malakk->GetAI())
                                malakk->AI()->DoAction(ACTION_SCHEDULE_FROSTBITE);
                        }
                        break;
                    case EVENT_LINGERING_PRESENCE_KAZRA_JIN:
                        if (Creature* kazraJin = pInstance->instance->GetCreature(pInstance->GetData64(NPC_KAZRA_JIN)))
                        {
                            if (isAlonePossessed(kazraJin->GetInstanceScript()))
                                me->AddAura(SPELL_POSSESSED, kazraJin);
                        }
                        break;
                    case EVENT_LINGERING_PRESENCE_HIGH_PRIESTRESS:
                        if (Creature* priestress = pInstance->instance->GetCreature(pInstance->GetData64(NPC_HIGH_PRIESTRESS_MAR_LI)))
                        {
                            if (isAlonePossessed(priestress->GetInstanceScript()))
                                me->AddAura(SPELL_POSSESSED, priestress);

                            if (priestress->GetAI())
                            {
                                priestress->AI()->DoAction(ACTION_SCHEDULE_WRATH_OF_THE_LOA_SHADOW);
                                priestress->AI()->DoAction(ACTION_SCHEDULE_SHADOWED_SPIRIT_SPAWN);
                            }
                        }
                        break;
                    case EVENT_LINGERING_PRESENCE_SUL_THE_SANDCRAWLER:
                        if (Creature* sul = pInstance->instance->GetCreature(pInstance->GetData64(NPC_SUL_THE_SANDCRAWLER)))
                        {
                            if (isAlonePossessed(sul->GetInstanceScript()))
                                me->AddAura(SPELL_POSSESSED, sul);

                            if (sul->GetAI())
                                sul->AI()->DoAction(ACTION_SCHEDULE_SANDSTROM);
                        }
                        break;
                    case EVENT_SOUL_FRAGMENT:
                    {
                        std::list<Player*> playerList;
                        GetPlayerListInGrid(playerList, me, 200.0f);

                        if (!playerList.empty())
                        {
                            JadeCore::RandomResizeList(playerList, 1);

                            targetFaction = playerList.front()->getFaction();
                            playerList.front()->setFaction(35);
                            playerList.front()->CastSpell(playerList.front(), SPELL_SOUL_FRAGMENT, false);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gara_jal_s_soulAI(creature);
        }
};

// Frost King Malakk - 69131
class boss_king_malakk : public CreatureScript
{
    public:
        boss_king_malakk() : CreatureScript("boss_king_malakk") { }

        struct boss_king_malakkAI : public BossAI
        {
            boss_king_malakkAI(Creature* creature) : BossAI(creature, DATA_CONCIL_OF_ELDERS)
            {
                pInstance = creature->GetInstanceScript();
            }

            EventMap events;
            InstanceScript* pInstance;
            bool firstPossessSwitched;
            bool secondPossessSwitched;
            uint32 coefficient;
            uint32 counter;
            uint32 targetFaction;

            void Reset()
            {
                if (pInstance)
                {
                    if (pInstance->GetBossState(FAIL))
                        pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, NOT_STARTED);

                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                }

                coefficient   = 0;
                counter       = 0;
                targetFaction = 0;
                firstPossessSwitched = false;
                secondPossessSwitched = false;
                _Reset();
                me->GetMotionMaster()->MoveTargetedHome();
                me->ReenableEvadeMode();
                me->SetPower(POWER_ENERGY, 0, false);
            }

            void EnterEvadeMode()
            {
                if (pInstance)
                {
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, FAIL);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                me->SetFullHealth();
                me->RemoveAllAuras();
                _EnterEvadeMode();
                me->GetMotionMaster()->MoveTargetedHome();

                firstPossessSwitched = false;
                secondPossessSwitched = false;
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }

            void EnterCombat(Unit* who)
            {
                StartFight(pInstance, me, who);

                events.Reset();
                events.ScheduleEvent(EVENT_FRIGID_ASSAULT, 30000);
                events.ScheduleEvent(EVENT_BITING_COLD, 62000);
            }

            void JustDied(Unit* killer)
            {
                summons.DespawnAll();
                if (pInstance)
                {
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_SUL_THE_SANDCRAWLER, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_HIGH_PRIESTRESS_MAR_LI, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_KAZRA_JIN, 200.0f))
                    if (creature->isAlive())
                        return;

                if (pInstance)
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, DONE);
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_SCHEDULE_FRIGID_ASSAULT)
                    events.ScheduleEvent(EVENT_FRIGID_ASSAULT, 40000);

                else if (action == ACTION_SCHEDULE_FROSTBITE)
                {
                    events.Reset();
                    events.ScheduleEvent(EVENT_FROSTBITE, 2000);
                }
            }

            void DamageTaken(Unit* /*killer*/, uint32 &damage)
            {
                if (!firstPossessSwitched)
                {
                    if (me->HealthBelowPctDamaged(67.0f, damage))
                    {
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            events.Reset();
                            events.ScheduleEvent(EVENT_FRIGID_ASSAULT, 30000);
                            events.ScheduleEvent(EVENT_BITING_COLD, 62000);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }

                            firstPossessSwitched = true;
                        }
                    }
                }

                if (!secondPossessSwitched)
                {
                    if (me->HealthBelowPctDamaged(25.0f, damage))
                    {
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            events.Reset();
                            events.ScheduleEvent(EVENT_FRIGID_ASSAULT, 30000);
                            events.ScheduleEvent(EVENT_BITING_COLD, 62000);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }

                            secondPossessSwitched = true;
                        }
                    }
                }
            }

            void RegeneratePower(Powers power, int32& value)
            {
                if (power != POWER_ENERGY)
                    return;

                if (me->GetPower(POWER_ENERGY) > 100)
                {
                    me->SetPower(POWER_ENERGY, 100, false);
                    return;
                }

                value = 0;

                if (me->HasAura(SPELL_POSSESSED))
                    value = 2; // Generates 1 point per minute

                if (me->HasAura(SPELL_LINGERING_PRESENCE))
                    value = 2 + coefficient;
            }

            uint32 GetData(uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    return coefficient;
                else if (index == DATA_SPELL_DMG_MULTIPLIER)
                    return counter;

                return 0;
            }

            void SetData(uint32 value, uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    value = coefficient;
            }

            void UpdateAI(const uint32 diff)
            {
                if (pInstance)
                {
                    if (pInstance->IsWipe())
                    {
                        EnterEvadeMode();
                        return;
                    }
                }

                if (!UpdateVictim())
                {
                    if (me->isInCombat())
                        me->CombatStop();
                    EnterEvadeMode();
                    return;
                }

                if (me->GetPower(POWER_ENERGY) == 100)
                {
                    events.Reset();

                    events.ScheduleEvent(EVENT_DARK_POWER, 1000);
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_FRIGID_ASSAULT:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            if (!target->HasAura(SPELL_FRIGID_ASSAULT_STUN))
                                me->CastSpell(target, SPELL_FRIGID_ASSAULT, true);
                        break;
                    case EVENT_BITING_COLD:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            me->CastSpell(target, SPELL_BITING_COLD, true);
                        events.ScheduleEvent(EVENT_BITING_COLD, 62000);
                        break;
                    case EVENT_FROSTBITE:
                        if (me->HasAura(SPELL_POSSESSED))
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            {
                                me->AddAura(SPELL_FROSTBITE_AURA, target);
                                me->CastSpell(target, SPELL_FROSTBITE, true);
                                events.ScheduleEvent(EVENT_FROSTBITE, 52000);
                            }
                        break;
                    case EVENT_DARK_POWER:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            counter++;
                            me->CastSpell(me, SPELL_DARK_POWER, true);
                            events.ScheduleEvent(EVENT_DARK_POWER, 10000);
                        }
                        break;
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_king_malakkAI(creature);
        }
};

// Kazra'jin - 69134
class boss_kazra_jin : public CreatureScript
{
    public:
        boss_kazra_jin() : CreatureScript("boss_kazra_jin") { }

        struct boss_kazra_jinAI : public BossAI
        {
            boss_kazra_jinAI(Creature* creature) : BossAI(creature, DATA_CONCIL_OF_ELDERS)
            {
                pInstance = creature->GetInstanceScript();
            }

            EventMap events;
            InstanceScript* pInstance;
            uint64 playerGuid;
            bool touchedTarget;
            bool firstPossessSwitched;
            bool secondPossessSwitched;
            bool hasSpawned;
            uint32 coefficient;
            uint32 counter;
            uint64 recklessTargetGuid;

            void Reset()
            {
                events.Reset();

                if (pInstance)
                {
                    if (pInstance->GetBossState(FAIL))
                        pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, NOT_STARTED);

                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                }

                touchedTarget         = true;
                firstPossessSwitched  = false;
                secondPossessSwitched = false;
                hasSpawned            = false;

                coefficient        = 0;
                counter            = 0;
                playerGuid         = 0;
                recklessTargetGuid = 0;
                _Reset();
                me->GetMotionMaster()->MoveTargetedHome();
                me->ReenableEvadeMode();
                me->SetPower(POWER_ENERGY, 0, false);
                me->SetReactState(REACT_AGGRESSIVE);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_SCHEDULE_AT_DMG)
                    events.ScheduleEvent(EVENT_RECKLESS_CHARGE_AREATRIGGER_DMG, 1000);
            }

            void JustDied(Unit* killer)
            {
                summons.DespawnAll();
                if (pInstance)
                {
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_SUL_THE_SANDCRAWLER, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_HIGH_PRIESTRESS_MAR_LI, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_FROST_KING_MALAKK, 200.0f))
                    if (creature->isAlive())
                        return;

                if (pInstance)
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, DONE);
            }

            void EnterEvadeMode()
            {
                if (pInstance)
                {
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, FAIL);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                me->SetFullHealth();
                me->RemoveAllAuras();
                _EnterEvadeMode();
                me->GetMotionMaster()->MoveTargetedHome();

                firstPossessSwitched  = false;
                secondPossessSwitched = false;
                hasSpawned            = false;
                touchedTarget         = true;
            }

            void EnterCombat(Unit* who)
            {
                StartFight(pInstance, me, who);
                events.ScheduleEvent(EVENT_RECKLESS_CHARGE, 5000);
            }

            void SetGUID(uint64 guid, int32 id)
            {
                if (id == 1)
                    playerGuid = guid;
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                if (!firstPossessSwitched)
                {
                    if (me->HasAura(SPELL_POSSESSED))
                    {
                        if (me->HealthBelowPctDamaged(70.0f, damage))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }

                            firstPossessSwitched = true;
                        }
                    }
                }

                if (!secondPossessSwitched)
                {
                    if (me->HealthBelowPctDamaged(25.0f, damage))
                    {
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }

                            secondPossessSwitched = true;
                        }
                    }
                }

                if (me->HasAura(SPELL_OVERLOAD))
                {
                    me->DealDamage(attacker, damage / 100 * 50);
                    me->SendSpellNonMeleeDamageLog(attacker, SPELL_OVERLOAD, damage / 100 * 40, SPELL_SCHOOL_MASK_NATURE, 0, 0, false, 0, false);
                }
                else if (me->HasAura(SPELL_DISCHARGE))
                {
                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, me, 200.0f);

                    for (auto player : playerList)
                    {
                        me->DealDamage(player, damage / 100 * 10);
                        me->SendSpellNonMeleeDamageLog(player, SPELL_DISCHARGE, damage / 100 * 10, SPELL_SCHOOL_MASK_NATURE, 0, 0, false, 0, false);
                    }
                }
            }

            void RegeneratePower(Powers power, int32& value)
            {
                if (power != POWER_ENERGY)
                    return;

                if (me->GetPower(POWER_ENERGY) > 100)
                {
                    me->SetPower(POWER_ENERGY, 100, false);
                    return;
                }

                value = 0;

                if (me->HasAura(SPELL_POSSESSED))
                    value = 2; // Generates 1 point per minute

                if (me->HasAura(SPELL_LINGERING_PRESENCE))
                    value = 2 + coefficient;
            }

            uint32 GetData(uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    return coefficient;
                else if (index == DATA_SPELL_DMG_MULTIPLIER)
                    return counter;

                return 0;
            }

            void SetData(uint32 value, uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    value = coefficient;
            }

            void UpdateAI(const uint32 diff)
            {
                if (pInstance)
                {
                    if (pInstance->IsWipe())
                    {
                        EnterEvadeMode();
                        return;
                    }
                }

                if (!me->HasAura(SPELL_RECKLESS_CHARGE_ROLLING))
                {
                    if (!UpdateVictim())
                    {
                        if (me->isInCombat())
                            me->CombatStop();
                        EnterEvadeMode();
                        return;
                    }
                }

                if (me->GetPower(POWER_ENERGY) == 100)
                {
                    events.Reset();

                    events.ScheduleEvent(EVENT_DARK_POWER, 1000);
                }

                if (me->HasAura(SPELL_OVERLOAD))
                    me->SendMeleeAttackStop(me->getVictim());

                if (me->HasAura(SPELL_RECKLESS_CHARGE_ROLLING))
                {
                    if (!touchedTarget)
                    {
                        if (!hasSpawned)
                        {
                            events.ScheduleEvent(EVENT_RECKLESS_CHARGE_AREATRIGGER, 400);
                            hasSpawned = true;
                        }

                        if (Player* target = Player::GetPlayer(*me, playerGuid))
                        {
                            if (me->GetDistance(target) <= 3.5f)
                            {
                                if (target->GetGUID() == playerGuid)
                                {
                                    events.CancelEvent(EVENT_RECKLESS_CHARGE_AREATRIGGER);
                                    events.CancelEvent(EVENT_RECKLESS_CHARGE_AREATRIGGER_DMG);
                                    me->CastSpell(target, SPELL_RECKLESS_CHARGE_KNOCK_BACK, true);
                                    me->canStartAttack(target, true);
                                    me->RemoveAura(SPELL_RECKLESS_CHARGE_ROLLING);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    me->Attack(target, true);
                                    playerGuid = 0;
                                    touchedTarget = true;
                                    hasSpawned = false;
                                    me->SetReactState(REACT_AGGRESSIVE);

                                    std::list<Creature*> creatureList;
                                    GetCreatureListWithEntryInGrid(creatureList, me, 69453, 200.0f);

                                    for (Creature* creature : creatureList)
                                        creature->DespawnOrUnsummon();

                                    if (me->HasAura(SPELL_POSSESSED))
                                    {
                                        if (!IsHeroic())
                                            me->AddAura(SPELL_OVERLOAD, me);
                                        else
                                            me->AddAura(SPELL_DISCHARGE, me);
                                    }
                                }
                            }
                        }
                    }
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_RECKLESS_CHARGE:
                    {
                        std::list<Player*> playerList;
                        GetPlayerListInGrid(playerList, me, 200.0f);

                        std::list<Player*>::iterator itr = playerList.begin();
                        Player* target = NULL;

                        if (!playerList.empty())
                        {
                            while (!target)
                            {
                                if (urand(0, 1))
                                    target = *itr;

                                ++itr;

                                if (itr == playerList.end())
                                    itr = playerList.begin();
                            }
                        }

                        if (target)
                            recklessTargetGuid = target->GetGUID();

                        me->SetReactState(REACT_PASSIVE);
                        me->CastSpell(me, SPELL_RECKLESS_CHARGE_ROLLING, true);
                        touchedTarget = false;
                        events.ScheduleEvent(EVENT_RECKLESS_CHARGE, 10000);
                    }
                        break;
                    case EVENT_RECKLESS_CHARGE_AREATRIGGER:
                            me->CastSpell(me, SPELL_RECKLESS_CHARGE_AREATRIGGER, false);
                            me->SummonCreature(NPC_RECKLESS_CHARGE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                            hasSpawned = false;
                        break;
                    case EVENT_RECKLESS_CHARGE_AREATRIGGER_DMG:
                    {
                        std::list<Creature*> creatureList;
                        GetCreatureListWithEntryInGrid(creatureList, me, 69453, 200.0f);

                        for (Creature* creature : creatureList)
                        {
                            std::list<Player*> playerList;
                            GetPlayerListInGrid(playerList, creature, 1.0f);

                            for (Player* player : playerList)
                            {
                                creature->CastSpell(player, SPELL_RECKLESS_CHARGE_DAMAGE, false);
                            }
                        }

                        events.ScheduleEvent(EVENT_RECKLESS_CHARGE_AREATRIGGER_DMG, 1000);
                        break;
                    }
                    case EVENT_DARK_POWER:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            counter++;
                            me->CastSpell(me, SPELL_DARK_POWER, true);
                            events.ScheduleEvent(EVENT_DARK_POWER, 10000);
                        }
                        break;
                    default:
                        break;
                }

                if (!me->HasAura(SPELL_RECKLESS_CHARGE_ROLLING))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_kazra_jinAI(creature);
        }
};

// Sul the Sandcrawler - 69078
class boss_sul_the_sandcrawler : public CreatureScript
{
    public:
        boss_sul_the_sandcrawler() : CreatureScript("boss_sul_the_sandcrawler") { }

        struct boss_sul_the_sandcrawlerAI : public BossAI
        {
            boss_sul_the_sandcrawlerAI(Creature* creature) : BossAI(creature, DATA_CONCIL_OF_ELDERS)
            {
                pInstance = creature->GetInstanceScript();
            }

            EventMap events;
            InstanceScript* pInstance;
            bool firstPossessSwitched;
            bool secondPossessSwitched;
            uint32 coefficient;
            uint32 counter;

            void Reset()
            {
                if (pInstance)
                {
                    if (pInstance->GetBossState(FAIL))
                        pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, NOT_STARTED);

                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                }

                events.Reset();

                firstPossessSwitched = false;
                secondPossessSwitched = false;

                coefficient = 0;
                counter     = 0;
                _Reset();
                me->GetMotionMaster()->MoveTargetedHome();
                me->ReenableEvadeMode();
                me->SetPower(POWER_ENERGY, 0, false);

                std::list<Creature*> livingSandList;
                GetCreatureListWithEntryInGrid(livingSandList, me, NPC_LIVING_SAND, 200.0f);

                std::list<Creature*> firstTwistedFateList;
                GetCreatureListWithEntryInGrid(firstTwistedFateList, me, NPC_FIRST_TWISTED_FATE, 200.0f);

                std::list<Creature*> secondTwistedFateList;
                GetCreatureListWithEntryInGrid(secondTwistedFateList, me, NPC_SECOND_TWISTED_FATE, 200.0f);

                for (auto creature : livingSandList)
                    creature->DespawnOrUnsummon();

                for (auto creature : firstTwistedFateList)
                    creature->DespawnOrUnsummon();

                for (auto creature : secondTwistedFateList)
                    creature->DespawnOrUnsummon();

                if (pInstance)
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }

            void EnterCombat(Unit* who)
            {
                StartFight(pInstance, me, who);

                events.ScheduleEvent(EVENT_SAND_BOLT, 10000);
                events.ScheduleEvent(EVENT_QUICKSAND, 35000);
            }

            void JustDied(Unit* killer)
            {
                summons.DespawnAll();
                if (pInstance)
                {
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_KAZRA_JIN, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_HIGH_PRIESTRESS_MAR_LI, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_FROST_KING_MALAKK, 200.0f))
                    if (creature->isAlive())
                        return;

                if (pInstance)
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, DONE);
            }

            void EnterEvadeMode()
            {
                if (pInstance)
                {
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, FAIL);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                me->SetFullHealth();
                me->RemoveAllAuras();
                events.Reset();
                summons.DespawnAll();
                _EnterEvadeMode();
                me->GetMotionMaster()->MoveTargetedHome();

                firstPossessSwitched = false;
                secondPossessSwitched = false;
            }

            void DamageTaken(Unit* /*killer*/, uint32 &damage)
            {
                if (!firstPossessSwitched)
                {
                    if (me->HasAura(SPELL_POSSESSED))
                    {
                        if (me->HealthBelowPctDamaged(70.0f, damage))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }

                            firstPossessSwitched = true;
                        }
                    }
                }

                if (!secondPossessSwitched)
                {
                    if (me->HealthBelowPctDamaged(25.0f, damage))
                    {
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }

                            secondPossessSwitched = true;
                        }
                    }
                }
            }

            void RegeneratePower(Powers power, int32& value)
            {
                if (power != POWER_ENERGY)
                    return;

                if (me->GetPower(POWER_ENERGY) > 100)
                {
                    me->SetPower(POWER_ENERGY, 100, false);
                    return;
                }

                value = 0;

                if (me->HasAura(SPELL_POSSESSED))
                    value = 2; // Generates 1 point per minute

                if (me->HasAura(SPELL_LINGERING_PRESENCE))
                    value = 2 + coefficient;
            }

            uint32 GetData(uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    return coefficient;
                else if (index == DATA_SPELL_DMG_MULTIPLIER)
                    return counter;

                return 0;
            }

            void SetData(uint32 value, uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    value = coefficient;
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_SCHEDULE_SANDSTROM)
                    events.ScheduleEvent(EVENT_SANDSTORM, 5000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (pInstance)
                {
                    if (pInstance->IsWipe())
                    {
                        EnterEvadeMode();
                        return;
                    }
                }

                if (!UpdateVictim())
                {
                    if (me->isInCombat())
                        me->CombatStop();
                    EnterEvadeMode();
                    return;
                }

                if (me->GetPower(POWER_ENERGY) == 100)
                {
                    events.Reset();

                    events.ScheduleEvent(EVENT_DARK_POWER, 1000);
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_SAND_BOLT:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            me->CastSpell(target, SPELL_SAND_BOLT, true);
                        events.ScheduleEvent(EVENT_SAND_BOLT, 10000);
                        break;
                    case EVENT_QUICKSAND:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            me->CastSpell(target, SPELL_QUICKSAND, true);
                            me->SummonCreature(NPC_LIVING_SAND, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                        }
                        events.ScheduleEvent(EVENT_QUICKSAND, 34000);
                        break;
                    case EVENT_SANDSTORM:
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                me->CastSpell(target, SPELL_SANDSTORM, true);

                            me->AddAura(SPELL_SANDSTORM_DUMMY, me);
                            events.ScheduleEvent(EVENT_SANDSTORM, 40000);
                        }
                        break;
                    case EVENT_DARK_POWER:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            counter++;
                            me->CastSpell(me, SPELL_DARK_POWER, true);
                            events.ScheduleEvent(EVENT_DARK_POWER, 10000);
                        }
                        break;
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_sul_the_sandcrawlerAI(creature);
        }
};

// Twisted Fate (first) - 69740
class mob_first_twisted_fate : public CreatureScript
{
    public:
        mob_first_twisted_fate() : CreatureScript("mob_first_twisted_fate") { }

        struct mob_first_twisted_fateAI : public ScriptedAI
        {
            mob_first_twisted_fateAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            bool visualLinked;
            uint16 linkTimer;
            uint16 distanceMax;
            uint64 summonerGuid;
            InstanceScript* instance;

            void Reset()
            {
                distanceMax  = 0;
                linkTimer    = 0;
                summonerGuid = 0;
                visualLinked = false;
                me->SetReactState(REACT_PASSIVE);
                me->AddAura(SPELL_SHADOW_VISUAL , me);
            }

            void IsSummonedBy(Unit* summoner)
            {
                summonerGuid = summoner->GetGUID();
                me->AddAura(SPELL_TWISTED_FATE_PERIODIC, me);
                linkTimer = 1500;
            }

            void UpdateAI(uint32 const diff)
            {
                if (linkTimer <= diff)
                {
                    if (!visualLinked)
                    {
                        if (Creature* twistedFate = GetClosestCreatureWithEntry(me, NPC_SECOND_TWISTED_FATE, 200.0f))
                        {
                            me->GetMotionMaster()->MoveChase(twistedFate, 1.0f, 1.0f);
                            visualLinked = true;
                        }
                    }
                }
                else
                   linkTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_first_twisted_fateAI(creature);
        }
};

// Twisted Fate (second) - 69746
class mob_second_twisted_fate : public CreatureScript
{
    public:
        mob_second_twisted_fate() : CreatureScript("mob_second_twisted_fate") { }

        struct mob_second_twisted_fateAI : public ScriptedAI
        {
            mob_second_twisted_fateAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            bool visualLinked;
            uint64 summonerGuid;
            uint16 linkTimer;
            uint16 distanceMax;
            InstanceScript* instance;

            void Reset()
            {
                linkTimer      = 0;
                summonerGuid   = 0;
                distanceMax    = 0;
                visualLinked = false;
                me->SetReactState(REACT_PASSIVE);
                me->AddAura(SPELL_SHADOW_VISUAL , me);
            }

            void IsSummonedBy(Unit* summoner)
            {
                summonerGuid = summoner->GetGUID();
                me->AddAura(SPELL_TWISTED_FATE_PERIODIC, me);
                linkTimer = 1500;
            }

            void UpdateAI(const uint32 diff)
            {
                if (linkTimer <= diff)
                {
                    if (!visualLinked)
                    {
                        if (Creature* twistedFate = GetClosestCreatureWithEntry(me, NPC_FIRST_TWISTED_FATE, 200.0f))
                        {
                            me->CastSpell(twistedFate, SPELL_TWISTED_FATE_LINK_VISUAL, false);
                            me->GetMotionMaster()->MoveChase(twistedFate, 1.0f, 1.0f);
                            visualLinked = true;
                        }
                    }
                }
                else
                   linkTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_second_twisted_fateAI(creature);
        }
};

// High Priestess Mar'li - 69132
class boss_high_priestress_mar_li : public CreatureScript
{
    public:
        boss_high_priestress_mar_li() : CreatureScript("boss_high_priestress_mar_li") { }

        struct boss_high_priestress_mar_liAI : public BossAI
        {
            boss_high_priestress_mar_liAI(Creature* creature) : BossAI(creature, DATA_CONCIL_OF_ELDERS)
            {
                pInstance = creature->GetInstanceScript();
            }

            EventMap events;
            InstanceScript* pInstance;
            bool firstPossessSwitched;
            bool secondPossessSwitched;
            uint32 coefficient;
            uint32 counter;

            void Reset()
            {
                if (pInstance)
                {
                    if (pInstance->GetBossState(FAIL))
                        pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, NOT_STARTED);

                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                }

                events.Reset();

                firstPossessSwitched  = false;
                secondPossessSwitched = false;

                coefficient = 0;
                counter     = 0;
                _Reset();
                me->GetMotionMaster()->MoveTargetedHome();
                me->ReenableEvadeMode();
                me->SetPower(POWER_ENERGY, 0, false);

                if (pInstance)
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_SCHEDULE_WRATH_OF_THE_LOA_SHADOW || action == ACTION_SCHEDULE_SHADOWED_SPIRIT_SPAWN)
                {
                    if (IsHeroic())
                    {
                        events.Reset();

                        events.ScheduleEvent(EVENT_TWISTED_FATE, 2000);
                        events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_SHADOW, 3000);
                        events.ScheduleEvent(EVENT_TWISTED_FATE_SECOND, 5000);
                        return;
                    }

                    events.Reset();
                    events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_SHADOW, 3000);
                    events.ScheduleEvent(EVENT_SHADOWED_LOA_SPIRIT_SUMMON, 5000);
                }
            }

            void EnterEvadeMode()
            {
                if (pInstance)
                {
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, FAIL);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                me->SetFullHealth();
                me->RemoveAllAuras();
                _EnterEvadeMode();
                me->GetMotionMaster()->MoveTargetedHome();

                std::list<Creature*> livingSandList;
                GetCreatureListWithEntryInGrid(livingSandList, me, NPC_LIVING_SAND, 200.0f);

                for (Creature* livingSand : livingSandList)
                    livingSand->DespawnOrUnsummon();

                firstPossessSwitched = false;
                secondPossessSwitched = false;
            }

            void EnterCombat(Unit* who)
            {
                StartFight(pInstance, me, who);

                events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_BLESSED, 10000);
                events.ScheduleEvent(EVENT_BLESSED_LOA_SPIRIT_SUMMON, 10000);
            }

            void DamageTaken(Unit* /*killer*/, uint32 &damage)
            {
                if (!firstPossessSwitched)
                {
                    if (me->HasAura(SPELL_POSSESSED))
                    {
                        if (me->HealthBelowPctDamaged(70.0f, damage))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            events.Reset();
                            events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_BLESSED, 10000);
                            events.ScheduleEvent(EVENT_BLESSED_LOA_SPIRIT_SUMMON, 10000);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }


                            firstPossessSwitched = true;
                        }
                    }
                }

                if (!secondPossessSwitched)
                {
                    if (me->HealthBelowPctDamaged(25.0f, damage))
                    {
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            me->SetPower(POWER_ENERGY, 0, false);
                            me->RemoveAura(SPELL_POSSESSED);
                            me->CastSpell(me, SPELL_LINGERING_PRESENCE, true);

                            events.Reset();
                            events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_BLESSED, 10000);
                            events.ScheduleEvent(EVENT_BLESSED_LOA_SPIRIT_SUMMON, 10000);

                            if (Creature* garaJalSoul = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                                if (garaJalSoul->GetAI())
                                {
                                    garaJalSoul->AI()->DoAction(ACTION_SCHEDULE_POSSESSION);
                                    garaJalSoul->AI()->DoAction(ACTION_SOUL_FRAGMENT);
                                }


                            secondPossessSwitched = true;
                        }
                    }
                }
            }

            void JustDied(Unit* killer)
            {
                summons.DespawnAll();
                if (pInstance)
                {
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENERIC_STUN);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                }

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_KAZRA_JIN, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_SUL_THE_SANDCRAWLER, 200.0f))
                    if (creature->isAlive())
                        return;

                if (Creature* creature = GetClosestCreatureWithEntry(me, NPC_FROST_KING_MALAKK, 200.0f))
                    if (creature->isAlive())
                        return;

                if (pInstance)
                    pInstance->SetBossState(DATA_CONCIL_OF_ELDERS, DONE);
            }

            void RegeneratePower(Powers power, int32& value)
            {
                if (power != POWER_ENERGY)
                    return;

                if (me->GetPower(POWER_ENERGY) > 100)
                {
                    me->SetPower(POWER_ENERGY, 100, false);
                    return;
                }

                value = 0;

                if (me->HasAura(SPELL_POSSESSED))
                    value = 2; // Generates 1 point per minute

                if (me->HasAura(SPELL_LINGERING_PRESENCE))
                    value = 2 + coefficient;
            }

            uint32 GetData(uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    return coefficient;
                else if (index == DATA_SPELL_DMG_MULTIPLIER)
                    return counter;

                return 0;
            }

            void SetData(uint32 value, uint32 index)
            {
                if (index == DATA_LINGERING_PRESENCE_COEF)
                    value = coefficient;
            }

            void UpdateAI(const uint32 diff)
            {
                if (pInstance)
                {
                    if (pInstance->IsWipe())
                    {
                        EnterEvadeMode();
                        return;
                    }
                }

                if (!UpdateVictim())
                {
                    if (me->isInCombat())
                        me->CombatStop();
                    EnterEvadeMode();
                    return;
                }

                if (me->GetPower(POWER_ENERGY) == 100)
                {
                    events.Reset();

                    events.ScheduleEvent(EVENT_DARK_POWER, 1000);
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_WRATH_OF_THE_LOA_BLESSED:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            me->CastSpell(target, SPELL_WRATH_OF_THE_LOA_BLESSED, true);
                        events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_BLESSED, 10000);
                        break;
                    case EVENT_WRATH_OF_THE_LOA_SHADOW:
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                me->CastSpell(target, SPELL_WRATH_OF_THE_LOA_SHADOW, true);
                            events.ScheduleEvent(EVENT_WRATH_OF_THE_LOA_SHADOW, 10000);
                        }
                        break;
                    case EVENT_BLESSED_LOA_SPIRIT_SUMMON:
                        me->CastSpell(me, SPELL_BLESSED_LOA_SPIRIT_SUMMON, false);
                        events.ScheduleEvent(EVENT_BLESSED_LOA_SPIRIT_SUMMON, 35000);
                        break;
                    case EVENT_SHADOWED_LOA_SPIRIT_SUMMON:
                        if (me->HasAura(SPELL_POSSESSED))
                            me->CastSpell(me, SPELL_SHADOWED_LOA_SPIRIT_SUMMONED, false);
                        events.ScheduleEvent(EVENT_SHADOWED_LOA_SPIRIT_SUMMON, 37000);
                        break;
                    case EVENT_DARK_POWER:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            counter++;
                            me->CastSpell(me, SPELL_DARK_POWER, true);
                            events.ScheduleEvent(EVENT_DARK_POWER, 10000);
                        }
                        break;
                    case EVENT_TWISTED_FATE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST))
                            me->CastSpell(me, SPELL_TWISTED_FATE_PRINCIPAL, true);
                        break;
                    case EVENT_TWISTED_FATE_SECOND:
                        if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST))
                        {
                            me->CastSpell(me, SPELL_TWISTED_FATE_SECOND_PRINCIPAL, false);

                            if (Creature* twistedFirst = GetClosestCreatureWithEntry(me, NPC_FIRST_TWISTED_FATE, 200.0f))
                            {
                                if (Creature* twistedSecond = GetClosestCreatureWithEntry(me, NPC_SECOND_TWISTED_FATE, 200.0f))
                                {
                                    float distance = twistedFirst->GetDistance(twistedSecond);

                                    CAST_AI(mob_first_twisted_fate::mob_first_twisted_fateAI, twistedFirst->AI())->distanceMax = distance;
                                    CAST_AI(mob_second_twisted_fate::mob_second_twisted_fateAI, twistedSecond->AI())->distanceMax = distance;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_high_priestress_mar_liAI(creature);
        }
};

// Living Sand - 69153
class mob_living_sand : public CreatureScript
{
    public:
        mob_living_sand() : CreatureScript("mob_living_sand") { }

        struct mob_living_sandAI : public ScriptedAI
        {
            mob_living_sandAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->AddAura(SPELL_SAND_VISUAL, me);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->SetFullHealth();
                events.Reset();

                if (me->HasAura(SPELL_FORTIFIED))
                    me->RemoveAura(SPELL_FORTIFIED);

                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 7.0f);

                for (auto player: playerList)
                {
                    if (!player->HasAura(SPELL_SAND_PERIODIC_DMG))
                        me->AddAura(SPELL_SAND_PERIODIC_DMG, player);

                    player->AddAura(SPELL_ENSNARED, player);
                }
            }

            void IsSummonedBy(Unit* attacker)
            {
                std::list<Creature*> livingSangList;
                GetCreatureListWithEntryInGrid(livingSangList, me, NPC_LIVING_SAND, 4.5f);

                for (Creature* livingSand : livingSangList)
                {
                    if (livingSand->HasAura(SPELL_TREACHEROUS_GROUND_RESIZE))
                    {
                        livingSand->DespawnOrUnsummon();
                        me->AddAura(SPELL_TREACHEROUS_GROUND, me);
                    }
                }

                events.ScheduleEvent(EVENT_ENSNARED, 1000);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_SANDSTORM)
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveAura(SPELL_SAND_VISUAL);
                    me->SetReactState(REACT_AGGRESSIVE);
                }
                else if (action == ACTION_TREACHEROUS_GROUND)
                {
                    std::list<Creature*> livingSangList;
                    GetCreatureListWithEntryInGrid(livingSangList, me, NPC_LIVING_SAND, 4.5f);

                    for (Creature* livingSand : livingSangList)
                    {
                        if (livingSand != me)
                        {
                            livingSand->DespawnOrUnsummon();
                            me->AddAura(SPELL_TREACHEROUS_GROUND_RESIZE, me);
                        }
                    }
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage > me->GetHealth())
                {
                    damage = 0;
                    Reset();
                }
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_ENSNARED:
                        if (me->HasAura(SPELL_SAND_VISUAL))
                        {
                            std::list<Player*> playerList;
                            GetPlayerListInGrid(playerList, me, 7.0f);

                            for (Player* player : playerList)
                                if (!player->HasAura(SPELL_ENTRAPPED))
                                    player->AddAura(SPELL_ENSNARED, player);
                        }

                        events.ScheduleEvent(EVENT_ENSNARED, 1000);
                        break;
                    default:
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_living_sandAI(creature);
        }
};

// Blessed Loa Spirit - 69480
class mob_blessed_loa_spirit : public CreatureScript
{
    public:
        mob_blessed_loa_spirit() : CreatureScript("mob_blessed_loa_spirit") { }

        struct mob_blessed_loa_spiritAI : public ScriptedAI
        {
            mob_blessed_loa_spiritAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            EventMap events;
            InstanceScript* instance;
            uint32 bossEntry;

            void Reset()
            {
                events.Reset();

                events.ScheduleEvent(EVENT_HEAL_WEAKER_TROLL, 20000);
                me->SetReactState(REACT_PASSIVE);
                bossEntry = 0;
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);

                if (bossEntry)
                {
                    if (Creature* boss = instance->instance->GetCreature(instance->GetData64(bossEntry)))
                    {
                        if (me->GetDistance(boss) <= 2.0f)
                        {
                            boss->ModifyHealth(boss->GetHealth() * 5 / 100);
                            me->DespawnOrUnsummon();
                        }
                    }
                }

                switch (events.ExecuteEvent())
                {
                    case EVENT_HEAL_WEAKER_TROLL:
                        {
                            uint32 mobEntries[4] = {NPC_KAZRA_JIN, NPC_SUL_THE_SANDCRAWLER, NPC_HIGH_PRIESTRESS_MAR_LI, NPC_FROST_KING_MALAKK};
                            uint32 minHealth = 0;
                            Creature* minBoss = NULL;

                            for (uint32 entry : mobEntries)
                            {
                                if (Creature* boss = instance->instance->GetCreature(instance->GetData64(entry)))
                                {
                                    if (entry == NPC_KAZRA_JIN)
                                    {
                                        minHealth = boss->GetMaxHealth();
                                        minBoss = boss;
                                    }

                                    if (minHealth > boss->GetHealth() && boss->isAlive())
                                    {
                                        minHealth = boss->GetHealth();
                                        minBoss = boss;
                                    }
                                }
                            }

                            if (minBoss)
                                if (bossEntry != minBoss->GetEntry())
                                    bossEntry = minBoss->GetEntry();

                            me->GetMotionMaster()->MoveChase(minBoss, 0.7f, 0.7f);
                        }
                        break;
                    default:
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_blessed_loa_spiritAI(creature);
        }
};

// Shadowed Loa Spirit - 69548
class mob_shadowed_lua_spirit : public CreatureScript
{
    public:
        mob_shadowed_lua_spirit() : CreatureScript("mob_shadowed_lua_spirit") { }

        struct mob_shadowed_lua_spiritAI : public ScriptedAI
        {
            mob_shadowed_lua_spiritAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            EventMap events;
            uint32 despawnTimer;
            uint64 targetGuid;
            InstanceScript* instance;

            void Reset()
            {
                events.Reset();
                despawnTimer = 0;
                targetGuid   = 0;

                events.ScheduleEvent(EVENT_OS_PLAYER, 20000);
                me->SetReactState(REACT_PASSIVE);
            }

            void SetGUID(uint64 guid, int32 /*index*/)
            {
                targetGuid = guid;
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);
                if (Player* player = Player::GetPlayer(*me, targetGuid))
                {
                    if (despawnTimer <= diff)
                    {

                        if (player->HasAura(SPELL_MARKED_SOUL))
                            player->RemoveAura(SPELL_MARKED_SOUL);

                        me->CastSpell(player, SPELL_SHADOWED_GIFT, false);
                    }
                    else
                        despawnTimer -= diff;

                    if (player->GetDistance(me) <= 6.0f)
                    {
                        me->CastSpell(player, SPELL_SHADOWED_GIFT, false);
                        me->DespawnOrUnsummon();
                    }
                }

                switch (events.ExecuteEvent())
                {
                    case EVENT_OS_PLAYER:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            me->AddAura(SPELL_MARKED_SOUL, target);
                            SetGUID(target->GetGUID(), 0);
                            despawnTimer = 20000;
                        }
                        break;
                    default:
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_shadowed_lua_spiritAI(creature);
    }
};

// Reckless Charge (rolling) - 137117
class spell_reckless_charge_rolling : public SpellScriptLoader
{
    public:
        spell_reckless_charge_rolling() : SpellScriptLoader("spell_reckless_charge_rolling") { }

        class spell_reckless_charge_rolling_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_reckless_charge_rolling_SpellScript);

            void HandleAfterCast()
            {
                if (Creature* caster = GetCaster()->ToCreature())
                {
                    if (caster->GetEntry() == NPC_KAZRA_JIN && caster->GetAI())
                    {
                        uint64 targetGuid = CAST_AI(boss_kazra_jin::boss_kazra_jinAI, caster->AI())->recklessTargetGuid;

                        if (Unit* target = ObjectAccessor::FindUnit(targetGuid))
                        {
                            caster->AttackStop();

                            if (caster->GetDistance(target) >= 3.5f)
                                caster->CastSpell(target, SPELL_RECKLESS_CHARGE_MOVEMENT, false);

                            if (Player* player = target->ToPlayer())
                            {
                                caster->AI()->SetGUID(player->GetGUID(), 1);
                                caster->AI()->DoAction(ACTION_SCHEDULE_AT_DMG);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_reckless_charge_rolling_SpellScript::HandleAfterCast);
            }
        };

        class spell_reckless_charge_rolling_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_reckless_charge_rolling_AuraScript);

            void OnRemove(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Creature* caster = GetCaster()->ToCreature())
                    if (caster->GetEntry() == NPC_KAZRA_JIN && caster->GetAI())
                        caster->SetReactState(REACT_AGGRESSIVE);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_reckless_charge_rolling_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_reckless_charge_rolling_AuraScript();
        }

        SpellScript* GetSpellScript() const
        {
            return new spell_reckless_charge_rolling_SpellScript();
        }
};

// Reckless Charge (movement) - 137131
class spell_reckless_charge_movement : public SpellScriptLoader
{
    public:
        spell_reckless_charge_movement() : SpellScriptLoader("spell_reckless_charge_movement") { }

        class spell_reckless_charge_movement_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_reckless_charge_movement_SpellScript);

            void HandleAfterCast()
            {
                if (Creature* caster = GetCaster()->ToCreature())
                {
                    uint64 targetGuid = CAST_AI(boss_kazra_jin::boss_kazra_jinAI, caster->AI())->recklessTargetGuid;

                    if (Unit* target = ObjectAccessor::FindUnit(targetGuid))
                    {
                        if (caster->GetEntry() == NPC_KAZRA_JIN && caster->GetAI())
                        {
                            caster->ClearUnitState(UNIT_STATE_CASTING | UNIT_STATE_STUNNED);

                            if (target)
                                caster->GetMotionMaster()->MovePoint(1, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                        }
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_reckless_charge_movement_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_reckless_charge_movement_SpellScript();
        }
};

// Frigid Assault (Stun) - 136904
class spell_frigid_assault : public SpellScriptLoader
{
    public:
        spell_frigid_assault() : SpellScriptLoader("spell_frigid_assault") { }

        class spell_frigid_assault_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_frigid_assault_AuraScript);

            void OnApply(constAuraEffectPtr aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (AuraPtr frigidAssault = aurEff->GetBase())
                    if (frigidAssault->GetStackAmount() == 15)
                    {
                        if (Unit* caster = GetCaster())
                        {
                            if (Unit* target = GetTarget())
                            {
                                target->RemoveAurasDueToSpell(SPELL_FRIGID_ASSAULT);
                                caster->RemoveAurasDueToSpell(SPELL_FRIGID_ASSAULT);
                                target->CastSpell(target, SPELL_FRIGID_ASSAULT_STUN, true);

                                if (Creature* malakk = caster->ToCreature())
                                    if (malakk->GetAI())
                                        malakk->AI()->DoAction(ACTION_SCHEDULE_FRIGID_ASSAULT);
                            }
                        }
                    }
            }

            void OnRemove(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Creature* malakk = caster->ToCreature())
                        if (malakk->GetAI())
                            malakk->AI()->DoAction(ACTION_SCHEDULE_FRIGID_ASSAULT);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_frigid_assault_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_frigid_assault_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_frigid_assault_AuraScript();
        }
};

// Lingering Presence - 136467
class spell_lingering_presence : public SpellScriptLoader
{
    public:
        spell_lingering_presence() : SpellScriptLoader("spell_lingering_presence") { }

        class spell_lingering_presence_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_lingering_presence_AuraScript);

            void OnApply(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                uint32 coefficient = 2 * 10 / 100;

                if (Unit* target = GetTarget())
                {
                    if (Creature* creature = target->ToCreature())
                    {
                        if (creature->GetAI())
                        {
                            creature->SetPower(POWER_ENERGY, 0, true);
                            creature->AI()->SetData(coefficient, DATA_LINGERING_PRESENCE_COEF);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_lingering_presence_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_lingering_presence_AuraScript();
        }
};

// Biting Cold - 136917
class spell_biting_cold_malakk : public SpellScriptLoader
{
    public:
        spell_biting_cold_malakk() : SpellScriptLoader("spell_biting_cold_malakk") { }

        class spell_biting_cold_malakk_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_biting_cold_malakk_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* target = GetExplTargetUnit())
                    if (Creature* caster = GetCaster()->ToCreature())
                        if (caster->GetEntry() == NPC_FROST_KING_MALAKK && caster->GetAI())
                            caster->AddAura(SPELL_BITING_COLD_AURA, target); 
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_biting_cold_malakk_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_biting_cold_malakk_SpellScript();
        }
};

// Frostbite - 136990
class spell_frosbite_malakk : public SpellScriptLoader
{
    public:
        spell_frosbite_malakk() : SpellScriptLoader("spell_frosbite_malakk") { }

        class spell_frosbite_malakk_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_frosbite_malakk_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* target = GetExplTargetUnit())
                    if (Creature* caster = GetCaster()->ToCreature())
                        if (caster->GetEntry() == NPC_FROST_KING_MALAKK && caster->GetAI())
                            caster->CastSpell(target, SPELL_FROSTBITE_PERIODIC, true);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_frosbite_malakk_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_frosbite_malakk_SpellScript();
        }
};

// Frostbite (aura) - 136922
class spell_frosbite_malakk_aura : public SpellScriptLoader
{
    public:
        spell_frosbite_malakk_aura() : SpellScriptLoader("spell_frosbite_malakk_aura") { }

        class spell_frosbite_malakk_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_frosbite_malakk_aura_AuraScript);

            InstanceScript* pInstance;

            void OnApply(constAuraEffectPtr aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (AuraPtr frostBite = aurEff->GetBase())
                {
                    if (uint8 stack = frostBite->GetStackAmount())
                    {
                        if (Unit* target = GetTarget())
                        {
                            std::list<Player*> playerList;
                            target->GetPlayerListInGrid(playerList, 4.0f);
                            uint8 size = playerList.size();

                            if (pInstance = target->GetInstanceScript())
                            {
                                if (!pInstance->instance->IsHeroic())
                                {
                                    uint8 amount = std::max(1, (stack - (2 * size)));
                                    return;
                                }

                                for (auto player : playerList)
                                {
                                    if (player->HasAura(SPELL_BODY_HEAT))
                                        uint8 amount = std::max(1, (stack - (2 * size)));
                                }
                            }
                        }
                    }
                }
            }

            void OnTick(constAuraEffectPtr /*aurEff*/)
            {
                if (Unit* target = GetTarget())
                {
                    std::list<Player*> playerList;
                    target->GetPlayerListInGrid(playerList, 4.0f);

                    if (pInstance = target->GetInstanceScript())
                    {
                        if (pInstance->instance->IsHeroic())
                        {
                            for (auto player : playerList)
                            {
                                if (!player->HasAura(SPELL_FROSTBITE_AURA) && !player->HasAura(SPELL_BODY_HEAT) && !player->HasAura(SPELL_CHILLED_TO_THE_BONE))
                                    player->AddAura(SPELL_BODY_HEAT, player);
                            }
                        }
                    }
                }
            }

            void OnRemove(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetTarget())
                    if (!target->HasAura(SPELL_BODY_HEAT))
                        target->AddAura(SPELL_CHILLED_TO_THE_BONE, target);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_frosbite_malakk_aura_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
                OnEffectApply += AuraEffectApplyFn(spell_frosbite_malakk_aura_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_frosbite_malakk_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_frosbite_malakk_aura_AuraScript();
        }
};

// Overload (aura) - 137149 / Discharge - 137166
class spell_overload_discharge_kazra_jin : public SpellScriptLoader
{
    public:
        spell_overload_discharge_kazra_jin() : SpellScriptLoader("spell_overload_discharge_kazra_jin") { }

        class spell_overload_discharge_kazra_jin_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_overload_discharge_kazra_jin_AuraScript);

            void OnApply(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->AddAura(SPELL_GENERIC_STUN, caster);
            }

            void OnRemove(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->RemoveAura(SPELL_GENERIC_STUN);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_overload_discharge_kazra_jin_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_overload_discharge_kazra_jin_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_overload_discharge_kazra_jin_AuraScript();
        }
};

// Ensnared - 136878
class spell_ensnared : public SpellScriptLoader
{
    public:
        spell_ensnared() : SpellScriptLoader("spell_ensnared") { }

        class spell_ensnared_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ensnared_AuraScript);

            void OnApply(constAuraEffectPtr aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (AuraPtr ensnared = aurEff->GetBase())
                {
                    if (uint8 stack = ensnared->GetStackAmount())
                    {
                        if (Unit* caster = GetCaster())
                        {
                            std::list<Player*> playerList;
                            caster->GetPlayerListInGrid(playerList, 7.0f);

                            for (auto player: playerList)
                                if (stack == 5)
                                {
                                    player->RemoveAura(SPELL_ENSNARED);
                                    player->AddAura(SPELL_ENTRAPPED, player);
                                }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_ensnared_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ensnared_AuraScript();
        }
};

// SandStorm - 136895
class spell_sandstorm : public SpellScriptLoader
{
    public:
        spell_sandstorm() : SpellScriptLoader("spell_sandstorm") { }

        class spell_sandstorm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sandstorm_AuraScript);

            void OnApply(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetTarget())
                    {
                        std::list<Creature*> creatureList;
                        GetCreatureListWithEntryInGrid(creatureList, caster, NPC_LIVING_SAND, 200.0f);

                        for (auto livingSand : creatureList)
                        {
                            if (livingSand->GetAI())
                            {
                                if (livingSand->HasAura(SPELL_SAND_VISUAL))
                                    livingSand->AI()->DoAction(ACTION_SANDSTORM);
                                else
                                    livingSand->CastSpell(livingSand, SPELL_FORTIFIED, false);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_sandstorm_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sandstorm_AuraScript();
        }
};

// Dark Power - 136507
class spell_dark_power : public SpellScriptLoader
{
    public:
        spell_dark_power() : SpellScriptLoader("spell_dark_power") { }

        class spell_dark_power_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dark_power_SpellScript);

            void HandleOnHit()
            {
                if (Unit* target = GetExplTargetUnit())
                    if (Creature* caster = GetCaster()->ToCreature())
                        if (caster->GetAI())
                        {
                            uint32 counter = caster->AI()->GetData(DATA_SPELL_DMG_MULTIPLIER);
                            SetHitDamage(GetSpellInfo()->Effects[0].BasePoints * 1 + (counter * 0.10));
                        }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_dark_power_SpellScript ::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dark_power_SpellScript();
        }
};

// Soul Fragment - 137641
class spell_soul_fragment : public SpellScriptLoader
{
    public:
        spell_soul_fragment() : SpellScriptLoader("spell_soul_fragment") { }

        class spell_soul_fragment_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_soul_fragment_AuraScript);

            InstanceScript* instance;

            void OnRemove(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (instance = caster->GetInstanceScript())
                        if (Creature* garaJal = instance->instance->GetCreature(instance->GetData64(NPC_GARA_JAL_SOUL)))
                            if (garaJal->GetAI())
                                caster->setFaction(CAST_AI(npc_gara_jal_s_soul::npc_gara_jal_s_soulAI, garaJal->AI())->targetFaction);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_soul_fragment_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_soul_fragment_AuraScript();
        }
};

// Soul Fragment (switch) - 137643
class spell_soul_fragment_switch : public SpellScriptLoader
{
    public:
        spell_soul_fragment_switch() :  SpellScriptLoader("spell_soul_fragment_switch") { }

        class spell_soul_fragment_switch_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_soul_fragment_switch_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        if (target->ToPlayer())
                        {
                            if (caster->ToPlayer())
                            {
                                caster->RemoveAura(SPELL_SOUL_FRAGMENT);
                                caster->CastSpell(target, SPELL_SOUL_FRAGMENT, false);
                                caster->CastSpell(target, SPELL_SOUL_FRAGMENT_SWITCH, false);
                            }
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_soul_fragment_switch_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_soul_fragment_switch_SpellScript();
        }
};

// Twisted Fate (first)- 137943
class spell_first_twisted_fate : public SpellScriptLoader
{
    public:
        spell_first_twisted_fate() :  SpellScriptLoader("spell_first_twisted_fate") { }

        class spell_first_twisted_fate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_first_twisted_fate_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                uint64 targetGuid = 0;

                if (Unit* caster = GetCaster())
                    if (Creature* twistedFate = caster->ToCreature())
                    {
                        targetGuid = CAST_AI(mob_first_twisted_fate::mob_first_twisted_fateAI, twistedFate->AI())->summonerGuid;

                        if (Unit* target = ObjectAccessor::FindUnit(targetGuid))
                            twistedFate->SetDisplayId(target->GetDisplayId());
                    }
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_first_twisted_fate_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_first_twisted_fate_SpellScript();
        }
};

// Twisted Fate (second) - 137964
class spell_second_twisted_fate : public SpellScriptLoader
{
    public:
        spell_second_twisted_fate() :  SpellScriptLoader("spell_second_twisted_fate") { }

        class spell_second_twisted_fate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_second_twisted_fate_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                uint64 targetGuid = 0;

                if (Unit* caster = GetCaster())
                    if (Creature* twistedFate = caster->ToCreature())
                    {
                        targetGuid = CAST_AI(mob_second_twisted_fate::mob_second_twisted_fateAI, twistedFate->AI())->summonerGuid;

                        if (Unit* target = ObjectAccessor::FindUnit(targetGuid))
                            twistedFate->SetDisplayId(target->GetDisplayId());
                    }
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_second_twisted_fate_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_second_twisted_fate_SpellScript();
        }
};

// Twisted Fate (damage) - 137972
class spell_twisted_fate_damage : public SpellScriptLoader
{
    public:
        spell_twisted_fate_damage() :  SpellScriptLoader("spell_twisted_fate_damage") { }

        class spell_twisted_fate_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_twisted_fate_damage_SpellScript);

            void DealDamage()
            {
                Unit* caster = GetCaster();
                uint16 distance = 0;
                float distanceMax = 0;

                if (!caster)
                    return;

                if (Creature* twistedFate = caster->ToCreature())
                {
                    if (twistedFate->GetEntry() == NPC_FIRST_TWISTED_FATE)
                    {
                        if (Creature* creature = GetClosestCreatureWithEntry(twistedFate, NPC_SECOND_TWISTED_FATE, 200.0f))
                        {
                            distance = creature->GetDistance(twistedFate);
                            distanceMax = CAST_AI(mob_first_twisted_fate::mob_first_twisted_fateAI, twistedFate->AI())->distanceMax;
                        }
                    }

                    else if (twistedFate->GetEntry() == NPC_SECOND_TWISTED_FATE)
                    {
                        if (Creature* creature = GetClosestCreatureWithEntry(twistedFate, NPC_FIRST_TWISTED_FATE, 200.0f))
                        {
                            distance = creature->GetDistance(twistedFate);
                            distanceMax = CAST_AI(mob_second_twisted_fate::mob_second_twisted_fateAI, twistedFate->AI())->distanceMax;
                        }
                    }

                    if (distance >= 0.0f && distance <= distanceMax)
                        SetHitDamage(100000.0f + int32(250000.0f * (distance / distanceMax)));
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_twisted_fate_damage_SpellScript::DealDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_twisted_fate_damage_SpellScript();
        }
};

void AddSC_boss_council_of_elders()
{
    new npc_gara_jal_s_soul();
    new boss_king_malakk();
    new boss_kazra_jin();
    new boss_sul_the_sandcrawler();
    new boss_high_priestress_mar_li();
    new mob_living_sand();
    new mob_blessed_loa_spirit();
    new mob_shadowed_lua_spirit();
    new mob_first_twisted_fate();
    new mob_second_twisted_fate();
    new spell_reckless_charge_rolling();
    new spell_reckless_charge_movement();
    new spell_frigid_assault();
    new spell_lingering_presence();
    new spell_biting_cold_malakk();
    new spell_frosbite_malakk();
    new spell_frosbite_malakk_aura();
    new spell_overload_discharge_kazra_jin();
    new spell_ensnared();
    new spell_sandstorm();
    new spell_dark_power();
    new spell_soul_fragment();
    new spell_soul_fragment_switch();
    new spell_first_twisted_fate();
    new spell_second_twisted_fate();
    new spell_twisted_fate_damage();
}