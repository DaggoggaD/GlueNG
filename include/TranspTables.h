/**
 * @file TranspTables.h
 * @brief Definitions and functions for Zobrist hashing and the Transposition Table.
 */
#pragma once
#ifndef TRANSP_TABLES
#define TRANSP_TABLES
#include "Headers.h"


 /**
  * @brief Bit rotation, making sure there are no left shifting losses.
  */
#define rot(x,k) (((x)<<(k))|((x)>>(64-(k))))

  /**
   * @name Zobrist Keys
   * @brief Arrays holding the random 64-bit numbers used to hash the board.
   * @{
   */
extern U64 pieceKeys[2][6][64];	/**< [color][piece][square] */
extern U64 sideKey;				/**< SideToMove info */
extern U64 castleKeys[16];		/**< 16 = 2^4 (all castle combinations) */
extern U64 enPassantKeys[8];	/**< enPassant for each file */
/** @} */

/**
 * @struct ranctx
 * @brief Context structure for the RKISS random number generator.
 * @note RKISS Implementation at https://www.chessprogramming.org/Bob_Jenkins#RKISS
 */
typedef struct ranctx {
	U64 a; U64 b; U64 c; U64 d; 
} ranctx;

/**
 * @enum HashFlag
 * @brief Represents the type of score stored in the transposition table.
 */
typedef enum {
	EXACT,
	ALPHA,
	BETA
} HashFlag;

/**
 * @struct TranspEntry
 * @brief A single entry in the Transposition Table.
 */
typedef struct TranspEntry {
	U64 key;
	int depth;
	int eval;
	HashFlag flag;
	int bestMove;
	int ancient;
} TranspEntry;

extern TranspEntry* TT;
extern int current_age;
extern int ttSize;
extern ranctx rng;

/**
 * @brief Returns a random 64-bit value, used for Zobrist hashing.
 * @param x Pointer to the RNG context state.
 * @return A random 64-bit integer.
 */
U64 ran_val(ranctx* x);

/**
 * @brief Initializes the RNG.
 * @param x Pointer to the RNG context state.
 * @param seed The starting seed for the random number generator.
 */
void ran_init(ranctx* x, U64 seed);

/**
 * @brief Initializes the pieces, side, castle, and enPassant random keys.
 */
void init_random_keys();

/**
 * @brief Calculates the Zobrist code for a given board position.
 * @note This function is not actually used in the search, as it is inherently slow.
 * The actual hash key is updated incrementally in make_move/unmake_move.
 * @param board Pointer to the current board.
 * @return The complete Zobrist hash key for the position.
 */
U64 get_zobrist_code(Board* board);

/**
 * @brief Returns the index in the transposition table for a given hash key.
 * @param hashKey The full 64-bit Zobrist key.
 * @return The index to be used in the TT array.
 */
static inline int get_hash_index(U64 hashKey) { return hashKey % ttSize; }

/**
 * @brief Initializes the transposition table.
 * @note The size is given in MB. It is either passed from the command line
 * or set to a default value (#define).
 * @param sizeMB The desired size of the Transposition Table in megabytes.
 */
void init_tt(int sizeMB);

/**
 * @brief Stores an entry in the transposition table.
 * @note It does so if the entry is empty, from a different age,
 * or if the new entry has a higher depth than the existing one.
 * @param key The Zobrist hash key of the position.
 * @param depth The depth at which the position was searched.
 * @param eval The evaluation score of the position.
 * @param flag The node type flag (EXACT, ALPHA, or BETA).
 * @param bestMove The best move found for this position.
 */
void store_tt_entry(U64 key, int depth, int eval, HashFlag flag, int bestMove);

#endif // !TRANSP_TABLES
