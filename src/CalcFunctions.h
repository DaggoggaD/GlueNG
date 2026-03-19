#ifndef CALC_FUNCTIONS_H
#define CALC_FUNCTIONS_H

#include "Headers.h"

void index_from_rank_file(const int rank, const int file, int* index);

void rank_file_from_index(const int index, int* rank, int* file);

void add_piece_to_board(const PieceType piece, const SelectionColor color, int index, Board* board);

int chebyshev_distance(int square1, int square2);

#endif // !CALC_FUNCTIONS_H
