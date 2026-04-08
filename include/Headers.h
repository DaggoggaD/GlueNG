#ifndef HEADERS_H
#define HEADERS_H
#define _CRT_SECURE_NO_WARNINGS

// Game constants
#define INF 1000000
#define EXTENSION_DEEPNESS 16
#define MAX_PLY 50 // For quiescence search, not the main search. The main search is limited by MAX_TURNS.
#define MAX_TURNS 100
#define STANDARD_HASH_SIZE_MB 64
#define BRD_SIZE 64

// Evaluation constants
#define END_GAME_SCORE 2000
#define LAZY_EVAL_MARGIN 150
#define KING_BIAS_SCORE 200
#define KING_CLOSENESS_BIAS 10
#define DOUBLED_PAWN_PENALTY 10
#define ISOLATED_PAWN_PENALTY 15
#define PASSED_PAWN_BONUS 50
#define KNIGHT_MOBILITY_BONUS 4
#define BISHOP_MOBILITY_BONUS 3
#define ROOK_MOBILITY_BONUS 1

#include "ChessDefinitions.h"
#include "ChessStructs.h"
#include "FenFunctions.h"
#include "Debug.h"
#include "LookUpTables.h"
#include "MagicBitboards.h"
#include "PseudoMovesCalc.h"
#include "CalcFunctions.h"
#include "GlueNG.h"
#include "MoveGeneration.h"
#include "MoveEvaluation.h"
#include "TranspTables.h"
#include "UCI.h"

#include <math.h>

#include <string.h>
#include <inttypes.h>
#include <intrin.h>
#include <time.h>

#endif // !HEADERS_H
