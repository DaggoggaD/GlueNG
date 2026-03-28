#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "Headers.h"
#define INFINITY 1000000
#define END_GAME_SCORE 2000
#define KING_BIAS_SCORE 200
#define KING_CLOSENESS_BIAS 10
#define EXTENSION_DEEPNESS 16
#define MAX_PLY 50

// Reset board status, bitboards and flags.
void unmake_move(Board* board, int move, int currDepth);

// Make the specified move. Updates all bitboards/arrays inside board struct.
void make_move(Board* board, int move, SelectionColor sideToMove, int currDepth);

// Performance test. (verified to depth 7)
U64 perft(Board* board, int depth);

// Same as perft, with specific info on any square.
U64 perft_divide(Board* board, int depth);

void generate_legal_moves(Board* board, MoveList* list);

int material_evaluation(Board* board, SelectionColor side, int* noPawnEval);

int pst_evaluation(Board* board, int noPawnEval);

int evaluate(Board* board);

int nega_max(Board* board, int depth, int alpha, int beta, int extension);

void order_moves(Board* board, MoveList* list, int currBest);

int best_move(Board* board, int depth, int currBest, int extension);

int best_move_iterative_deepening(Board* board, int maxTime, int maxDepth);

#endif // !MOVE_GENERATION_H
