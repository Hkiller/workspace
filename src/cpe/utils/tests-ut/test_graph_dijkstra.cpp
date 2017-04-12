#include "GraphTest.hpp"

TEST_F(GraphTest, dijkstras_basic) {
    add_nodes("a b c d e f");

	add_edge("a", "b", 7);
	add_edge("a", "c", 9);
    add_edge("a", "f", 14);

	add_edge("b", "c", 10);
    add_edge("b", "d", 15);

    add_edge("c", "d", 11);
	add_edge("c", "f", 2);

    add_edge("d", "e", 6);

    add_edge("e", "f", 9);

    ASSERT_EQ(0, shortest_path_dijkstra("a", "e"));

    ASSERT_STREQ("a[0]-(9)->c[9]-(11)->d[20]-(6)->e[26]", dump_weight_path());
}

