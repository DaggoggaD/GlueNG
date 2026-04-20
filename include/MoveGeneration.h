/**
 * @file MoveGeneration.h
 * @brief Function declarations for making/unmaking moves and generating legal moves.
 */
#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "Headers.h"


 /**
  * @brief Resets board status, bitboards and flags.
  * @param board Pointer to the current board.
  * @param move The packed move to unmake.
  * @param currDepth The current search depth.
  */
void unmake_move(Board* board, int move, int currDepth);

/**
 * @brief Makes the specified move. Updates all bitboards/arrays inside the board struct.
 * @param board Pointer to the current board.
 * @param move The packed move to make.
 * @param sideToMove The color making the move.
 * @param currDepth The current search depth.
 */
void make_move(Board* board, int move, SelectionColor sideToMove, int currDepth);

/**
 * @brief Performance test.
 * @note Verified to depth 9.
 * @param board Pointer to the current board.
 * @param depth The target depth to reach.
 * @return The total number of leaf nodes (positions) calculated.
 */
U64 perft(Board* board, int depth);

/**
 * @brief Same as perft, with specific info on any square.
 * @param board Pointer to the current board.
 * @param depth The target depth to reach.
 * @return The total number of leaf nodes calculated.
 */
U64 perft_divide(Board* board, int depth);

/**
 * @brief Generates all pseudo legal moves and filters out the illegal ones.
 * @param board Pointer to the current board.
 * @param list Pointer to the MoveList to fill with legal moves.
 */
void generate_legal_moves(Board* board, MoveList* list);

#endif // !MOVE_GENERATION_H
