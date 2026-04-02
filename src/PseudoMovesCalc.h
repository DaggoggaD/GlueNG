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


// Functions to get the pseudo legal moves for a piece on a square, 
// given the blockers and own pieces bitboards. Returns moves bitboard.

U64 get_knight_pseudo_moves(int square, U64 ownPieces);

U64 get_king_pseudo_moves(int square, U64 ownPieces);

inline static U64 get_rook_pseudo_moves(int square, U64 blockers, U64 ownPieces);

inline static U64 get_bishop_pseudo_moves(int square, U64 blockers, U64 ownPieces);

inline static U64 get_queen_pseudo_moves(int square, U64 blockers, U64 ownPieces);


// Bit packing to save space:
// 0 - 5: fromSquare (0-63) 6 - 11: toSquare (0-63) 12 - 15: piece type (0-5)
// 16 - 19: promotion 20: enPassant (0 - 1)
// 
// Note: promotion: 0 = no promotion, != 0 PieceType piece
//		 enPassant: 0 = not enPassant move, 1 = enPassant move
// 
// Decode it with GET_FROM_SQUARE(move), GET_TO_SQUARE(move), GET_PIECE_TYPE(move) macros.
void add_move_to_list(MoveList* list, int fromSquare, int toSquare, PieceType piece, PieceType promoted_to, bool enPassant);

// Generate and add to list all the possible pseudo legal moves in a certain board.
void white_generate_pseudo_moves(Board* board, MoveList* list);

void black_generate_pseudo_moves(Board* board, MoveList* list);

// Learnt from https://www.chessprogramming.org/BitScan, 
// gets the index of the least significant bit in a bitboard.
static inline int get_lsb_index(U64 b) {
	unsigned long index;
	_BitScanForward64(&index, b);
	return index;
}

// Checks if a square is attacked by any "attackingSide" pieces.
bool is_square_attacked(Board* board, int square, SelectionColor attackingSide);

#endif // !PSEUDO_MOVES_CALC_H