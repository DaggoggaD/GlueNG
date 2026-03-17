#ifndef CHESS_STRUCTS_H
#define CHESS_STRUCTS_H
#define PieceTypeN 6
#define PlayerN 2

#include "ChessDefinitions.h"


typedef struct {
	SelectionColor sideToMove;

	// Pieces bitboards: [SelectionColor White - Black][PAWN / ROOK / ...]
	U64 pieceBitboards[PlayerN][PieceTypeN];

	// Occupancy bitboards: [SelectionColor White - Black - Both]
	U64 occupiedBitboards[3];

	// Special moves information
	int castlingPerms;
	int enPassant; //-1 if no en passant square, otherwise the index

	// Move state information
	int halfMoves;
	short fiftyMoveCounter;
	short repetitionCount;

	// Evaluation helper stats
	int kingSq[2];

} Board;


#endif // !CHESS_STRUCTS_H
