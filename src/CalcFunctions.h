#ifndef CALC_FUNCTIONS_H
#define CALC_FUNCTIONS_H

#include "Headers.h"

// LSF (least significant file) translation to index
void index_from_rank_file(const int rank, const int file, int* index);

// LSF little endian rank and file translation from index
void rank_file_from_index(const int index, int* rank, int* file);

// Adds a piece to ALL relevant bitboards. Checks for valid index input.
void add_piece_to_board(const PieceType piece, const SelectionColor color, int index, Board* board);

// Chebyshev distance between two squares, for king move generation and evaluation.
int chebyshev_distance(int square1, int square2);

// Manhattan distance between two squares, for evaluation.
int manhattan_distance(int square1, int square2);

#endif // !CALC_FUNCTIONS_H
