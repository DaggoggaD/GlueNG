/**
 * @file CalcFunctions.h
 * @brief Math utilities and index conversion calculations for the board.
 */
#ifndef CALC_FUNCTIONS_H
#define CALC_FUNCTIONS_H

#include "Headers.h"

 /**
  * @brief LSF (least significant file) translation to index.
  * @param rank The rank index.
  * @param file The file index.
  * @param index Pointer to the integer where the resulting square index (0-63) will be stored.
  */
void index_from_rank_file(const int rank, const int file, int* index);

/**
 * @brief LSF little endian rank and file translation from an index.
 * @param index The square index (0-63).
 * @param rank Pointer to the integer where the resulting rank will be stored.
 * @param file Pointer to the integer where the resulting file will be stored.
 */
void rank_file_from_index(const int index, int* rank, int* file);

/**
 * @brief Adds a piece to ALL relevant bitboards. Checks for valid index input.
 * @param piece The PieceType to add.
 * @param color The SelectionColor of the piece.
 * @param index The square index (0-63) where the piece will be placed.
 * @param board Pointer to the board state to update.
 */
void add_piece_to_board(const PieceType piece, const SelectionColor color, int index, Board* board);

/**
 * @brief Calculates the Chebyshev distance between two squares.
 * @note Used for king move generation and evaluation.
 * @param square1 The first square index (0-63).
 * @param square2 The second square index (0-63).
 * @return The Chebyshev distance (maximum of file difference and rank difference).
 */
int chebyshev_distance(int square1, int square2);

/**
 * @brief Calculates the Manhattan distance between two squares.
 * @note Used for evaluation logic.
 * @param square1 The first square index (0-63).
 * @param square2 The second square index (0-63).
 * @return The Manhattan distance (sum of file difference and rank difference).
 */
int manhattan_distance(int square1, int square2);

#endif // !CALC_FUNCTIONS_H
