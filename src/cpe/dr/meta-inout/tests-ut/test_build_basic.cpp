#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "BuildTest.hpp"

TEST_F(BuildTest, metalib_basic) {
    struct DRInBuildMeta * seq_meta = dr_inbuild_metalib_add_meta(m_builder);
    ASSERT_TRUE(seq_meta);

    dr_inbuild_meta_set_current_version(seq_meta, 1);
    dr_inbuild_meta_set_type(seq_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(seq_meta, "seq");

    struct DRInBuildMetaEntry * count_entry = dr_inbuild_meta_add_entry(seq_meta);
    ASSERT_TRUE(count_entry);
    dr_inbuild_entry_set_name(count_entry, "count");
    dr_inbuild_entry_set_id(count_entry, -1);
    dr_inbuild_entry_set_type(count_entry, "uint32");
    dr_inbuild_entry_set_array_count(count_entry, 1);

    struct DRInBuildMetaEntry * data_entry = dr_inbuild_meta_add_entry(seq_meta);
    ASSERT_TRUE(data_entry);
    dr_inbuild_entry_set_name(data_entry, "data");
    dr_inbuild_entry_set_id(data_entry, -1);
    dr_inbuild_entry_set_type(data_entry, "uint32");
    dr_inbuild_entry_set_array_refer(data_entry, "count");
    dr_inbuild_entry_set_array_count(data_entry, 0);

    ASSERT_EQ(0, build());

    LPDRMETA r_meta = meta("seq");
    ASSERT_TRUE(r_meta);

    LPDRMETAENTRY r_entry_data = entry("seq", "data");
    ASSERT_TRUE(r_entry_data);

    EXPECT_EQ((size_t)4, dr_entry_data_start_pos(r_entry_data, 0));
}

