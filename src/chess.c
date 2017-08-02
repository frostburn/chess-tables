#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "chess-tables/node.h"
#include "chess-tables/chess.h"
#include "full-dict/indicator.h"

#define MAX_CHILDREN (1024)

#define CANONIZE_INNER(op) {\
    board.kings = kings;\
    board.rooks = op(board.rooks);\
    board.bishops = op(board.bishops);\
    board.player = op(board.player);\
    return board;\
}

Board canonize_board(Board board) {
    piece_t kings = piece_mirror_v(board.kings);
    if (kings < board.kings) {
        CANONIZE_INNER(piece_mirror_v);
    }
    kings = piece_mirror_h(kings);
    if (kings < board.kings) {
        CANONIZE_INNER(piece_rot_180);
    }
    kings = piece_mirror_v(kings);
    if (kings < board.kings) {
        CANONIZE_INNER(piece_mirror_h);
    }
    kings = piece_mirror_d(kings);
    if (kings < board.kings) {
        CANONIZE_INNER(piece_rot_270);
    }
    kings = piece_mirror_v(kings);
    if (kings < board.kings) {
        CANONIZE_INNER(piece_mirror_a);
    }
    kings = piece_mirror_h(kings);
    if (kings < board.kings) {
        CANONIZE_INNER(piece_rot_90);
    }
    kings = piece_mirror_v(kings);
    if (kings < board.kings) {
        CANONIZE_INNER(piece_mirror_d);
    }
    return board;
}

int is_canonical(Board board) {
    piece_t kings = piece_mirror_v(board.kings);
    if (kings < board.kings) {
        return 0;
    }
    kings = piece_mirror_h(kings);
    if (kings < board.kings) {
        return 0;
    }
    kings = piece_mirror_v(kings);
    if (kings < board.kings) {
        return 0;
    }
    kings = piece_mirror_d(kings);
    if (kings < board.kings) {
        return 0;
    }
    kings = piece_mirror_v(kings);
    if (kings < board.kings) {
        return 0;
    }
    kings = piece_mirror_h(kings);
    if (kings < board.kings) {
        return 0;
    }
    kings = piece_mirror_v(kings);
    if (kings < board.kings) {
        return 0;
    }
    return 1;
}

void print_board(Board board) {
    for (int i = 0; i < 64; ++i) {
        piece_t p = 1ULL << i;
        if ((i + i / 8) % 2) {
            printf("\x1b[0;43m");
        } else {
            printf("\x1b[0;46m");
        }
        if (board.player & p) {
            printf("\x1b[37;1m");
        } else {
            printf("\x1b[30m");
        }
        if (board.kings & p) {
            printf("♚ ");
        } else if (board.rooks & p) {
            printf("♜ ");
        } else if (board.bishops & p) {
            printf("♝ ");
        } else {
            printf("  ");
        }
        // ♛ ♞ ♟
        if (i % 8 == 7) {
            printf("\x1b[0m\n");
        }
    }
    printf("\x1b[0m\n");
}

piece_t shift(piece_t piece, int x, int y) {
    if (x < 0) {
        piece >>= -x;
    } else {
        piece <<= x;
    }
    if (y < 0) {
        piece >>= -8*y;
    } else {
        piece <<= 8*y;
    }
    return piece;
}

float result(Board board) {
    if (!(board.rooks | board.bishops)) {
        return 0;
    }
    piece_t white_king = board.kings & board.player;
    piece_t black_king = board.kings & ~board.player;
    if (!black_king) {
        if (!white_king) {
            return 0;
        }
        return 1;
    } else  if (!white_king) {
        return -1;
    }
    return NAN;
}

int scan(piece_t piece) {
    return __builtin_ffsll(piece) - 1;
}

size_t hash(Board board) {
    piece_t white_king = board.kings & board.player;
    piece_t black_king = board.kings & ~board.player;
    piece_t white_rook = board.rooks & board.player;
    piece_t black_rook = board.rooks & ~board.player;

    size_t h = 0;
    h += scan(white_king) + 1;
    h *= 65;
    h += scan(black_king) + 1;
    h *= 65;
    h += scan(white_rook) + 1;
    h *= 65;
    h += scan(black_rook) + 1;

    return h;
}

Board from_hash(size_t h) {
    piece_t white_king = 0;
    piece_t black_king = 0;
    piece_t white_rook = 0;
    piece_t black_rook = 0;
    int idx;
    idx = (h % 65) - 1;
    if (idx >= 0) {
        black_rook = 1ULL << idx;
    }
    h /= 65;
    idx = (h % 65) - 1;
    if (idx >= 0) {
        white_rook = 1ULL << idx;
    }
    h /= 65;
    idx = (h % 65) - 1;
    if (idx >= 0) {
        black_king = 1ULL << idx;
    }
    h /= 65;
    idx = (h % 65) - 1;
    if (idx >= 0) {
        white_king = 1ULL << idx;
    }
    Board board = {
        .player=(white_king | white_rook),
        .kings=(white_king | black_king),
        .rooks=(white_rook | black_rook)
    };
    if (board.kings & board.rooks) {
        return (Board){0};
    }
    return board;
}

int indicator(keys_t k) {
    return is_canonical(from_hash(k));
}

#define MAKE_MOVE(child, type, piece, move) {\
    child.type ^= piece;\
    child.player ^= piece;\
    child.kings &= ~move;\
    child.rooks &= ~move;\
    child.bishops &= ~move;\
    child.type |= move;\
    child.player |= move;\
    child.player = ~child.player;\
}

#define BEAM_INNER(index, type, piece) {\
    piece_t move = 1ULL << (index);\
    if (move & any & parent.player) break;\
    Board child = parent;\
    MAKE_MOVE(child, type, piece, move);\
    out[num++] = child;\
    if (move & any) break;\
}

int children(Board parent, Board *out) {
    if (!isnan(result(parent))) {
        return 0;
    }
    int num = 0;
    int idx, x, y;

    piece_t any = parent.kings | parent.rooks | parent.bishops;
    piece_t king = parent.kings & parent.player;
    idx = scan(king);
    x = idx % 8;
    y = idx / 8;
    for (int j =  -1; j <= 1; ++j) {
        if (x + j < 0 || x + j >= 8) {
            continue;
        }
        for (int k =  -1; k <= 1; ++k) {
            if (y + k < 0 || y + k >= 8) {
                continue;
            }
            if (!j && !k) {
                continue;
            }
            piece_t move = shift(king, j, k);

            Board child = parent;
            MAKE_MOVE(child, kings, king, move);

            out[num++] = child;
        }
    }

    piece_t rooks = parent.rooks & parent.player;
    while (rooks) {
        idx = scan(rooks);
        piece_t rook = 1ULL << idx;
        x = idx % 8;
        y = idx / 8;
        for (int i = x + 1; i < 8; ++i) {
            BEAM_INNER(i + 8*y, rooks, rook);
        }
        for (int i = x - 1; i >= 0; --i) {
            BEAM_INNER(i + 8*y, rooks, rook);
        }
        for (int i = y + 1; i < 8; ++i) {
            BEAM_INNER(x + 8*i, rooks, rook);
        }
        for (int i = y - 1; i >= 0; --i) {
            BEAM_INNER(x + 8*i, rooks, rook);
        }
        rooks ^= rook;
    }
    return num;
}

void solve_all_the_things() {
    size_t num = 65*65*65*65;
    IndicatorDict *dict = malloc(sizeof(IndicatorDict));
    indicator_dict_init(dict, indicator, num, 256);

    NodeValue *nodes = malloc(dict->num_keys * sizeof(NodeValue));
    Board children_[MAX_CHILDREN];

    size_t i = 0;
    keys_t key = dict->min_key;
    while (key <= dict->max_key) {
        float res = result(from_hash(key));
        if (isnan(res)) {
            nodes[i] = NODE_VALUE_UNKNOWN;
        } else {
            nodes[i] = (NodeValue){res, res, 0, 0};
        }
        ++i;
        key = indicator_dict_next(dict, key);
    }

    printf("Number of positions %llu\n", dict->num_keys);

    int running = 1;
    while (running) {
        running = 0;
        i = 0;
        key = dict->min_key;
        while (key <= dict->max_key) {
            if (i % 100000 == 0) {
                printf("%zu\n", 100 * i / dict->num_keys);
            }
            if (node_value_terminal(nodes[i])) {
                ++i;
                key = indicator_dict_next(dict, key);
                continue;
            }
            Board board = from_hash(key);
            NodeValue parent = NODE_VALUE_INITIAL;
            int num_children = children(board, children_);
            for (int j = 0; j < num_children; ++j) {
                size_t h = hash(canonize_board(children_[j]));
                NodeValue child = nodes[indicator_dict_index(dict, h)];
                parent = node_value_negamax(parent, child);
            }
            if (!node_value_equal(parent, nodes[i])) {
                running = 1;
                nodes[i] = parent;
            }
            ++i;
            key = indicator_dict_next(dict, key);
        }
        Board board = {.player=3, .kings=4097, .rooks=2};
        board = canonize_board(board);
        print_board(board);
        node_value_repr(nodes[indicator_dict_index(dict, hash(board))]);
        int numc = children(board, children_);
        printf("%d\n", numc);
        for (int i = 0; i < numc; ++i) {
            Board child = canonize_board(children_[i]);
            print_board(child);
            node_value_repr(nodes[indicator_dict_index(dict, hash(child))]);
        }
    }

    free(nodes);
}
