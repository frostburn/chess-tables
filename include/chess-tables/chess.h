#ifndef CHESS_TABLESS_CHESS_GUARD
#define CHESS_TABLESS_CHESS_GUARD

#include "chess-tables/piece.h"

typedef struct Board
{
    piece_t player;
    piece_t kings;
    piece_t rooks;
    piece_t bishops;
} Board;

void solve_all_the_things();

#endif
