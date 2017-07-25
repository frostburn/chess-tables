#ifndef CHESS_TABLESS_NODE_H_GUARD
#define CHESS_TABLESS_NODE_H_GUARD

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

void node_value_repr(NodeValue node);

NodeValue node_value_negamax(NodeValue parent, NodeValue child);

int node_value_equal(NodeValue a, NodeValue b);

int node_value_terminal(NodeValue node);

#endif
