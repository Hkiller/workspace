#include "ReqTest.hpp"

TEST_F(ReqTest, req_create_basic) {
    dp_req_t req = createReq("req1", 12);
    ASSERT_TRUE(req);

    EXPECT_TRUE(NULL == dp_req_parent(req));
}

TEST_F(ReqTest, req_create_child_basic) {
    dp_req_t parent = createReq("req1", 12);
    ASSERT_TRUE(parent);

    char buf[33];

    dp_req_t child = t_dp_req_create_child(parent, "req2", buf, sizeof(buf));
    ASSERT_TRUE(child);

    EXPECT_TRUE(parent == dp_req_parent(child));
}

