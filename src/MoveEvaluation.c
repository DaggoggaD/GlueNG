#include "../include/MoveEvaluation.h"

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

const int passedPawnBonus[8] = { 0, 10, 20, 40, 70, 120, 200, 0 };

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

// Bitmaksk for pawn evaluation
U64 fileMasks[8];
U64 rowMasks[8];
U64 adjacentFileMasks[8];

// Late move reductions
int lmrTable[64][256];

// Killer moves
int killerMoves[MAX_TURNS][2];
int historyCutTable[2][64][64];

// --- Move evaluation ---

void init_evaluation_masks() {
	for (int i = 0; i < 8; i++) {
		fileMasks[i] = 0x0101010101010101ULL << i;
		rowMasks[i] = 0xFFULL << (i * 8);
	}

	for (int i = 0; i < 8; i++) {
		adjacentFileMasks[i] = 0;
		if (i > 0) adjacentFileMasks[i] |= fileMasks[i - 1];
		if (i < 7) adjacentFileMasks[i] |= fileMasks[i + 1];
	}
}

static inline void generate_pseudo_moves(Board* board, MoveList* list) {
	if (board->sideToMove == WHITE) white_generate_pseudo_moves(board, list);
	else black_generate_pseudo_moves(board, list);
}

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

int mobility_evaluation(Board* board) {
		int score = 0;
		U64 whiteOccupancy = board->occupiedBitboards[WHITE];
		U64 blackOccupancy = board->occupiedBitboards[BLACK];
		U64 bothOccupancy = board->occupiedBitboards[BOTH];

		// WHITE SIDE

		U64 knights = board->pieceBitboards[WHITE][KNIGHT];
		while (knights) {
			int sq = get_lsb_index(knights);
			score += (int)__popcnt64(get_knight_pseudo_moves(sq, whiteOccupancy)) * KNIGHT_MOBILITY_BONUS;
			knights &= knights - 1;
		}


		U64 bishops = board->pieceBitboards[WHITE][BISHOP];
		while (bishops) {
			int sq = get_lsb_index(bishops);
			score += (int)__popcnt64(get_bishop_pseudo_moves(sq, bothOccupancy, whiteOccupancy)) * BISHOP_MOBILITY_BONUS;
			bishops &= bishops - 1;
		}


		U64 rooks = board->pieceBitboards[WHITE][ROOK];
		while (rooks) {
			int sq = get_lsb_index(rooks);
			score += (int)__popcnt64(get_rook_pseudo_moves(sq, bothOccupancy, whiteOccupancy)) * ROOK_MOBILITY_BONUS;
			rooks &= rooks - 1;
		}

		// BLACK SIDE

		U64 bKnights = board->pieceBitboards[BLACK][KNIGHT];
		while (bKnights) {
			int sq = get_lsb_index(bKnights);
			score -= (int)__popcnt64(get_knight_pseudo_moves(sq, blackOccupancy)) * KNIGHT_MOBILITY_BONUS;
			bKnights &= bKnights - 1;
		}

		U64 bBishops = board->pieceBitboards[BLACK][BISHOP];
		while (bBishops) {
			int sq = get_lsb_index(bBishops);
			score -= (int)__popcnt64(get_bishop_pseudo_moves(sq, bothOccupancy, blackOccupancy)) * BISHOP_MOBILITY_BONUS;
			bBishops &= bBishops - 1;
		}


		U64 bRooks = board->pieceBitboards[BLACK][ROOK];
		while (bRooks) {
			int sq = get_lsb_index(bRooks);
			score -= (int)__popcnt64(get_rook_pseudo_moves(sq, bothOccupancy, blackOccupancy)) * ROOK_MOBILITY_BONUS;
			bRooks &= bRooks - 1;
		}


		return score;

}

int pst_lazy_evaluation(Board* board, int noPawnEval) {
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

int pawn_evaluation(Board* board) {
	int score = 0;
	
	U64 wPawn = board->pieceBitboards[WHITE][PAWN];
	U64 bPawn = board->pieceBitboards[BLACK][PAWN];

	for (int i = 0; i < 8; i++)
	{
		int wPop = (int)__popcnt64(wPawn & fileMasks[i]);
		int bPop = (int)__popcnt64(bPawn & fileMasks[i]);

		if (wPop > 0) {

			if (wPop > 1) score -= (wPop - 1) * DOUBLED_PAWN_PENALTY;

			if (!(wPawn & adjacentFileMasks[i])) score -= wPop * ISOLATED_PAWN_PENALTY;
			
			if (!(bPawn & adjacentFileMasks[i]) && bPop == 0) {

				U64 passedPawns = wPawn & fileMasks[i];
				while (passedPawns) {
					int sq = get_lsb_index(passedPawns);
					int rank = sq / 8;

					U64 forwardMask = ~((1ULL << ((rank + 1) * 8)) - 1);
					U64 blockingPawns = bPawn & (fileMasks[i] | adjacentFileMasks[i]) & forwardMask;

					if (blockingPawns == 0) {
						score += passedPawnBonus[rank];
					}
					passedPawns &= passedPawns - 1;

					passedPawns &= passedPawns - 1;
				}
			}
		}

		if (bPop > 0) {

			if (bPop > 1) score += (bPop - 1) * DOUBLED_PAWN_PENALTY;

			if (!(bPawn & adjacentFileMasks[i])) score += bPop * ISOLATED_PAWN_PENALTY;

			if (!(wPawn & adjacentFileMasks[i]) && wPop == 0) {
				U64 passedPawns = bPawn & fileMasks[i];
				while (passedPawns) {
					int sq = get_lsb_index(passedPawns);
					int rank = mirror_index(sq) / 8;

					U64 forwardMask = (1ULL << (rank * 8)) - 1;
					U64 blockingPawns = wPawn & (fileMasks[i] | adjacentFileMasks[i]) & forwardMask;


					if (blockingPawns == 0) {
						score -= passedPawnBonus[mirror_index(sq) / 8];
					}
					passedPawns &= passedPawns - 1;
				}
			}

		}
	}

	return score;
}

int king_safety_evaluation(Board* board) {
	int score = 0;

	int wKingSq = get_lsb_index(board->pieceBitboards[WHITE][KING]);
	int bKingSq = get_lsb_index(board->pieceBitboards[BLACK][KING]);

	int wKingFile = wKingSq % 8;
	int bKingFile = bKingSq % 8;

	U64 wPawns = board->pieceBitboards[WHITE][PAWN];

	if ((wPawns & fileMasks[wKingFile]) == 0) {
		score -= KING_FILE_OPEN_PENALTY;
	}
	if (wKingFile > 0 && (wPawns & fileMasks[wKingFile - 1]) == 0) {
		score -= KING_ADJ_OPEN_PENALTY;
	}
	if (wKingFile < 7 && (wPawns & fileMasks[wKingFile + 1]) == 0) {
		score -= KING_ADJ_OPEN_PENALTY;
	}

	U64 bPawns = board->pieceBitboards[BLACK][PAWN];

	if ((bPawns & fileMasks[bKingFile]) == 0) {
		score += KING_FILE_OPEN_PENALTY;
	}
	if (bKingFile > 0 && (bPawns & fileMasks[bKingFile - 1]) == 0) {
		score += KING_ADJ_OPEN_PENALTY;
	}
	if (bKingFile < 7 && (bPawns & fileMasks[bKingFile + 1]) == 0) {
		score += KING_ADJ_OPEN_PENALTY;
	}

	return score;
}

int evaluate(Board* board, int alpha, int beta) {
	int score = 0;
	int noPawnEval = 0;

	score += material_evaluation(board, WHITE, &noPawnEval);
	score += pst_lazy_evaluation(board, noPawnEval);

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
	else {
		score += king_safety_evaluation(board);
	}

	int lazyScore = (board->sideToMove == WHITE) ? score: -score;

	if (lazyScore + LAZY_EVAL_MARGIN <= alpha || lazyScore - LAZY_EVAL_MARGIN >= beta) {
		return lazyScore;
	}

	score += pawn_evaluation(board);

	score += mobility_evaluation(board);

	return (board->sideToMove == WHITE) ? score : -score;
}

// --- Move search ---
// Despite many of the functions below containig similar code,
// it's better not to merge them as they have different purposes,
// and merging them would present more edge cases, making the code less readable.

void init_lmr_table() {
	for (int d = 0; d < 64; d++) {
		for (int m = 0; m < 256; m++) {
			lmrTable[d][m] = 0;
			if (d >= 3 && m >= 3) {
				lmrTable[d][m] = (int)(0.99 + log(d) * log(m) / 3.14);
			}
		}
	}
}

void make_null_move(Board* board, int currDepth) {
	board->history[currDepth].castlingPerms = board->castlingPerms;
	board->history[currDepth].enPassant = board->enPassant;
	board->history[currDepth].halfMoves = board->halfMoves;
	board->history[currDepth].hashKey = board->hashKey;

	if (board->enPassant != -1) {
		board->hashKey ^= enPassantKeys[board->enPassant % 8];
		board->enPassant = -1;
	}

	board->sideToMove = 1 - board->sideToMove;
	board->hashKey ^= sideKey;
	
	board->halfMoves = 0;
}

void unmake_null_move(Board* board, int currDepth) {
	board->sideToMove = 1 - board->sideToMove;
	board->enPassant = board->history[currDepth].enPassant;
	board->halfMoves = board->history[currDepth].halfMoves;
	board->hashKey = board->history[currDepth].hashKey;
}

bool is_heavy_board(Board* board) {
	return (board->pieceBitboards[board->sideToMove][KNIGHT] |
		board->pieceBitboards[board->sideToMove][BISHOP] |
		board->pieceBitboards[board->sideToMove][ROOK] |
		board->pieceBitboards[board->sideToMove][QUEEN]) > 0;
}

bool is_repetition(Board* board, int ply) {
	int searchLimit = ply - board->halfMoves;
	if (searchLimit < 0) searchLimit = 0;

	for (int i = ply - 2; i >= searchLimit; i -= 2) {
		if (board->history[i].hashKey == board->hashKey) {
			return true;
		}
	}

	
	if (board->halfMoves > ply) {
		int halfMovesInGame = board->halfMoves - ply;
		int gameLimit = board->gamePly - halfMovesInGame;
		if (gameLimit < 0) gameLimit = 0;

		int startIndex = board->gamePly - ((ply % 2) == 0 ? 2 : 1);

		for (int i = startIndex; i >= gameLimit; i -= 2) {
			if (i >= 0 && board->gameHistory[i] == board->hashKey) {
				return true;
			}
		}
	}

	return false;
}

void order_moves(Board* board, MoveList* list, int ttMove, int ply) {

	// Optimization for later: add both white and black attack squares bitboard,
	// to penalize moving into an attacked square

	const ttMoveScore =			10000000;
	const captureBonus =		100000;
	const promotionBonus =		100000;
	const killerMove1Score =	80000;
	const killerMove2Score =	70000;
	const historyBonus =		60000;


	for (int i = 0; i < list->count; i++) {
		int move = list->moves[i];
		int moveScore = 0;

		if (ttMove == move) {
			list->scores[i] = ttMoveScore;
			continue;
		}

		// Unpack move
		
		int fromSq = GET_MOVE_SOURCE(move);
		int toSq = GET_MOVE_TARGET(move);
		PieceType piece = GET_MOVE_PIECE(move);
		PieceType captured = board->pieceOnSquare[toSq];
		PieceType promoted = GET_PROMOTION_PIECE(move);

		__assume(piece >= 0 && piece <= 5);
		__assume(captured >= 0 && captured <= 5);
		__assume(promoted >= 0 && promoted <= 5);

		// Positive feedback for capturing an opponent's piece
		if (captured != NONE) {
			moveScore += captureBonus + (10 * pieceValues[captured] - pieceValues[piece]);
		}
		else if (GET_MOVE_EN_PASSANT(move)) {
			// L'en-passant è un pedone che mangia un pedone
			moveScore += captureBonus + (10 * pieceValues[0] - pieceValues[0]);
		}

		// A promotion notably increases the value
		if (promoted != 0) {
			moveScore += promotionBonus + pieceValues[promoted];
		}

		// Killer moves and History heuristic
		if (moveScore == 0) {
			if (move == killerMoves[ply][0]) {
				moveScore += killerMove1Score;

			}
			else if (move == killerMoves[ply][1]) {
				moveScore += killerMove2Score;

			}
			else {
				int hScore = historyCutTable[board->sideToMove][fromSq][toSq];

				if (hScore > historyBonus) hScore = historyBonus;

				moveScore += hScore;
			}

 
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

int quiescence_search(Board* board, int alpha, int beta, int ply) {
	if (ply > MAX_PLY - 1) return evaluate(board, alpha, beta);

	nodesCalculated++;
	if ((nodesCalculated & 2047) == 0) {
		if (clock() > targetTime) {
			timeOut = true;
		}
	}
	if (timeOut) return 0;

	int standPat = evaluate(board, alpha, beta);
	int bestVal = standPat;

	// If current player skips it's turn and the evaluation is
	// still very good, return immediatly.
	if (bestVal >= beta) return bestVal;

	// Otherwise, switch alpha
	if (bestVal > alpha) alpha = bestVal;

	MoveList list = { 0 };
	generate_pseudo_moves(board, &list);

	order_moves(board, &list, 0, ply);

	// Standard move iteration, checking only capture moves
	for (int i = 1; i < list.count; i++)
	{
		int move = list.moves[i];

		int toSq = GET_MOVE_TARGET(move);
		PieceType captured = board->pieceOnSquare[toSq];

		// Skip if not a capture/promotion
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

int nega_max(Board* board, int depth, int alpha, int beta, int extension, int ply, bool allowNull) {
	// Heavily commented by design, since all functions 
	// below contain similar logic and calls.

	nodesCalculated++;
	if ((nodesCalculated & 2047) == 0) {
		if (clock() > targetTime) {
			timeOut = true;
		}
	}

	if (ply > 0 && is_repetition(board,ply)) {
		return 0;
	}

	// Transp table checking: if we have already explored this position,
	// and the stored depth is higher than the current one, 
	// we can return the stored evaluation.
	int originalAlpha = alpha;
	TranspEntry* entry = &TT[get_hash_index(board->hashKey)];
	int ttMove = 0;

	if (entry->key == board->hashKey) {
		ttMove = entry->bestMove;

		// This ensures some sort of "only if higher precision",
		// despite depth is not the only determining factor.
		if (entry->depth >= depth) {
			if (entry->flag == EXACT) return entry->eval;
			if (entry->flag == ALPHA && entry->eval <= alpha) return entry->eval;
			if (entry->flag == BETA && entry->eval >= beta) return entry->eval;
		}
	}

	if (timeOut) return 0;
	if (depth <= 0) return quiescence_search(board, alpha, beta, ply);


	// Null move pruning
	bool ttFailsLow = false;
	if (entry->key == board->hashKey && entry->eval < beta) {
		ttFailsLow = true;
	}

	if (!ttFailsLow && !is_square_attacked(board, board->kingSq[board->sideToMove], 1 - board->sideToMove) && is_heavy_board(board)) {
		int reduction = 4;

		make_null_move(board, ply);
		int eval = -nega_max(board, depth - reduction, -beta, -beta + 1, extension, ply + 1, false);
		unmake_null_move(board, ply);

		if (timeOut) return 0;
		if (eval >= beta) return eval;
	}
	


	// --- Actual negamax search ---
	int best = -INF;
	int legalMoves = 0;
	MoveList list = { 0 };

	// Used to update the best position in this scenario, for the transposition table.
	// If beta cut happens, it will be ignored, since the opponent will never pick it.
	int bestMoveInPosition = 0;
	
	generate_pseudo_moves(board, &list);
	
	// Add priority to bestMove from the TT, to improve alpha-beta pruning.
	order_moves(board, &list, ttMove, ply);

	for (int i = 0; i < list.count; i++) {
		int move = list.moves[i];
		int nextDepth = depth - 1;
		SelectionColor sideMoved = board->sideToMove;
		bool isQuiet = (board->pieceOnSquare[GET_MOVE_TARGET(move)] == NONE) && (GET_PROMOTION_PIECE(move) == 0) && (GET_MOVE_EN_PASSANT(move) == 0);

		make_move(board, move, sideMoved, ply);
		bool isAttacked = is_square_attacked(board, board->kingSq[sideMoved], board->sideToMove);
		
		// Check if square is attacked. The opponent is now board->sideToMove,
		// since make_move changes opponent side.
		if (isAttacked) {

			// The move is illegal: unmake move, without increasing the counter.
			unmake_move(board, move, ply);
			continue;
		}
		legalMoves++;

		// Late move reduction: if it's not one of the first (ordered) moves,
		// the depth is reduced.
		bool isCheckOpponent = is_square_attacked(board, board->kingSq[board->sideToMove], sideMoved);
		bool isKiller = (move == killerMoves[ply][0] || move == killerMoves[ply][1]);

		if (depth >= 3 && legalMoves > 2 && isQuiet && !isCheckOpponent && !isKiller) {
			int safeDepth = depth > 63 ? 63 : depth;
			int safeMoves = list.count > 255 ? 255 : list.count;

			nextDepth = depth - lmrTable[safeDepth][safeMoves];

			if (nextDepth < 1) nextDepth = 1;
		}


		// Move extension if in check (added in V0.4)
		int currentExtension = extension < EXTENSION_DEEPNESS &&
			is_square_attacked(board, board->kingSq[board->sideToMove], sideMoved) ? 1 : 0;

		
		// PVS search (nagaScout).
		int score;
		bool needsFullSearch = (legalMoves == 1) ? true : false;
		
		
		if (!needsFullSearch) {

			// PVS Search
			score = -nega_max(board, nextDepth + currentExtension, -alpha - 1, -alpha, extension + currentExtension, ply + 1, true);

			
			if (score > alpha && (nextDepth < depth - 1 || score < beta) ) {
					needsFullSearch = true;
			}
		}
		
		if (needsFullSearch) {
			score = -nega_max(board, depth - 1 + currentExtension, -beta, -alpha, extension + currentExtension, ply + 1, true);
		}

		

		unmake_move(board, move, ply);

		if (timeOut) return 0;

		if (score >= beta && isQuiet) {
			if (move != killerMoves[ply][0]) {
				killerMoves[ply][1] = killerMoves[ply][0];
				killerMoves[ply][0] = move;
			}
			historyCutTable[board->sideToMove][GET_MOVE_SOURCE(move)][GET_MOVE_TARGET(move)] += depth*depth;
		}

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
			return -INF - depth;
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

int best_move(Board* board, int depth, int currBest, int* outScore) {

	// Here, for alpha-beta pruning, alpha represents our worst (pickable) scenario,
	// bete the best one the opponent would give us.
	int alpha = -INF;
	int beta = INF;
	int best = -INF;
	int bestMove = 0;
	int legalMoves = 0;
	MoveList list = { 0 };

	// Generate pseudo legal moves
	generate_pseudo_moves(board, &list);
	order_moves(board, &list, currBest, 0);

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

		int score;
		if (legalMoves == 1) {
			score = -nega_max(board, depth - 1 + extension, -beta, -alpha, extension, 1, true);
		}
		else {
			score = -nega_max(board, depth - 1 + extension, -alpha - 1, -alpha, extension, 1, true);

			if (score > alpha && score < beta) {
				score = -nega_max(board, depth - 1 + extension, -beta, -alpha, extension, 1, true);
			}
		}

		unmake_move(board, move, 0);

		if (timeOut) break;

		// Save score
		if (legalMoves == 1 || score > best) {
			best = score;
			bestMove = move;
		}

		// Alpha-beta pruning
		if (best > alpha) {
			alpha = best;
		}
	}

	*outScore = best;
	if(!timeOut) store_tt_entry(board->hashKey, depth, best, EXACT, bestMove);
	return bestMove;
}

int best_move_iterative_deepening(Board* board, int maxTime, int maxDepth) {
	timeOut = false;
	nodesCalculated = 0;

	int bestMove = 0;
	int searchedScore = -INF;
	clock_t startTime = clock();
	clock_t maxTicks = ((clock_t)maxTime * CLOCKS_PER_SEC) / 1000;
	targetTime = startTime + maxTicks;

	memset(killerMoves, 0, sizeof(killerMoves));
	memset(historyCutTable, 0, sizeof(historyCutTable));

	for (int i = 1; i < maxDepth; i++)
	{
		int currBestScore = -INF - 1;
		int currBest = best_move(board, i, bestMove, &currBestScore);

		if (timeOut) {
			if (bestMove == 0) bestMove = currBest;
			break;
		}
		bestMove = currBest;
		searchedScore = currBestScore;


		// UCI info output
		clock_t currentTime = clock();
		int timeElapsedMs = (int)((currentTime - startTime) * 1000 / CLOCKS_PER_SEC);
		if (timeElapsedMs == 0) timeElapsedMs = 1;

		printf("info depth %d score cp %d nodes %lld time %d\n", i, searchedScore, nodesCalculated, timeElapsedMs);
		fflush(stdout);

	}
	return bestMove;
}