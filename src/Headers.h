#ifndef HEADERS_H
#define HEADERS_H
#define _CRT_SECURE_NO_WARNINGS

#define INF 1000000
#define END_GAME_SCORE 2000
#define KING_BIAS_SCORE 200
#define KING_CLOSENESS_BIAS 10
#define EXTENSION_DEEPNESS 16
#define MAX_PLY 50
#define STANDARD_HASH_SIZE_MB 64
#define BRD_SIZE 64

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
