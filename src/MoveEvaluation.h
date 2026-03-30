#pragma once
#ifndef MOVE_EVALUATION_H
#define MOVE_EVALUATION_H

#include "Headers.h"

#define INFINITY 1000000
#define END_GAME_SCORE 2000
#define KING_BIAS_SCORE 200
#define KING_CLOSENESS_BIAS 10
#define EXTENSION_DEEPNESS 16
#define MAX_PLY 50

int material_evaluation(Board* board, SelectionColor side, int* noPawnEval);

int pst_evaluation(Board* board, int noPawnEval);

int evaluate(Board* board);

int nega_max(Board* board, int depth, int alpha, int beta, int extension);

void order_moves(Board* board, MoveList* list, int ttMove);

int best_move(Board* board, int depth, int currBest, int extension);

int best_move_iterative_deepening(Board* board, int maxTime, int maxDepth);


#endif // !MOVE_EVALUATION_H


