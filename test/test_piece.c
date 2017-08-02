#include <assert.h>

#include "chess-tables/piece.h"

int main() {
    piece_t corner = 1;
    assert(piece_mirror_v(corner) == 1ULL << (8*7));
    assert(piece_mirror_h(corner) == 1ULL << 7);
    assert(piece_mirror_d(corner) == 1ULL);

    return 0;
}
