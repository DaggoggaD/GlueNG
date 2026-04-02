#ifndef DEBUG_H
#define DEBUG_H

#include "Headers.h"

typedef struct MoveList MoveList;

// Used to translate from enum to string to visualize pieces.
void debug_select_string_from_piece(const PieceId piece, char* str);

// Prints current checkerboard pieces, 
// side to move, castling permissions, and halfmove.
void debug_board_visualizer(Board* board);

// Visualizes any bitmask as an 8x8 grid.
// X: bit is set, . otherswise.
void debug_bitmask_visualizer(U64 bitmask);

// Visualizes precomputed attacks for knights and kings.
// S: square of interest, X: attacked square.
void debug_precomputed_attack_visualizer(int square, PieceType type);

// Prints a move in human readable format, e.g. e2e4, g7g8q, etc.
// Does not print annotations. Used in UCI output.
void debug_move(int move);

// Prints a list of moves in human readable format.
void debug_move_list(MoveList* list);

#endif // !DEBUG_H
