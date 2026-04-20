/**
 * @file LookupTables.h
 * @brief Precomputed lookup tables for piece attacks and masks.
 */
#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "Headers.h"

 /**
  * @name Global Lookup Arrays
  * @brief Arrays storing precomputed attacks and occupancy masks.
  * @{
  */
extern U64 knightAttackSquares[64];
extern U64 kingAttackSquares[64];
extern U64 pawnSingleAttacks[PlayerN][64];
extern U64 rookOccupanciesMasks[64];
extern U64 bishopOccupanciesMasks[64];
/** @} */


/**
 * @name Knight Move Constants
 * @brief File masks used to prevent wrapping when generating Knight attacks.
 * @note From https://www.chessprogramming.org/Knight_Pattern
 * @{
 */
extern const U64 notAFile;
extern const U64 notHFile;
extern const U64 notABFile;
extern const U64 notGHFile;
/** @} */

/**
 * @brief Initializes pawn single attack squares. Called on startup.
 * @{
 */
void initialize_pawn_single_attack_squares();
void initialize_knight_attack_squares();
void initialize_king_attack_squares();
void initialize_rook_masks();
void initialize_bishop_masks();
/** @} */

#endif // !LOOKUP_TABLES_H
