#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "node.c"

#define MAX_CHILDREN (1024)

/*
 * Simplified chess where pieces can capture their own color
 */

typedef unsigned long long int piece_t;

typedef struct Board
{
    piece_t player;
    piece_t kings;
    piece_t rooks;
    piece_t bishops;
} Board;

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
    for (int i = 0; i < 64; ++i) {
        if ((1ULL << i) & piece) {
            return i;
        }
    }
    return -1;
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

Board do_the_rook(Board parent, piece_t rook, piece_t move) {
    Board child = parent;
    child.rooks ^= rook;
    child.player ^= rook;

    child.rooks |= move;
    child.player |= move;

    child.kings &= ~move;
    child.bishops &= ~move;

    child.player = ~child.player;
    return child;
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
            child.kings ^= king;
            child.player ^= king;

            child.kings |= move;
            child.player |= move;

            child.rooks &= ~move;
            child.bishops &= ~move;

            child.player = ~child.player;
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
            piece_t move = 1ULL << (i + 8*y);
            Board child = do_the_rook(parent, rook, move);
            out[num++] = child;
            if (move & any) {
                break;
            }
        }
        for (int i = x - 1; i >= 0; --i) {
            piece_t move = 1ULL << (i + 8*y);
            Board child = do_the_rook(parent, rook, move);
            out[num++] = child;
            if (move & any) {
                break;
            }
        }
        for (int i = y + 1; i < 8; ++i) {
            piece_t move = 1ULL << (x + 8*i);
            Board child = do_the_rook(parent, rook, move);
            out[num++] = child;
            if (move & any) {
                break;
            }
        }
        for (int i = y - 1; i >= 0; --i) {
            piece_t move = 1ULL << (x + 8*i);
            Board child = do_the_rook(parent, rook, move);
            out[num++] = child;
            if (move & any) {
                break;
            }
        }
        rooks ^= rook;
    }
    return num;
}

void solve_all_the_things() {
    size_t num = 65*65*65*65;
    NodeValue *nodes = malloc(num * sizeof(NodeValue));
    Board children_[MAX_CHILDREN];

    for (size_t i = 0; i < num; ++i) {
        float res = result(from_hash(i));
        if (isnan(res)) {
            nodes[i] = NODE_VALUE_UNKNOWN;
        } else {
            nodes[i] = (NodeValue){res, res, 0, 0};
        }
    }

    int running = 1;
    while (running) {
        running = 0;
        for (size_t i = 0; i < num; ++i) {
            if (i % 1000000 == 0) {
                printf("%zu\n", 100 * i / num);
            }
            if (node_value_terminal(nodes[i])) {
                continue;
            }
            NodeValue parent = NODE_VALUE_INITIAL;
            int num_children = children(from_hash(i), children_);
            for (int j = 0; j < num_children; ++j) {
                size_t h = hash(children_[j]);
                NodeValue child = nodes[h];
                parent = node_value_negamax(parent, child);
            }
            if (!node_value_equal(parent, nodes[i])) {
                running = 1;
                nodes[i] = parent;
            }
        }
        Board board = {.player=3, .kings=4097, .rooks=2};
        print_board(board);
        node_value_repr(nodes[hash(board)]);
        int numc = children(board, children_);
        printf("%d\n", numc);
        for (int i = 0; i < numc; ++i) {
            Board child = children_[i];
            print_board(child);
            node_value_repr(nodes[hash(child)]);
        }
    }

    free(nodes);
}

void main() {
    solve_all_the_things();
}