#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "BuildFromXmlTest.hpp"

class BuildFromXmlStructAlign : public BuildFromXmlTest {
};

TEST_F(BuildFromXmlStructAlign, align1_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='S1' version='1' id='33' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    EXPECT_EQ(1, dr_meta_align(meta("S1")));
    EXPECT_EQ((size_t)3, dr_meta_size(meta("S1")));
}

TEST_F(BuildFromXmlStructAlign, align2_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='S1' version='1' id='33' align='2'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ((size_t)4, dr_meta_size(meta("S1")));
}


TEST_F(BuildFromXmlStructAlign, align_composite_2_1) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='S1' version='1' id='33' align='2'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='S2' version='1' id='34' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ((size_t)4, dr_meta_size(meta("S1")));
    EXPECT_EQ((size_t)5, dr_meta_size(meta("S2")));
}

TEST_F(BuildFromXmlStructAlign, align_composite_1_2) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='S1' version='1' id='33' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='S2' version='1' id='34' align='2'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(1)
    struct S1 {
        int8_t a1;
        int16_t a2;
    };
#pragma pack(2)
    struct S2 {
        int8_t a1;
        S1 a2;
    };
#pragma pack()

    EXPECT_EQ(sizeof(S1), dr_meta_size(meta("S1")));
    EXPECT_EQ(sizeof(S2), dr_meta_size(meta("S2")));

    S2 s2;
    EXPECT_EQ((size_t)((char*)&s2.a2 - (char*)&s2), dr_entry_data_start_pos(entry("S2", "a2"), 0));
    EXPECT_EQ((size_t)1, dr_entry_data_start_pos(entry("S2", "a2"), 0));
}

TEST_F(BuildFromXmlStructAlign, align_composite) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='S1' version='1' id='33' align='2'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='S2' version='1' id='34' align='4'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='S1'/>"
        "    </struct>"
        "    <struct name='S3' version='1' id='35' align='8'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='S2'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(2)
    struct S1 {
        int8_t a1;
        int16_t a2;
    };
#pragma pack(4)
    struct S2 {
        int8_t a1;
        S1 a2;
    };
#pragma pack(8)
    struct S3 {
        int8_t a1;
        S2 a2;
    };
#pragma pack()

    EXPECT_EQ(sizeof(S1), dr_meta_size(meta("S1")));
    EXPECT_EQ(sizeof(S2), dr_meta_size(meta("S2")));
    EXPECT_EQ(sizeof(S3), dr_meta_size(meta("S3")));

    S3 s3;
    EXPECT_EQ((size_t)2, dr_entry_data_start_pos(entry("S2", "a2"), 0));
    EXPECT_EQ((size_t)((char*)&s3.a2 - (char*)&s3), dr_entry_data_start_pos(entry("S3", "a2"), 0));
}

TEST_F(BuildFromXmlStructAlign, align_array_fix) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='S1' version='1' id='33' align='2'>"
        "	     <entry name='a2' type='int16'/>"
        "	     <entry name='a1' type='int8'/>"
        "    </struct>"
        "    <struct name='S2' version='1' id='34' align='4'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='S1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(2)
    struct S1 {
        int16_t a2;
        int8_t a1;
    };
#pragma pack(4)
    struct S2 {
        int8_t a1;
        S1 a2[5];
    };
#pragma pack()

    EXPECT_EQ(sizeof(S1), dr_meta_size(meta("S1")));
    EXPECT_EQ(sizeof(S2), dr_meta_size(meta("S2")));

    S2 s2;
    EXPECT_EQ((size_t)((char*)&s2.a2[0] - (char*)&s2), dr_entry_data_start_pos(entry("S2", "a2"), 0));
    EXPECT_EQ((size_t)((char*)&s2.a2[1] - (char*)&s2), dr_entry_data_start_pos(entry("S2", "a2"), 1));
}

TEST_F(BuildFromXmlStructAlign, align_union_array) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='Pos' version='1'>"
        "        <entry name='category' type='uint8' id='1'/>"
        "        <entry name='pos' type='uint8' id='2'/>"
        "    </struct>"
        "    <struct name='Data' version='1' align='8'>"
        "        <entry name='gid' type='uint64' id='1'/>"
        "        <entry name='exp' type='uint32' id='2'/>"
        "        <entry name='hero_id' type='uint16' id='3'/>"
        "        <entry name='pos' type='Pos' id='4'/>"
        "        <entry name='level' type='uint8' id='5'/>"
        "        <entry name='qulity' type='uint8' id='6'/>"
        "    </struct>"
        "    <union name='DataOpData' version='1' align='8'>"
        "        <entry name='add' type='Data' id='1'/>"
        "        <entry name='update' type='Data' id='2'/>"
        "        <entry name='remove' type='uint64' id='3'/>"
        "    </union>"
        "    <struct name='DataOp' version='1' align='8'>"
        "        <entry name='type' type='int16' id='1'/>"
        "        <entry name='data' type='DataOpData' id='2' select='type'/>"
        "    </struct>"
        "    <struct name='DataOpList' version='1' align='8'>"
        "        <entry name='count' type='uint16' id='1'/>"
        "        <entry name='data' type='DataOp' id='2' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>");

    struct Pos {
        uint8_t category;
        uint8_t pos;
    };

#pragma pack(8)    
    struct Data{
        uint64_t gid;
        uint32_t exp;
        uint16_t hero_id;
        Pos pos;
        uint8_t level;
        uint8_t qulity;
    };

    union DataOpData {
        Data add;
        Data update;
        uint64_t remove;
    };

    struct DataOp {
        uint16_t type;
        DataOpData data;
    };

    struct DataOpList {
        uint8_t count;
        DataOp data[1];
    } l;
#pragma pack()    

    EXPECT_EQ(sizeof(Data), dr_meta_size(meta("Data")));
    EXPECT_EQ(sizeof(DataOpData), dr_meta_size(meta("DataOpData")));
    EXPECT_EQ(sizeof(DataOp), dr_meta_size(meta("DataOp")));
    EXPECT_EQ(sizeof(DataOpList), dr_meta_size(meta("DataOpList")));

    EXPECT_EQ(
        (size_t)((char*)&l.data[0].data.add.gid - (char*)&l),
        dr_meta_path_to_off(meta("DataOpList"), "data[0].data.add.gid", NULL));

    EXPECT_EQ(
        (size_t)((char*)&l.data[1].data.add.gid - (char*)&l),
        dr_meta_path_to_off(meta("DataOpList"), "data[1].data.add.gid", NULL));

    EXPECT_EQ(
        (size_t)((char*)&l.data[5].data.add.gid - (char*)&l),
        dr_meta_path_to_off(meta("DataOpList"), "data[5].data.add.gid", NULL));
}

TEST_F(BuildFromXmlStructAlign, align_bug_case_1) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='HeroPos' version='1' align='8'>"
        "        <entry name='category' type='uint8' id='1'/>"
        "        <entry name='pos' type='uint8' id='2'/>"
        "    </struct>"
        "    <struct name='Hero' version='1' align='8'>"
        "        <entry name='gid' type='uint64' id='1'/>"
        "        <entry name='exp' type='uint32' id='2'/>"
        "        <entry name='hero_id' type='uint16' id='3'/>"
        "        <entry name='pos' type='HeroPos' id='4'/>"
        "        <entry name='level' type='uint8' id='5'/>"
        "        <entry name='qulity' type='uint8' id='6'/>"
        "    </struct>"
        "</metalib>");

#pragma pack(8)
    struct HeroPos {
        uint8_t capacity;
        uint8_t pos;
    };
    
    struct Hero {
        uint64_t gid;
        uint32_t exp;
        uint16_t hero_id;
        HeroPos pos;
        uint8_t level;
        uint8_t qulity;
    };
#pragma pack()

    Hero hero;

    EXPECT_EQ(sizeof(hero), dr_meta_size(meta("Hero")));
    EXPECT_EQ(sizeof(hero.pos), dr_meta_size(meta("HeroPos")));

    EXPECT_EQ((size_t)((char*)&hero.gid - (char*)&hero), dr_entry_data_start_pos(entry("Hero", "gid"), 0));
    EXPECT_EQ((size_t)((char*)&hero.exp - (char*)&hero), dr_entry_data_start_pos(entry("Hero", "exp"), 0));
    EXPECT_EQ((size_t)((char*)&hero.hero_id - (char*)&hero), dr_entry_data_start_pos(entry("Hero", "hero_id"), 0));
    EXPECT_EQ((size_t)((char*)&hero.pos - (char*)&hero), dr_entry_data_start_pos(entry("Hero", "pos"), 0));
    EXPECT_EQ((size_t)((char*)&hero.level - (char*)&hero), dr_entry_data_start_pos(entry("Hero", "level"), 0));
    EXPECT_EQ((size_t)((char*)&hero.qulity - (char*)&hero), dr_entry_data_start_pos(entry("Hero", "qulity"), 0));
}

