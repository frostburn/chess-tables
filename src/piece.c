#include "chess-tables/piece.h"

#define C64(x) (x ## ULL)

/**
 * Flip a bitboard vertically about the centre ranks.
 * Rank 1 is mapped to rank 8 and vice versa.
 * @param x any bitboard
 * @return bitboard x flipped vertically
 */
piece_t piece_mirror_v(piece_t x) {
    return __builtin_bswap64(x);
}

/**
 * Mirror a bitboard horizontally about the center files.
 * File a is mapped to file h and vice versa.
 * @param x any bitboard
 * @return bitboard x mirrored horizontally
 */
piece_t piece_mirror_h (piece_t x) {
    const piece_t k1 = C64(0x5555555555555555);
    const piece_t k2 = C64(0x3333333333333333);
    const piece_t k4 = C64(0x0f0f0f0f0f0f0f0f);
    x = ((x >> 1) & k1) +  2*(x & k1);
    x = ((x >> 2) & k2) +  4*(x & k2);
    x = ((x >> 4) & k4) + 16*(x & k4);
    return x;
}

/**
 * Flip a bitboard about the diagonal a1-h8.
 * Square h1 is mapped to a8 and vice versa.
 * @param x any bitboard
 * @return bitboard x flipped about diagonal a1-h8
 */
piece_t piece_mirror_d(piece_t x) {
    piece_t t;
    const piece_t k1 = C64(0x5500550055005500);
    const piece_t k2 = C64(0x3333000033330000);
    const piece_t k4 = C64(0x0f0f0f0f00000000);
    t  = k4 & (x ^ (x << 28));
    x ^=       t ^ (t >> 28) ;
    t  = k2 & (x ^ (x << 14));
    x ^=       t ^ (t >> 14) ;
    t  = k1 & (x ^ (x <<  7));
    x ^=       t ^ (t >>  7) ;
    return x;
}

/**
 * Flip a bitboard about the antidiagonal a8-h1.
 * Square a1 is mapped to h8 and vice versa.
 * @param x any bitboard
 * @return bitboard x flipped about antidiagonal a8-h1
 */
piece_t piece_mirror_a(piece_t x) {
   piece_t t;
   const piece_t k1 = C64(0xaa00aa00aa00aa00);
   const piece_t k2 = C64(0xcccc0000cccc0000);
   const piece_t k4 = C64(0xf0f0f0f00f0f0f0f);
   t  =       x ^ (x << 36) ;
   x ^= k4 & (t ^ (x >> 36));
   t  = k2 & (x ^ (x << 18));
   x ^=       t ^ (t >> 18) ;
   t  = k1 & (x ^ (x <<  9));
   x ^=       t ^ (t >>  9) ;
   return x;
}

piece_t piece_rot_180(piece_t x) {
    return piece_mirror_h(piece_mirror_v(x));
}

piece_t piece_rot_270(piece_t x) {
    return piece_mirror_d(piece_mirror_h(x));
}

piece_t piece_rot_90(piece_t x) {
    return piece_mirror_d(piece_mirror_v(x));
}
