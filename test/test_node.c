#include <assert.h>

#include "chess-tables/node.h"

int main() {
    NodeValue terminal_1 = {1, 1, 0, 0};
    NodeValue terminal_2 = {2, 2, 0, 0};

    assert(node_value_terminal(terminal_2));

    NodeValue node_1 = NODE_VALUE_INITIAL;
    node_1 = node_value_negamax(node_1, terminal_1);
    node_1 = node_value_negamax(node_1, terminal_2);

    assert(node_1.low == -1);
    assert(node_1.high == -1);
    assert(node_1.low_distance == 1);
    assert(node_1.high_distance == 1);

    NodeValue node_2 = NODE_VALUE_INITIAL;
    node_2 = node_value_negamax(node_2, NODE_VALUE_UNKNOWN);
    node_2 = node_value_negamax(node_2, node_1);

    assert(node_2.low == 1);
    assert(node_2.high == VALUE_MAX);
    assert(node_2.low_distance == 2);
    assert(node_2.high_distance == DISTANCE_MAX);
    assert(!node_value_equal(node_1, node_2));

    return 0;
}
