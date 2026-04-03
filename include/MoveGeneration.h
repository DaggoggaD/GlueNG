#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "Headers.h"


// Reset board status, bitboards and flags.
void unmake_move(Board* board, int move, int currDepth);

// Make the specified move. Updates all bitboards/arrays inside board struct.
void make_move(Board* board, int move, SelectionColor sideToMove, int currDepth);

// Performance test. (verified to depth 7)
U64 perft(Board* board, int depth);

// Same as perft, with specific info on any square.
U64 perft_divide(Board* board, int depth);

void generate_legal_moves(Board* board, MoveList* list);

#endif // !MOVE_GENERATION_H
