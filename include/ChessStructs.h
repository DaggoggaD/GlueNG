/**
 * @file ChessStructs.h
 * @brief Definitions for the core data structures used to represent the chessboard and game state.
 */
#ifndef CHESS_STRUCTS_H
#define CHESS_STRUCTS_H
#define PieceTypeN 6
#define PlayerN 2

#include "Headers.h"

/**
 * @struct UndoHistory
 * @brief Stores irreversible information necessary to completely unmake a move.
 */
typedef struct UndoHistory {
	int castlingPerms;
	int enPassant;
	int halfMoves;
	PieceType capturedPiece;
	U64 hashKey;
} UndoHistory;

/**
 * @struct Board
 * @brief Represents the complete state of a chess game.
 */
typedef struct {
	SelectionColor sideToMove;

	/** @brief Pieces bitboards: [SelectionColor White - Black][PAWN / ROOK / ...] */
	U64 pieceBitboards[PlayerN][PieceTypeN];

	/** @brief Occupancy bitboards: [SelectionColor White - Black - Both] */
	U64 occupiedBitboards[3];

	/** @brief Bigger, slower array used only to check piece type on a specific square. NONE on empty squares. */
	PieceType pieceOnSquare[64];

	/** @brief Special moves information. Castling permissions using bit flags. */
	int castlingPerms;

	/** @brief -1 if no en passant square, otherwise the index. */
	int enPassant;

	/** @brief Move state information. Number of half-moves played in the current game. */
	int gamePly;

	/** @brief Half-moves since the last capture or pawn advance (for the 50-move rule). */
	int halfMoves;

	short fiftyMoveCounter;
	short repetitionCount;

	/** @brief History array to restore previous states during unmake_move. */
	UndoHistory history[1024];

	/** @brief Evaluation helper stats. Tracks king positions for quick distance evaluations. */
	int kingSq[2];

	/** @brief Zobrist hash key of the current position. */
	U64 hashKey;

	/** @brief Used for draw detection (3-fold repetition). */
	U64 gameHistory[1024];

} Board;


#endif // !CHESS_STRUCTS_H
