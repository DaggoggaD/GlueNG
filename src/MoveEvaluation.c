#include "MoveEvaluation.h"

bool timeOut = false;
clock_t targetTime = 0;
U64 nodesCalculated = 0;

// Standard piece values
const int pieceValues[] = {
	100,   // PAWN
	350,   // KNIGHT
	350,   // BISHOP
	525,   // ROOK
	1000,  // QUEEN
	10000  // KING
};

// Standard Piece-square Tables from https://www.chessprogramming.org/Simplified_Evaluation_Function
const int pawnValueTable[64] = {
	0,  0,  0,  0,  0,  0,  0,  0,
	50, 50, 50, 50, 50, 50, 50, 50,
	10, 10, 20, 30, 30, 20, 10, 10,
	5,  5, 10, 25, 25, 10,  5,  5,
	0,  0,  0, 20, 20,  0,  0,  0,
	5, -5,-10,  0,  0,-10, -5,  5,
	5, 10, 10,-20,-20, 10, 10,  5,
	0,  0,  0,  0,  0,  0,  0,  0
};

const int knightValueTable[64] = {
	-50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  0, 10, 15, 15, 10,  0,-30,
	-30,  5, 15, 20, 20, 15,  5,-30,
	-30,  0, 15, 20, 20, 15,  0,-30,
	-30,  5, 10, 15, 15, 10,  5,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-30,-30,-30,-30,-40,-50,
};

const int bishopValueTable[64] = {
	-20,-10,-10,-10,-10,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0, 10, 10, 10, 10,  0,-10,
	-10, 10, 10, 10, 10, 10, 10,-10,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-20,-10,-10,-10,-10,-10,-10,-20,
};

const int rooksValueTable[64] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  5, 10, 10, 10, 10, 10, 10,  5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	  0,  0,  0,  5,  5,  0,  0,  0
};

const int queenValueTable[64] = {
	-20,-10,-10, -5, -5,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5,  5,  5,  5,  0,-10,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	  0,  0,  5,  5,  5,  5,  0, -5,
	-10,  5,  5,  5,  5,  5,  0,-10,
	-10,  0,  5,  0,  0,  0,  0,-10,
	-20,-10,-10, -5, -5,-10,-10,-20
};

const int kingStartValueTable[64] = {
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-20,-30,-30,-40,-40,-30,-30,-20,
	-10,-20,-20,-20,-20,-20,-20,-10,
	20, 20,  0,  0,  0,  0, 20, 20,
	20, 30, 10,  0,  0, 10, 30, 20
};

const int kingEndValueTable[64] = {
	-50,-40,-30,-20,-20,-30,-40,-50,
	-30,-20,-10,  0,  0,-10,-20,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-30,  0,  0,  0,  0,-30,-30,
	-50,-30,-30,-30,-30,-30,-30,-50
};

const int opponentKingEndValueTable[64] = {
	50, 40, 30, 20, 20, 30, 40, 50,
	40, 30, 20, 10, 10, 20, 30, 40,
	30, 20, 10,  0,  0, 10, 20, 30,
	20, 10,  0,  0,  0,  0, 10, 20,
	20, 10,  0,  0,  0,  0, 10, 20,
	30, 20, 10,  0,  0, 10, 20, 30,
	40, 30, 20, 10, 10, 20, 30, 40,
	50, 40, 30, 20, 20, 30, 40, 50
};

// Generation helpers

static inline void generate_pseudo_moves(Board* board, MoveList* list) {
	if (board->sideToMove == WHITE) white_generate_pseudo_moves(board, list);
	else black_generate_pseudo_moves(board, list);
}

// --- Move evaluation ---

static inline int mirror_index(int index) {
	return index ^ 56;
}

int material_evaluation(Board* board, SelectionColor side, int* noPawnEval) {
	int whiteMaterial = 0;
	int blackMaterial = 0;

	whiteMaterial += (int)__popcnt64(board->pieceBitboards[WHITE][KNIGHT]) * pieceValues[KNIGHT];
	whiteMaterial += (int)__popcnt64(board->pieceBitboards[WHITE][BISHOP]) * pieceValues[BISHOP];
	whiteMaterial += (int)__popcnt64(board->pieceBitboards[WHITE][ROOK]) * pieceValues[ROOK];
	whiteMaterial += (int)__popcnt64(board->pieceBitboards[WHITE][QUEEN]) * pieceValues[QUEEN];

	blackMaterial += (int)__popcnt64(board->pieceBitboards[BLACK][KNIGHT]) * pieceValues[KNIGHT];
	blackMaterial += (int)__popcnt64(board->pieceBitboards[BLACK][BISHOP]) * pieceValues[BISHOP];
	blackMaterial += (int)__popcnt64(board->pieceBitboards[BLACK][ROOK]) * pieceValues[ROOK];
	blackMaterial += (int)__popcnt64(board->pieceBitboards[BLACK][QUEEN]) * pieceValues[QUEEN];
	
	*noPawnEval = whiteMaterial + blackMaterial;

	int score = whiteMaterial - blackMaterial;

	score += (int)__popcnt64(board->pieceBitboards[WHITE][PAWN]) * pieceValues[PAWN];
	score -= (int)__popcnt64(board->pieceBitboards[BLACK][PAWN]) * pieceValues[PAWN];


	return (side == WHITE) ? score : -score;
}

static inline bool is_endgame(int noPawnEval) {
	return (noPawnEval < END_GAME_SCORE) ? true : false;
}

// From white's perspective!
int pst_evaluation(Board* board, int noPawnEval) {
	int score = 0;


	U64 wKnight = board->pieceBitboards[WHITE][KNIGHT];
	while (wKnight) {
		int index = get_lsb_index(wKnight);
		score += knightValueTable[index];

		wKnight &= wKnight - 1;
	}

	U64 wBishop = board->pieceBitboards[WHITE][BISHOP];
	while (wBishop) {
		int index = get_lsb_index(wBishop);
		score += bishopValueTable[index];

		wBishop &= wBishop - 1;
	}

	U64 wRook = board->pieceBitboards[WHITE][ROOK];
	while (wRook) {
		int index = get_lsb_index(wRook);
		score += rooksValueTable[index];

		wRook &= wRook - 1;
	}

	U64 wQueen = board->pieceBitboards[WHITE][QUEEN];
	while (wQueen) {
		int index = get_lsb_index(wQueen);
		score += queenValueTable[index];

		wQueen &= wQueen - 1;
	}

	U64 wPawn = board->pieceBitboards[WHITE][PAWN];
	while (wPawn) {
		int index = get_lsb_index(wPawn);
		score += pawnValueTable[index];

		wPawn &= wPawn - 1;
	}



	// Black side

	U64 bKnight = board->pieceBitboards[BLACK][KNIGHT];
	while (bKnight) {
		int index = get_lsb_index(bKnight);
		score -= knightValueTable[mirror_index(index)];
		bKnight &= bKnight - 1;
	}

	U64 bBishop = board->pieceBitboards[BLACK][BISHOP];
	while (bBishop) {
		int index = get_lsb_index(bBishop);
		score -= bishopValueTable[mirror_index(index)];
		bBishop &= bBishop - 1;
	}

	U64 bRook = board->pieceBitboards[BLACK][ROOK];
	while (bRook) {
		int index = get_lsb_index(bRook);
		score -= rooksValueTable[mirror_index(index)];
		bRook &= bRook - 1;
	}

	U64 bQueen = board->pieceBitboards[BLACK][QUEEN];
	while (bQueen) {
		int index = get_lsb_index(bQueen);
		score -= queenValueTable[mirror_index(index)];
		bQueen &= bQueen - 1;
	}

	U64 bPawn = board->pieceBitboards[BLACK][PAWN];
	while (bPawn) {
		int index = get_lsb_index(bPawn);
		score -= pawnValueTable[mirror_index(index)];
		bPawn &= bPawn - 1;
	}

	// If material is low, use endgame king tables.
	bool endgamePhase = is_endgame(noPawnEval);


	// Kings evaluations
	U64 bKing = board->pieceBitboards[BLACK][KING];
	int index = get_lsb_index(bKing);

	if (endgamePhase) score -= kingEndValueTable[mirror_index(index)];
	else score -= kingStartValueTable[mirror_index(index)];


	U64 wKing = board->pieceBitboards[WHITE][KING];
	index = get_lsb_index(wKing);

	if (endgamePhase) score += kingEndValueTable[index];
	else score += kingStartValueTable[index];

	return score;
}

int evaluate(Board* board) {
	int score = 0;
	int noPawnEval = 0;

	score += material_evaluation(board, WHITE, &noPawnEval);
	score += pst_evaluation(board, noPawnEval);


	if (is_endgame(noPawnEval)) {
		int distance = manhattan_distance(board->kingSq[WHITE], board->kingSq[BLACK]);
		int bonus = (14 - distance) * KING_CLOSENESS_BIAS;

		if (score > KING_BIAS_SCORE) {
			int opponentToBorder = opponentKingEndValueTable[board->kingSq[BLACK]];
			bonus += opponentToBorder;

			score += bonus;
		}
		else if (score < -KING_BIAS_SCORE) {
			int opponentToBorder = opponentKingEndValueTable[board->kingSq[WHITE]];
			bonus += opponentToBorder;
			score -= bonus;
		}
	}
	return (board->sideToMove == WHITE) ? score : -score;
}

// --- Minimax ---

// Both these functions are extremely similar. They could probably
// be merged, but it's probably faster and (way) clearer like this.

// At the end of the main search, it runs a deeper one till there are no more 
// captures/promotions/enpassants.
int quiescence_search(Board* board, int alpha, int beta, int ply) {
	if (ply > MAX_PLY - 1) return evaluate(board);

	nodesCalculated++;
	if ((nodesCalculated & 2047) == 0) {
		if (clock() > targetTime) {
			timeOut = true;
		}
	}

	if (timeOut) return 0;

	int standPat = evaluate(board);

	int bestVal = standPat;

	// If current player skips it's turn, and the evaluation is
	// still very good, return immediatly.
	if (bestVal >= beta) return bestVal;

	// Otherwise, switch alpha
	if (bestVal > alpha) alpha = bestVal;

	MoveList list = { 0 };
	generate_pseudo_moves(board, &list);

	order_moves(board, &list, 0);

	// Standard move iteration, checking only capture moves
	for (int i = 1; i < list.count; i++)
	{
		int move = list.moves[i];

		int toSq = GET_MOVE_TARGET(move);
		PieceType captured = board->pieceOnSquare[toSq];

		// Skip if not a capture
		if (captured == NONE && GET_MOVE_EN_PASSANT(move) == 0 && GET_PROMOTION_PIECE(move) == 0) continue;

		SelectionColor sideMoved = board->sideToMove;

		make_move(board, move, sideMoved, ply);

		if (is_square_attacked(board, board->kingSq[sideMoved], board->sideToMove)) {
			unmake_move(board, move, ply);
			continue;
		}

		int score = -quiescence_search(board, -beta, -alpha, ply + 1);
		unmake_move(board, move, ply);

		// Alpha beta checking
		if (score >= beta) return score;
		if (score > bestVal) bestVal = score;
		if (score > alpha) alpha = score;

	}

	return bestVal;
}

// Explores the moves tree, searching for the best eval.
// Returns (int) best score.
int nega_max(Board* board, int depth, int alpha, int beta, int extension, int ply) {
	nodesCalculated++;
	if ((nodesCalculated & 2047) == 0) {
		if (clock() > targetTime) {
			timeOut = true;
		}
	}

	// Transp table checking: if we already explored this position,
	// and the stored depth is higher than the current one, 
	// we can return the stored evaluation.
	int originalAlpha = alpha;
	TranspEntry* entry = &TT[get_hash_index(board->hashKey)];
	int ttMove = 0;


	if (entry->key == board->hashKey) {
		ttMove = entry->bestMove;

		if (entry->depth >= depth) {
			if (entry->flag == EXACT) return entry->eval;
			if (entry->flag == ALPHA && entry->eval <= alpha) return entry->eval;
			if (entry->flag == BETA && entry->eval >= beta) return entry->eval;
		}
	}

	if (timeOut) return 0;

	if (depth == 0) return quiescence_search(board, alpha, beta, ply);

	int best = -INFINITY;
	int legalMoves = 0;
	MoveList list = { 0 };

	// Used to update the best position in this scenario, for the transposition table.
	// If beta cut happens, it will be ignored, since the opponent will never pick it.
	int bestMoveInPosition = 0;
	
	generate_pseudo_moves(board, &list);

	// Add priority to bestMove from the TT, to improve alpha-beta pruning.
	order_moves(board, &list, ttMove);

	for (int i = 0; i < list.count; i++) {
		int move = list.moves[i];

		SelectionColor sideMoved = board->sideToMove;

		// Make the move
		make_move(board, move, sideMoved, ply);

		
		U64 realKey = get_zobrist_code(board);
		

		// Check if square is attacked. The opponent is now board->sideToMove,
		// since make_move changes opponent side.
		if (is_square_attacked(board, board->kingSq[sideMoved], board->sideToMove)) {

			// The move is illegal: unmake move, without increasing the counter.
			unmake_move(board, move, ply);
			continue;
		}

		legalMoves++;


		int currentExtension = extension < EXTENSION_DEEPNESS &&
			is_square_attacked(board, board->kingSq[board->sideToMove], sideMoved) ? 1 : 0;

		// From our point of view, the lower depth is in the opponent
		// reference: an high score for him is a terrible score for us.
		int score = -nega_max(board, depth - 1 + currentExtension, -beta, -alpha, extension + currentExtension, ply + 1);
		unmake_move(board, move, ply);

		if (timeOut) return 0;

		// Save score
		if (score > best) {
			best = score;
			bestMoveInPosition = move;
		}

		// Alpha-beta pruning

		// Better move found, store new alpha
		if (best > alpha) {
			alpha = best;
		}

		// Move too good, opponent would pick any previous branch.
		if (alpha >= beta) {
			store_tt_entry(board->hashKey, depth, best, BETA, bestMoveInPosition);
			return best;
		}

	}

	// If we didn't find any legal moves, it's either a checkmate (-infinity)
	// or a draw.
	if (legalMoves == 0) {
		SelectionColor otherSide = 1 - board->sideToMove;

		if (is_square_attacked(board, board->kingSq[board->sideToMove], otherSide)) {
			return -INFINITY - depth;
		}
		else {
			return 0;
		}
	}

	if (timeOut) return 0;

	HashFlag flag;
	if (best <= originalAlpha) {
		flag = ALPHA;
	}
	else {
		flag = EXACT;
	}

	store_tt_entry(board->hashKey, depth, best, flag, bestMoveInPosition);

	return best;
}

// Helps massively alpha-beta pruning, by ordering the moves 
// according to their scores.
void order_moves(Board* board, MoveList* list, int ttMove) {

	// Optimization for later: add both white and black attack squares bitboard,
	// to penalize moving into an attacked square

	for (int i = 0; i < list->count; i++) {
		int move = list->moves[i];
		int moveScore = 0;

		if (ttMove == move) {
			list->scores[i] = 10000000;
			continue;
		}

		// Unpack move
		int toSq = GET_MOVE_TARGET(move);
		PieceType piece = GET_MOVE_PIECE(move);
		PieceType captured = board->pieceOnSquare[toSq];
		PieceType promoted = GET_PROMOTION_PIECE(move);

		__assume(piece >= 0 && piece <= 5);
		__assume(captured >= 0 && captured <= 5);
		__assume(promoted >= 0 && promoted <= 5);

		// Positive feedback for capturing an opponent's piece
		if (captured != NONE) {
			moveScore += 10 * pieceValues[captured] - pieceValues[piece];
		}

		// A promotion notably increases the value
		if (promoted != 0) {
			moveScore += pieceValues[promoted];
		}

		// Penalize for moving into occupied square
		// See optimization above.
		/*if (is_square_attacked(board, toSq, 1 - board->sideToMove)) {
			moveScore -= pieceValues[piece];
		}*/

		list->scores[i] = moveScore;
	}

	// Selection sort the moves
	for (int i = 0; i < list->count - 1; i++) {
		int max_idx = i;
		for (int j = i + 1; j < list->count; j++) {
			if (list->scores[j] > list->scores[max_idx]) {
				max_idx = j;
			}
		}
		if (max_idx != i) {
			int tempScore = list->scores[i];
			list->scores[i] = list->scores[max_idx];
			list->scores[max_idx] = tempScore;

			int tempMove = list->moves[i];
			list->moves[i] = list->moves[max_idx];
			list->moves[max_idx] = tempMove;
		}
	}
}

// Explores the moves tree, with negamax. NOTE:
// it returns the (int) best move, NOT score.
int best_move(Board* board, int depth, int currBest, int* outScore) {

	// Here, for alpha-beta pruning, alpha represents our worst (pickable) scenario,
	// bete the best one the opponent would give us.
	int alpha = -INFINITY;
	int beta = INFINITY;

	int best = -INFINITY;
	int bestMove = 0;
	int legalMoves = 0;

	MoveList list = { 0 };

	// Generate pseudo legal moves
	generate_pseudo_moves(board, &list);

	order_moves(board, &list, currBest);

	for (int i = 0; i < list.count; i++) {
		int move = list.moves[i];

		SelectionColor sideMoved = board->sideToMove;

		// Make the move
		make_move(board, move, sideMoved, 0);

		// Check if square is attacked. The opponent is now board->sideToMove,
		// since make_move changes opponent side.
		if (is_square_attacked(board, board->kingSq[sideMoved], board->sideToMove)) {

			// The move is illegal: unmake move, without increasing the counter.
			unmake_move(board, move, 0);
			continue;
		}

		legalMoves++;
		int extension = is_square_attacked(board, board->kingSq[board->sideToMove], sideMoved) ? 1 : 0;

		// From our point of view, the lower depth is in the opponent
		// reference: an high score for him is a terrible score for us.
		int score = -nega_max(board, depth - 1 + extension, -beta, -alpha, extension, 1);
		unmake_move(board, move, 0);

		if (timeOut) break;

		// Save score
		// Bug fix: legalMoves == 1 is needed, as if it sees too long
		// in the future, and there is no possible way out of checkmate,
		// it will decide not to move any piece (a1a1) as none can deliver
		// decent performance.
		if (legalMoves == 1 || score > best) {
			best = score;
			bestMove = move;
		}


		if (best > alpha) {
			alpha = best;
		}
	}

	*outScore = best;

	if(!timeOut) store_tt_entry(board->hashKey, depth, best, EXACT, bestMove);

	return bestMove;
}

// Instead of running a single fixed depth search, runs at depth 1 ... max depth, or till
// timeout.
int best_move_iterative_deepening(Board* board, int maxTime, int maxDepth) {

	timeOut = false;
	nodesCalculated = 0;

	int bestMove = 0;
	int searchedScore = -INFINITY;
	clock_t startTime = clock();
	clock_t maxTicks = ((clock_t)maxTime * CLOCKS_PER_SEC) / 1000;
	targetTime = startTime + maxTicks;

	for (int i = 1; i < maxDepth; i++)
	{
		int currBestScore = -INFINITY - 1;
		int currBest = best_move(board, i, bestMove, &currBestScore);

		if (timeOut) {
			if (bestMove == 0) bestMove = currBest;
			break;
		}
		bestMove = currBest;
		searchedScore = currBestScore;


		// UCI info
		clock_t currentTime = clock();
		int timeElapsedMs = (int)((currentTime - startTime) * 1000 / CLOCKS_PER_SEC);
		if (timeElapsedMs == 0) timeElapsedMs = 1;

		printf("info depth %d score cp %d nodes %lld time %d\n", i, searchedScore, nodesCalculated, timeElapsedMs);
		fflush(stdout);

	}
	return bestMove;
}