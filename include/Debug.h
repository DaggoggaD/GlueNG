/**
 * @file Debug.h
 * @brief Function declarations for debugging and visualization utilities.
 */
#ifndef DEBUG_H
#define DEBUG_H

#include "Headers.h"

typedef struct MoveList MoveList;

/**
 * @brief Used to translate from enum to string to visualize pieces.
 * @param piece The PieceId enum value to translate.
 * @param str Pointer to a character array where the resulting string will be stored.
 */
void debug_select_string_from_piece(const PieceId piece, char* str);

/**
 * @brief Prints current checkerboard pieces, side to move, castling permissions, and halfmove.
 * @param board Pointer to the current board state to visualize.
 */
void debug_board_visualizer(Board* board);

/**
 * @brief Visualizes any bitmask as an 8x8 grid.
 * @note X: bit is set, . otherwise.
 * @param bitmask The 64-bit integer (bitboard) to visualize.
 */
void debug_bitmask_visualizer(U64 bitmask);

/**
 * @brief Visualizes precomputed attacks for knights and kings.
 * @note S: square of interest, X: attacked square.
 * @param square The square index (0-63) of interest.
 * @param type The piece type (Knight or King) to visualize attacks for.
 */
void debug_precomputed_attack_visualizer(int square, PieceType type);

/**
 * @brief Prints a move in human readable format, e.g. e2e4, g7g8q, etc.
 * @note Does not print annotations. Used in UCI output.
 * @param move The packed move to print.
 */
void debug_move(int move);

/**
 * @brief Prints a list of moves in human-readable format.
 * @param list Pointer to the MoveList to print.
 */
void debug_move_list(MoveList* list);

#endif // !DEBUG_H
