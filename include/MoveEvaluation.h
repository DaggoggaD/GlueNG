#pragma once
#ifndef MOVE_EVALUATION_H
#define MOVE_EVALUATION_H

#include "Headers.h"

extern int lmrTable[64][256];
extern U64 fileMasks[8];
extern U64 adjacentFileMasks[8];
extern U64 rowMasks[8];

// --- Move evaluation ---

// Initializes the fileMasks and adjacentFileMasks, used for pawn evaluation
void init_evaluation_masks();

// Generates pseudo-legal moves depending on the side to move
static inline void generate_pseudo_moves(Board* board, MoveList* list);

// The implementation is based on withe's perspective, 
// so the black pieces are evaluated using mirrored PSTs
static inline int mirror_index(int index);

// Evaluates current position based on material only.
// (Also calculates the noPawnEval, which is used to determine endgame phase)
int material_evaluation(Board* board, SelectionColor side, int* noPawnEval);

// Determines if the position is in endgame phase, based on the noPawnEval value.
static inline bool is_endgame(int noPawnEval);

// Evaluates the position based on the number of pseudo-legal moves for each piece type.
int mobility_evaluation(Board* board, SelectionColor side, int* noPawnEval);

// Evaluates current position based on piece position,
// using piece-square tables. Also considers endgame phase for king evaluation.
// From white's perspective. 
// Does not consider mobility scores, as they are heavier, so it's done only if
// the position is close to beta.
int pst_lazy_evaluation(Board* board, int noPawnEval);


// Evaluates the position based on the pawn structure, 
// using file and row masks.
int pawn_evaluation(Board* board);

// Evaluates the current position, by summing material 
// and piece-square tables evaluations. Also adds a bonus 
// in endgame phase, based on the distance between the kings and
// the opponent king distance to the border.
int evaluate(Board* board, int alpha, int beta);


// --- Move search ---

//Initializes the late move reduction table, to avoid log calculations
// at runtime. The reduction alghorithm is based on
// the obsidian implementation: 
// https://www.chessprogramming.org/Late_Move_Reductions#Reduction_Depth
void init_lmr_table();

// Makes a null move, by switching the side to move and increasing the ply.
// Used for NMH (Null Move Heuristic) pruning.
void make_null_move(Board* board, int currDepth);

// Resets the board to the state before the null move, 
// by switching back the side to move and decreasing the ply.
void unmake_null_move(Board* board, int currDepth);

// Checks if the position is "heavy", meaning that it has a lot 
// of pieces on the board, which makes the null move heuristic more effective.
bool is_heavy_board(Board* board);

// Checks if the current position has already been reached in the game, 
// by checking history of positions.
bool is_repetition(Board* board, int ply);

// Helps massively alpha-beta pruning, by ordering the moves 
// according to their scores. (MVV - LVA + promotion bonus)
void order_moves(Board* board, MoveList* list, int ttMove, int ply);

// At the end of the main search, it runs a deeper one untill there are no more 
// captures/promotions/enpassants.
int quiescence_search(Board* board, int alpha, int beta, int ply);

// Explores the moves tree, searching for the best eval.
// Returns (int) best score.
int nega_max(Board* board, int depth, int alpha, int beta, int extension, bool allowNull);

// Explores the moves tree, with negamax. NOTE:
// it returns the (int) best move, NOT score.
int best_move(Board* board, int depth, int currBest, int extension);

// Instead of running a single fixed depth search, runs at depth 1 ... max depth, or till
// timeout.
int best_move_iterative_deepening(Board* board, int maxTime, int maxDepth);

#endif // !MOVE_EVALUATION_H