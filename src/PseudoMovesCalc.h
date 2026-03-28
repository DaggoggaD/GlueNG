#ifndef PSEUDO_MOVES_CALC_H
#define PSEUDO_MOVES_CALC_H

#define GET_MOVE_SOURCE(move) ((move) & 0x3F)
#define GET_MOVE_TARGET(move) (((move) >> 6) & 0x3F)
#define GET_MOVE_PIECE(move)  (((move) >> 12) & 0xF)
#define GET_PROMOTION_PIECE(move)  (((move) >> 16) & 0xF)
#define GET_MOVE_EN_PASSANT(move)  (((move) >> 20) & 0x1)

#include "Headers.h"
#include <assert.h>

typedef struct MoveList{
    int moves[256];
	int scores[256];
    int count;
} MoveList;


// Returns a bitboard of pieces attacks from a given square bitboard.
// NOTE: PSEUDO-LEGAL.
U64 get_knight_pseudo_moves(int square, U64 ownPieces);

U64 get_king_pseudo_moves(int square, U64 ownPieces);

inline static U64 get_rook_pseudo_moves(int square, U64 blockers, U64 ownPieces);

inline static U64 get_bishop_pseudo_moves(int square, U64 blockers, U64 ownPieces);

inline static U64 get_queen_pseudo_moves(int square, U64 blockers, U64 ownPieces);

void white_generate_pseudo_moves(Board* board, MoveList* list);

void black_generate_pseudo_moves(Board* board, MoveList* list);

void add_move_to_list(MoveList* list, int fromSquare, int toSquare, PieceType piece, PieceType promoted_to, bool enPassant);

static inline int get_lsb_index(U64 b) {
	unsigned long index;
	_BitScanForward64(&index, b);
	return index;
}


bool is_square_attacked(Board* board, int square, SelectionColor attackingSide);

#endif // !PSEUDO_MOVES_CALC_H