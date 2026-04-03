#pragma once
#ifndef TRANSP_TABLES
#include "Headers.h"


// Bit rotation, making sure no left shifting losses
#define rot(x,k) (((x)<<(k))|((x)>>(64-(k))))

// RKISS Implementation at https://www.chessprogramming.org/Bob_Jenkins#RKISS
U64 pieceKeys[2][6][64]; // [color][piece][square]
U64 sideKey;             // SideToMove info
U64 castleKeys[16];      // 16 = 2^4 (all castle combinations)
U64 enPassantKeys[8];    // enPassant for each file

typedef struct ranctx {
	U64 a; U64 b; U64 c; U64 d; 
} ranctx;

typedef enum {
	EXACT,
	ALPHA,
	BETA
} HashFlag;

typedef struct {
	U64 key;
	int depth;
	int eval;
	HashFlag flag;
	int bestMove;
	int ancient; // see later
} TranspEntry;

extern TranspEntry* TT;
extern int current_age;
extern int ttSize;
ranctx rng;

// Returns a random 64-bit value, used for Zobrist hashing.
U64 ran_val(ranctx* x);

// Initialize rng
void ran_init(ranctx* x, U64 seed);

// Initialize pieces/side/castle/enPassant keys
void init_random_keys();

// Calculates the Zobrist code for a given board position. Note that
// this function is not actually used in the search, as it is inherently slow.
// The actual hash key is updated incrementally in make_move/unmake_move.
U64 get_zobrist_code(Board* board);

// Returns the index in the transposition table for a given hash key.
static inline int get_hash_index(U64 hashKey) { return hashKey % ttSize; }

// Initializes the transposition table, size given in MB.
// Either passed from command line or set to default value (#define).
void init_tt(int sizeMB);

// Stores an entry in the transposition table.
// Does so if the entry is empty, from a different age, 
// or if the new entry has higher depth than the existing one.
void store_tt_entry(U64 key, int depth, int eval, HashFlag flag, int bestMove);

#endif // !TRANSP_TABLES
