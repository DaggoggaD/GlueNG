#ifndef PSEUDO_MOVES_CALC_H
#define PSEUDO_MOVES_CALC_H

#define GET_MOVE_SOURCE(move) ((move) & 0x3F)
#define GET_MOVE_TARGET(move) (((move) >> 6) & 0x3F)
#define GET_MOVE_PIECE(move)  (((move) >> 12) & 0xF)

#include "Headers.h"

typedef struct {
    int moves[256];
    int count;
} MoveList;


// Returns a bitboard of pieces attacks from a given square bitboard.
// NOTE: PSEUDO-LEGAL.

U64 get_knight_pseudo_moves(int square, U64 ownPieces);

U64 get_king_pseudo_moves(int square, U64 ownPieces);

U64 get_rook_pseudo_moves(int square, U64 blockers, U64 ownPieces);

U64 get_bishop_pseudo_moves(int square, U64 blockers, U64 ownPieces);

U64 get_queen_pseudo_moves(int square, U64 blockers, U64 ownPieces);

void white_generate_pseudo_moves(Board* board, MoveList* list);

#endif // !PSEUDO_MOVES_CALC_H
