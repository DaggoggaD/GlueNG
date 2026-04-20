/**
* @file MoveEvaluation.h
* @brief This file contains the function declarations for move evaluation and search in the chess engine.*/
#pragma once
#ifndef MOVE_EVALUATION_H
#define MOVE_EVALUATION_H

#include "Headers.h"

/**@brief Precalculated table for late move reductions, to avoid log calculations at runtime.*/
extern int lmrTable[64][256];
/**@brief Precalculated masks for each file.*/
extern U64 fileMasks[8];
/**@brief Precalculated masks for adjacent files of a specific file (for pawn evaluation).*/
extern U64 adjacentFileMasks[8];
/**@brief Precalculated masks for each row.*/
extern U64 rowMasks[8];

/**@brief struct to represent the state of the search.*/
typedef struct {
	int depth;		/**<Remaining depth.*/
	int alpha;		/**<Lower search bound.*/
	int beta;		/**<Upper search bound.*/
	int extension;	/**<Search extension (when in check).*/
	int ply;		/**<Current half moves from search root.*/
	bool allowNull; /**<Flag to stop nested Null Moves Heuristics.*/
} SearchState;

typedef struct TranspEntry TranspEntry;

// ==============================================================================
// MOVE EVALUATION
// ==============================================================================

/**
 * @brief Initializes the fileMasks and adjacentFileMasks, used for pawn evaluation.
 */
void init_evaluation_masks();


/**
 * @brief Generates pseudo-legal moves depending on the side to move.
 * * @param board Pointer to the current board.
 * @param list Pointer to the move list where generated moves will be stored.
 */
static void generate_pseudo_moves(Board* board, MoveList* list);

/**
 * @brief The implementation is based on white's perspective, so the black pieces are evaluated using mirrored PSTs.
 * * @param index The original square index.
 * @return The mirrored index for the black pieces.
 */
static int mirror_index(int index);


/**
 * @brief Evaluates current position based on material only.
 * * (Also calculates the noPawnEval, which is used to determine endgame phase).
 * * @param board Pointer to the current.
 * @param side The color of the side to evaluate.
 * @param noPawnEval Pointer to an integer to store the material evaluation without pawns.
 * @return The material evaluation score.
 */
int material_evaluation(Board* board, SelectionColor side, int* noPawnEval);

/**
 * @brief Determines if the position is in endgame phase, based on the noPawnEval value.
 * * @param noPawnEval The material evaluation without pawns.
 * @return True if in the endgame phase, false otherwise.
 */
static bool is_endgame(int noPawnEval);

/**
 * @brief Evaluates the position based on the number of pseudo-legal moves for each piece type.
 * * @param board Pointer to the current board.
 * @param side The color of the side to evaluate.
 * @param noPawnEval Pointer to the material evaluation without pawns.
 * @return The mobility evaluation score.
 */
int mobility_evaluation(Board* board, SelectionColor side, int* noPawnEval);

/**
 * @brief Evaluates current position based on piece position, using piece-square tables.
 * * Also considers endgame phase for king evaluation. From white's perspective.
 * Does not consider mobility scores, as they are heavier, so it's done only if the position is close to beta.
 * Old, replaced with PeSTO evaluation.
 * * @param board Pointer to the current board.
 * @param noPawnEval The material evaluation without pawns (used to check endgame phase).
 * @return The static piece-square evaluation score.
 */
int pst_lazy_evaluation(Board* board, int noPawnEval);

/**
 * @brief Evaluates current position based on piece position.
 * * Uses the PeSTO evaluation tables, which take into account a more precise
 * evaluation (const tables taken from Chess Programming Wiki) and endgame phase.
 * * @param board Pointer to the current board.
 * @param phase The current game phase.
 * @return The PeSTO evaluation score in centipawns.
 */
int pesto_evaluation(Board* board, int phase);

/**
 * @brief Evaluates white pawns (passed, doubled, isolated), by checking the file and adjacent files.
 * * Helper function for pawn_evaluation.
 * * @param wPawn Bitboard representing white pawns.
 * @param bPawn Bitboard representing black pawns.
 * @param file The current file (0-7) being evaluated.
 * @param score Pointer to the score variable to update.
 */
void evaluate_white_passed_pawns(U64 wPawn, U64 bPawn, int file, int* score);

/**
 * @brief Evaluates black pawns (passed, doubled, isolated), by checking the file and adjacent files.
 * * Helper function for pawn_evaluation.
 * * @param wPawn Bitboard representing white pawns.
 * @param bPawn Bitboard representing black pawns.
 * @param file The current file (0-7) being evaluated.
 * @param score Pointer to the score variable to update.
 */
void evaluate_black_passed_pawns(U64 wPawn, U64 bPawn, int file, int* score);

/**
 * @brief Evaluates the pawn structure, by checking each file for doubled, isolated and passed pawns.
 * * @param board Pointer to the current board.
 * @param i The file index to search (0-7).
 * @param score Pointer to the score variable to update.
 * @param wPawn Bitboard representing white pawns.
 * @param bPawn Bitboard representing black pawns.
 */
void search_pawn_file(Board* board, int i, int* score, U64 wPawn, U64 bPawn);

/**
 * @brief Evaluates the position based on the pawn structure, using file and row masks.
 * * @param board Pointer to the current board.
 * @return The pawn structure evaluation score.
 */
int pawn_evaluation(Board* board);

/**
 * @brief Evaluates the position based on the position of nearby pawns, shielding the king, or exposing it.
 * * Tapered with game phase in evaluate().
 * * @param board Pointer to the current board.
 * @return The king safety evaluation score.
 */

int king_safety_evaluation(Board* board);

/**
 * @brief Moves the king closer in the endgame, making them a valuable attack piece.
 * * Adds a bonus for pushing the opponent king to the border.
 * * @param board Pointer to the current board.
 * @param score The current evaluation score.
 * @param phase The current game phase.
 * @return The adjusted score including the king distance bonus.
 */
int king_distance_evaluation(Board* board, int score, int phase);

/**
 * @brief Evaluates the current position, by summing material and piece-square tables evaluations.
 * * Also adds a bonus in endgame phase, based on the distance between the kings and
 * the opponent king distance to the border.
 * * @param board Pointer to the current board.
 * @param alpha Lower bound of the search window.
 * @param beta Upper bound of the search window.
 * @return The final evaluation score of the board.
 */
int evaluate(Board* board, int alpha, int beta);


// ==============================================================================
// Move search
// ==============================================================================

/**
 * @brief Initializes the late move reduction table, to avoid log calculations at runtime.
 * * The reduction algorithm is based on the obsidian implementation:
 * https://www.chessprogramming.org/Late_Move_Reductions#Reduction_Depth
 */
void init_lmr_table();

/**
 * @brief Makes a null move, by switching the side to move and increasing the ply.
 * * Used for NMH (Null Move Heuristic) pruning.
 * * @param board Pointer to the current board.
 * @param currDepth The current search depth.
 */
void make_null_move(Board* board, int currDepth);

/**
 * @brief Resets the board to the state before the null move, by switching back the side to move and decreasing the ply.
 * * @param board Pointer to the current board.
 * @param currDepth The current search depth.
 */
void unmake_null_move(Board* board, int currDepth);

/**
 * @brief Checks if the position is "heavy", meaning that it has a lot of pieces on the board.
 * * This makes the null move heuristic more effective.
 * * @param board Pointer to the current board.
 * @return True if the board is considered heavy, false otherwise.
 */
bool is_heavy_board(Board* board);

/**
 * @brief Checks if the current position has already been reached in the game, by checking history of positions.
 * * @param board Pointer to the current board.
 * @param ply The current ply from the root.
 * @return True if a repetition is found, false otherwise.
 */
bool is_repetition(Board* board, int ply);

/**
 * @brief Helps massively alpha-beta pruning, by ordering the moves according to their scores.
 * * (MVV - LVA + promotion bonus)
 * * @param board Pointer to the current board.
 * @param list Pointer to the move list to be ordered.
 * @param ttMove A transposition table move to be prioritized.
 * @param ply The current ply from the root.
 */
void order_moves(Board* board, MoveList* list, int ttMove, int ply);

/**
 * @brief At the end of the main search, it runs a deeper one until there are no more captures/promotions/enpassants.
 * * @param board Pointer to the current board.
 * @param alpha Lower bound of the search window.
 * @param beta Upper bound of the search window.
 * @param ply The current ply from the root.
 * @return The quiescence evaluation score.
 */
int quiescence_search(Board* board, int alpha, int beta, int ply);

/**
 * @brief Checks if the current position is already stored in the transposition table.
 * * @param board Pointer to the current board.
 * @param entry Pointer to the Transposition Table entry.
 * @param depth The current search depth.
 * @param alpha Lower bound of the search window.
 * @param beta Upper bound of the search window.
 * @param outScore Pointer to store the score retrieved from the table.
 * @return True if a valid TT entry was found, false otherwise.
 */
bool probe_tt(Board* board, TranspEntry* entry, int depth, int alpha, int beta, int* outScore);

/**
 * @brief If the position is not in the transp table, we apply a null move (skipping our turn).
 * * If the position is still in our favor (score >= beta), we can prune this branch.
 * * @param board Pointer to the current board.
 * @param entry Pointer to the Transposition Table entry.
 * @param depth The remaining search depth.
 * @param beta Upper bound of the search window.
 * @param extension Current depth extensions accumulated.
 * @param ply The current ply from the root.
 * @param outScore Pointer to store the result of the null move search.
 * @return True if a null move cutoff occurred, false otherwise.
 */
bool do_null_move_pruning(Board* board, TranspEntry* entry, int depth, int beta, int extension, int ply, int* outScore);

/**
 * @brief Evaluates a specific move, considering the current board state, search state, legal moves, and move list.
 * * @param board Pointer to the current board.
 * @param move The specific move to evaluate.
 * @param state Pointer to the current search state structure.
 * @param legalMoves Pointer to the count of legal moves found.
 * @param list Pointer to the current move list.
 * @return The evaluated score of the move.
 */
int evaluate_move(Board* board, int move, SearchState* state, int* legalMoves, MoveList* list);

/**
 * @brief Explores the moves tree, searching for the best eval.
 * * @param board Pointer to the current board state.
 * @param depth The remaining search depth.
 * @param alpha Lower bound of the search window.
 * @param beta Upper bound of the search window.
 * @param extension Current depth extensions accumulated.
 * @param ply The current ply from the root.
 * @param allowNull Flag to permit Null Move Heuristic.
 * @return The best score found in this branch.
 */
int nega_max(Board* board, int depth, int alpha, int beta, int extension, int ply, bool allowNull);

/**
 * @brief Explores the moves tree, with negamax.
 * * @param board Pointer to the current board state.
 * @param depth The remaining search depth.
 * @param currBest The current best score.
 * @param extension Current depth extensions accumulated.
 * @return The encoded best move as an integer.
 */
int best_move(Board* board, int depth, int currBest, int extension);

/**
 * @brief Instead of running a single fixed depth search, runs at depth 1 ... max depth, or till timeout.
 * * @param board Pointer to the current board state.
 * @param maxTime Maximum allocated time in milliseconds.
 * @param maxDepth The absolute maximum depth to reach.
 * @return The final best move found before timeout.
 */
int best_move_iterative_deepening(Board* board, int maxTime, int maxDepth);

#endif // !MOVE_EVALUATION_H