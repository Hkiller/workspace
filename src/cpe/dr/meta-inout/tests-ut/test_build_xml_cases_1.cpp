#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "BuildFromXmlTest.hpp"

TEST_F(BuildFromXmlTest, case_1) {
    parseMeta(
        "<metalib tagsetversion='1' name='NetData' version='1'>"
        "    <struct name='Skill' version='1'>"
        "        <entry name='skill_id' type='uint16' id='1'/>"
        "        <entry name='level' type='uint8' id='2'/>"
        "    </struct>"
        "    <struct name='Buff' version='1'>"
        "        <entry name='local_id' type='uint16'/>"
        "        <entry name='buff_id' type='uint8'/>"
        "        <entry name='expire_time' type='uint32'/>"
        "    </struct>"
        "    <struct name='RoleNumericAttrs' version='1'>"
        "        <entry name='v_l1_strength' type='uint16'/>"
        "        <entry name='v_l1_intelligence' type='uint16'/>"
        "        <entry name='v_l1_defence' type='uint16'/>"
        "        <entry name='v_l1_quality' type='uint16'/>"
        "        <entry name='v_l2_life' type='uint32'/>"
        "        <entry name='v_l2_physics_damage' type='uint32'/>"
        "        <entry name='v_l2_physics_defence' type='uint32'/>"
        "        <entry name='v_l2_magic_damage' type='uint32'/>"
        "        <entry name='v_l2_magic_defence' type='uint32'/>"
        "        <entry name='v_l2_block' type='uint16'/>"
        "        <entry name='v_l2_crit' type='uint16'/>"
        "        <entry name='v_l2_init_angry' type='uint16'/>"
        "    </struct>"
        "    <struct name='BattleRoleData' version='1'>"
        "        <entry name='name' type='string' size='64' id='1'/>"
        "        <entry name='level' type='uint16'  id='2'/>"
        "        <entry name='pos' type='uint8' id='3'/>"
        "        <entry name='buff_count' type='uint8' id='4'/>"
        "        <entry name='buffs' type='Buff' count='7' refer='buff_count' id='5'/>"
        "        <entry name='skill_count' type='uint8' id='6'/>"
        "        <entry name='skills' type='Skill' count='4' refer='skill_count' id='7'/>"
        "        <entry name='panel_values' type='RoleNumericAttrs' id='8'/>"
        "    </struct>"
        "    <struct name='BattleHeroData' version='1'>"
        "        <entry name='hero_id' type='uint16' id='1'/>"
        "        <entry name='pos' type='uint8' id='2'/>"
        "        <entry name='level' type='uint8' id='3'/>"
        "        <entry name='qulity' type='uint8' id='4'/>"
        "    </struct>"
        "    <struct name='BattleSideData' version='1'>"
        "        <entry name='role' type='BattleRoleData' id='1'/>"
        "        <entry name='hero_count' type='uint8' id='2'/>"
        "        <entry name='heros' type='BattleHeroData' count='8' refer='hero_count' id='3'/>"
        "    </struct>"
        "    <struct name='BattleData' version='1' id='METATYPEID_BATTLE_DATA'>"
        "        <entry name='l_side' type='BattleSideData' id='1'/>"
        "        <entry name='r_side' type='BattleSideData' id='2'/>"
        "    </struct>"
        "</metalib>",
        8
        );

    typedef struct _Skill {
        uint16_t skill_id;
        uint8_t level;
    } SKILL;

    typedef struct _Buff {
        uint16_t local_id;
        uint8_t buff_id;
        uint32_t expire_time;
    } BUFF;

    typedef struct _RoleNumericAttrs {
        uint16_t v_l1_strength;
        uint16_t v_l1_intelligence;
        uint16_t v_l1_defence;
        uint16_t v_l1_quality;
        uint32_t v_l2_life;
        uint32_t v_l2_physics_damage;
        uint32_t v_l2_physics_defence;
        uint32_t v_l2_magic_damage;
        uint32_t v_l2_magic_defence;
        uint16_t v_l2_block;
        uint16_t v_l2_crit;
        uint16_t v_l2_init_angry;
    } ROLENUMERICATTRS;

    typedef struct _BattleRoleData {
        char name[64];
        uint16_t level;
        uint8_t pos;
        uint8_t buff_count;
        BUFF buffs[7];
        uint8_t skill_count;
        SKILL skills[4];
        ROLENUMERICATTRS panel_values;
    } BATTLEROLEDATA;

    typedef struct _BattleHeroData {
        uint16_t hero_id;
        uint8_t pos;
        uint8_t level;
        uint8_t qulity;
    } BATTLEHERODATA;

    typedef struct _BattleSideData {
        BATTLEROLEDATA role;
        uint8_t hero_count;
        BATTLEHERODATA heros[8];
    } BATTLESIDEDATA;

    EXPECT_EQ(sizeof(SKILL), dr_meta_size(meta("Skill")));
    EXPECT_EQ(sizeof(BATTLEROLEDATA), dr_meta_size(meta("BattleRoleData")));
    EXPECT_EQ(sizeof(BATTLESIDEDATA), dr_meta_size(meta("BattleSideData")));

    BATTLESIDEDATA data;
    EXPECT_EQ(
        (const char *)&data.hero_count - (const char *)&data,
        dr_entry_data_start_pos(entry("BattleSideData", "hero_count"), 0));
}
