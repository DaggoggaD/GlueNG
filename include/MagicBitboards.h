/**
 * @file MagicBitboards.h
 * @brief Definitions and functions for sliding piece move generation using Magic Bitboards.
 * * @note Learnt from:
 * - https://analog-hors.github.io/site/magic-bitboards/
 * - https://www.chessprogramming.org/Magic_Bitboards
 */

#ifndef MAGIC_BITBOARDS_H
#define MAGIC_BITBOARDS_H

#include "Headers.h"

 /**
  * @struct mEntry
  * @brief Represents a single magic entry for a specific square and piece type.
  */
typedef struct mEntry {
	U64 occupanciesMask;
	U64 magicN;
	unsigned short indexBits;
} MagicEntry;

/**
 * @name Global Magic Arrays
 * @brief Precomputed tables containing magic entries and actual move bitboards.
 * @{
 */
extern MagicEntry RookMagics[BRD_SIZE];
extern MagicEntry BishopMagics[BRD_SIZE];
extern U64 RookMoves[BRD_SIZE][4096];
extern U64 BishopMoves[BRD_SIZE][512];
/** @} */


/**
 * @brief Given a blocker configuration and a magic entry, it calculates the magic index
 * by applying the magic multiplication and the appropriate shift.
 * * @param b Blocker configuration bitboard.
 * @param entry Magic entry containing the occupancy mask, the magic number and the index bits.
 * @return The calculated magic index.
 */
int get_magic_index(U64 b, MagicEntry* entry);

/**
 * @brief Counts the number of all active bits in a bitboard.
 * * @note Could use __popcnt64. However, it is only called
 * during initialization, so it is not a performance concern.
 * * @param b The bitboard to evaluate.
 * @return The number of active bits.
 */
int count_active_bits(U64 b);

/**
 * @brief Given a mask and a patternID, it sets the bits in the occipancy
 * bitboard according to the active bits in the mask and the patternID.
 * * @param mask Pre-calculated bitboard, all possible positions without blockers and edges.
 * @param patternID Number from 0 to 2^(active bits in the mask) - 1, used to set the
 * corresponding bits in the occupancy bitboard.
 * @return The constructed occupancy bitboard.
 */
U64 set_occupancies_mapping(U64 mask, int patternID);

/**
 * @brief Calculates all the attack rays for a rook, starting on the given square,
 * and with the given blocker configuration. It also counts the blocker squares as attacked.
 * * @param square Rook position, 0-63 index.
 * @param blockers Bitboard with blocker positions.
 * @return The bitboard containing the rook's attack rays.
 */
U64 calc_rook_attack_rays(int square, U64 blockers);

/**
 * @brief Same as "calc_rook_attack_rays", but for bishops.
 * * @param square Bishop position, 0-63 index.
 * @param blockers Bitboard with blocker positions.
 * @return The bitboard containing the bishop's attack rays.
 */
U64 calc_bishop_attack_rays(int square, U64 blockers);

/**
 * @brief Initializes the rook, bishop and queen magic entries and attack tables.
 * * @note Could easily be optimized by merging the loops, but it is clearer to keep them separated,
 * and the initialization is only done once on startup, so it is not a problem.
 */
void initialize_magic_bitboards();

#endif // !MAGIC_BITBOARDS_H
