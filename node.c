#include <assert.h>
#include <stdio.h>

#define VALUE_MIN (-127)
#define VALUE_MAX (127)
#define DISTANCE_MAX (254)

typedef signed char value_t;
typedef unsigned char distance_t;

typedef struct NodeValue
{
    value_t low;
    value_t high;
    distance_t low_distance;
    distance_t high_distance;
} NodeValue;

static const NodeValue NODE_VALUE_UNKNOWN = {VALUE_MIN, VALUE_MAX, DISTANCE_MAX, DISTANCE_MAX};

static const NodeValue NODE_VALUE_INITIAL = {VALUE_MIN, VALUE_MIN, DISTANCE_MAX, 0};

void node_value_repr(NodeValue node) {
    printf("(NodeValue){%d, %d, %d, %d}\n", node.low, node.high, node.low_distance, node.high_distance);
}

NodeValue node_value_negamax(NodeValue parent, NodeValue child) {
    #ifdef CONTRACT
        assert(parent.low <= parent.high);
        assert(child.low <= child.high);
        assert(parent.low != VALUE_MAX);
        assert(parent.high != VALUE_MIN);
        assert(child.low != VALUE_MAX);
        assert(child.high != VALUE_MIN);
    #endif
    #ifdef NODE_DEBUG
        print_node(parent);
        print_node(child);
    #endif
    distance_t low_distance = child.low_distance + 1;
    distance_t high_distance = child.high_distance + 1;
    if (-child.high > parent.low) {
        parent.low = -child.high;
        parent.low_distance = high_distance;
    }
    else if (-child.high == parent.low && high_distance < parent.low_distance) {
        parent.low_distance = high_distance;
    }
    if (-child.low > parent.high) {
        parent.high = -child.low;
        parent.high_distance = low_distance;
    }
    else if (-child.low == parent.high && low_distance > parent.high_distance) {
        parent.high_distance = low_distance;
    }
    if (parent.low_distance > DISTANCE_MAX) {
        parent.low_distance = DISTANCE_MAX;
    }
    if (parent.high_distance > DISTANCE_MAX) {
        parent.high_distance = DISTANCE_MAX;
    }
    #ifdef CONTRACT
        assert(parent.low <= parent.high);
    #endif
    return parent;
}

int node_value_equal(NodeValue a, NodeValue b) {
    if (a.low != b.low) {
        return 0;
    }
    if (a.high != b.high) {
        return 0;
    }
    if (a.low_distance != b.low_distance) {
        return 0;
    }
    return a.high_distance == b.high_distance;
}

int node_value_terminal(NodeValue node) {
    return !node.low_distance && !node.high_distance;
}
