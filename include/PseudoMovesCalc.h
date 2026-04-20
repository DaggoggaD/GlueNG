/**
* @file PseudoMovesCalc.h
* @brief This file contains the function declarations for calculating pseudo legal moves in the chess engine. 
* * It includes functions to generate pseudo-legal moves for each piece type, as well as a function to check if a square is attacked by any pieces of a given color. The moves are represented in a compact format using bit packing, and the file also defines macros to extract move information from the packed move representation.
*/

#ifndef PSEUDO_MOVES_CALC_H
#define PSEUDO_MOVES_CALC_H

#include "Headers.h"
#include <assert.h>

/**
 * @name Move Bit-Packing Macros
 * @brief Macros to extract move information from the 32 bit packed move representation.
 * * Bit packing structure:
 * - 0 - 5: fromSquare (0-63)
 * - 6 - 11: toSquare (0-63)
 * - 12 - 15: piece type (0-5)
 * - 16 - 19: promotion (0 = no promotion, otherwise promoted PieceType)
 * - 20: enPassant (0 = not enPassant move, 1 = enPassant move)
 * @{
 */
#define GET_MOVE_SOURCE(move) ((move) & 0x3F)
#define GET_MOVE_TARGET(move) (((move) >> 6) & 0x3F)
#define GET_MOVE_PIECE(move)  (((move) >> 12) & 0xF)
#define GET_PROMOTION_PIECE(move)  (((move) >> 16) & 0xF)
#define GET_MOVE_EN_PASSANT(move)  (((move) >> 20) & 0x1)
 /** @} */


/**
 * @struct MoveList
 * @brief Structure to hold a list of generated moves and their corresponding scores.
 */
typedef struct MoveList{
    int moves[256];		/**< Array storing the bit packed moves. */
	int scores[256];	/**< Array storing the score of each move for move ordering. */
    int count;			/**< Number of moves currently in the list. */
} MoveList;


/**
 * @brief Gets the pseudo legal moves for a Knight.
 * @param square The square index (0-63) where the Knight is located.
 * @param ownPieces A bitboard representing all pieces of the moving side.
 * @return A bitboard with all valid target squares for the Knight.
 */
U64 get_knight_pseudo_moves(int square, U64 ownPieces);

/**
 * @brief Gets the pseudo legal moves for a King.
 * @param square The square index (0-63) where the King is located.
 * @param ownPieces A bitboard representing all pieces of the moving side.
 * @return A bitboard with all valid target squares for the King.
 */
U64 get_king_pseudo_moves(int square, U64 ownPieces);

/**
 * @brief Gets the pseudo legal moves for a Rook.
 * @param square The square index (0-63) where the Rook is located.
 * @param blockers A bitboard representing all pieces on the board (both colors).
 * @param ownPieces A bitboard representing all pieces of the moving side.
 * @return A bitboard with all valid target squares for the Rook.
 */
U64 get_rook_pseudo_moves(int square, U64 blockers, U64 ownPieces);

/**
 * @brief Gets the pseudo legal moves for a Bishop.
 * @param square The square index (0-63) where the Bishop is located.
 * @param blockers A bitboard representing all pieces on the board (both colors).
 * @param ownPieces A bitboard representing all pieces of the moving side.
 * @return A bitboard with all valid target squares for the Bishop.
 */
U64 get_bishop_pseudo_moves(int square, U64 blockers, U64 ownPieces);

/**
 * @brief Gets the pseudo legal moves for a Queen.
 * @param square The square index (0-63) where the Queen is located.
 * @param blockers A bitboard representing all pieces on the board (both colors).
 * @param ownPieces A bitboard representing all pieces of the moving side.
 * @return A bitboard with all valid target squares for the Queen.
 */
U64 get_queen_pseudo_moves(int square, U64 blockers, U64 ownPieces);


/**
 * @brief Encodes a move using bit packing and adds it to the MoveList.
 * @param list Pointer to the MoveList where the move will be added.
 * @param fromSquare The starting square index (0-63).
 * @param toSquare The destination square index (0-63).
 * @param piece The type of piece being moved.
 * @param promoted_to The piece type to promote to (0 if not a promotion).
 * @param enPassant Boolean indicating if the move is an en-passant capture.
 */
void add_move_to_list(MoveList* list, int fromSquare, int toSquare, PieceType piece, PieceType promoted_to, bool enPassant);

/**
 * @brief Generates all possible pseudo legal moves for White.
 * @param board Pointer to the current board state.
 * @param list Pointer to the MoveList to populate.
 */
void white_generate_pseudo_moves(Board* board, MoveList* list);

/**
 * @brief Generates all possible pseudo legal moves for Black.
 * @param board Pointer to the current board state.
 * @param list Pointer to the MoveList to populate.
 */
void black_generate_pseudo_moves(Board* board, MoveList* list);

/**
 * @brief Gets the index of the least significant bit (LSB) in a bitboard.
 * @note Learnt from https://www.chessprogramming.org/BitScan
 * @param b The bitboard to scan.
 * @return The index (0-63) of the least significant bit.
 */
static inline int get_lsb_index(U64 b) {
	unsigned long index;
	_BitScanForward64(&index, b);
	return index;
}

/**
 * @brief Checks if a specific square is attacked by any piece of the given color.
 * @param board Pointer to the current board state.
 * @param square The index of the square to check (0-63).
 * @param attackingSide The color of the potential attackers.
 * @return true if the square is attacked by the specified side, false otherwise.
 */
bool is_square_attacked(Board* board, int square, SelectionColor attackingSide);

#endif // !PSEUDO_MOVES_CALC_H