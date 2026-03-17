#ifndef GLUE_NG_H
#define GLUE_NG_H

#include "Headers.h"

void index_from_rank_file(const int rank, const int file, int* index);

void rank_file_from_index(const int index, int* rank, int* file);

void add_piece_to_board(const PieceType piece, const SelectionColor color, int index, Board* board);

int chebyshev_distance(int square1, int square2);

// Returns a bitboard of knight attacks from a given square bitboard.
// NOTE: PSEUDO-LEGAL, does not check for pieces of the same color on the target square.
// nor king-king distance, or check conditions.
U64 knight_attacks_on_square(U64 b);
U64 king_attacks_from_square(int square);
U64 get_rook_attacks_from_square(int square, U64 blockers);
U64 get_bishop_attacks_from_square(int square, U64 blockers);
U64 get_queen_attacks_from_square(int square, U64 blockers);


#endif // !GLUE_NG_H
