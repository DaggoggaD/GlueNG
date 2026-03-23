#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "Headers.h"
#define INFINITY 1000000

// Reset board status, bitboards and flags.
void unmake_move(Board* board, int move, int currDepth);

// Make the specified move. Updates all bitboards/arrays inside board struct.
void make_move(Board* board, int move, SelectionColor sideToMove, int currDepth);

// Performance test. (verified to depth 7)
U64 perft(Board* board, int depth);

// Same as perft, with specific info on any square.
U64 perft_divide(Board* board, int depth);

void generate_legal_moves(Board* board, MoveList* list);

int material_evaluation(Board* board, SelectionColor side);

int pst_evaluation(Board* board);

int evaluate(Board* board);

int nega_max(Board* board, int depth, int alpha, int beta);

int best_move(Board* board, int depth);

#endif // !MOVE_GENERATION_H
