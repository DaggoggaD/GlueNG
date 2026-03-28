#include "MoveGeneration.h"

bool timeOut = false;
clock_t targetTime = 0;
U64 nodesCalculated = 0;

// Castling helpers consts
const int castlingMask[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	 7, 15, 15, 15,  3, 15, 15, 11
};

const int rookCastleFrom[64] = {
	[G1] = H1,
	[C1] = A1,
	[G8] = H8,
	[C8] = A8
};

const int rookCastleTo[64] = {
	[G1] = F1,
	[C1] = D1,
	[G8] = F8,
	[C8] = D8
};

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


// --- Move generation ---

void unmake_move(Board* board, int move, int currDepth) {
	// Restore board propreties
	board->halfMoves = board->history[currDepth].halfMoves;
	board->castlingPerms = board->history[currDepth].castlingPerms;
	board->enPassant = board->history[currDepth].enPassant;
	board->sideToMove = 1-board->sideToMove;


	// Unpack Move Struct
	SelectionColor sideToMove = board->sideToMove;
	SelectionColor opposed = sideToMove ^ 1;
	PieceType piece = GET_MOVE_PIECE(move);
	PieceType promotion = GET_PROMOTION_PIECE(move); // 0: no promotion, !0 piece type
	int fromSq = GET_MOVE_SOURCE(move);
	int toSq = GET_MOVE_TARGET(move);
	bool enPassant = GET_MOVE_EN_PASSANT(move);

	U64 fromBB = 1ULL << fromSq;
	U64 toBB = 1ULL << toSq;
	U64 fromToBB = fromBB ^ toBB;
	PieceType pieceToPlace = (promotion == 0) ? piece : promotion;

	__assume(piece >= 0 && piece <= 5);
	__assume(promotion >= 0 && promotion <= 5);

	board->pieceBitboards[sideToMove][piece] ^= fromBB;
	board->pieceBitboards[sideToMove][pieceToPlace] ^= toBB;
	board->occupiedBitboards[sideToMove] ^= fromToBB;

	if (board->history[currDepth].capturedPiece != NONE && !enPassant) {
		PieceType captureBBdPiece = board->history[currDepth].capturedPiece;
		board->pieceBitboards[opposed][captureBBdPiece] ^= toBB;
		board->occupiedBitboards[opposed] ^= toBB;
	}

	if (enPassant) {
		// mult to change direction based on WHITE/BLACK turn.
		// and calculate "real" pawn position.
		int mult = (sideToMove == WHITE) ? 1 : -1;
		int realPawnIndex = toSq - mult * 8;
		U64 realPawnBB = 1ULL << realPawnIndex;

		__assume(realPawnIndex >= 0 && realPawnIndex <= 63);

		PieceType captureBBdPiece = PAWN;
		board->pieceOnSquare[realPawnIndex] = PAWN;
		board->pieceBitboards[opposed][captureBBdPiece] ^= realPawnBB;
		board->occupiedBitboards[opposed] ^= realPawnBB;
	}

	if (piece == KING && abs(fromSq - toSq) == 2) {
		int rookFrom = rookCastleFrom[toSq];
		int rookTo = rookCastleTo[toSq];

		U64 rookFromToBB = (1ULL << rookFrom) ^ (1ULL << rookTo);

		board->pieceBitboards[sideToMove][ROOK] ^= rookFromToBB;
		board->occupiedBitboards[sideToMove] ^= rookFromToBB;

		board->pieceOnSquare[rookFrom] = ROOK;
		board->pieceOnSquare[rookTo] = NONE;
	}

	board->pieceOnSquare[fromSq] = piece;
	board->pieceOnSquare[toSq] = board->history[currDepth].capturedPiece;
	board->occupiedBitboards[BOTH] = board->occupiedBitboards[WHITE] | board->occupiedBitboards[BLACK];

	if (piece == KING) board->kingSq[sideToMove] = fromSq;
}

void make_move(Board* board, int move, SelectionColor sideToMove, int currDepth) {
	// Unpack Move struct
	SelectionColor opposed = sideToMove ^ 1;
	PieceType piece = GET_MOVE_PIECE(move);
	PieceType promotion = GET_PROMOTION_PIECE(move); // 0: no promotion, !0 piece type
	int fromSq = GET_MOVE_SOURCE(move);
	int toSq = GET_MOVE_TARGET(move);
	bool enPassant = GET_MOVE_EN_PASSANT(move);

	// Update UndoHistory
	board->history[currDepth].castlingPerms = board->castlingPerms;
	board->history[currDepth].enPassant = board->enPassant;
	board->history[currDepth].halfMoves = board->halfMoves;
	board->history[currDepth].capturedPiece = board->pieceOnSquare[toSq];


	
	// To stop the debugger complaining, masked earlier to 
	// be in correct index range
	__assume(piece >= 0 && piece <= 5);
	__assume(promotion >= 0 && promotion <= 5);

	// Helper bitmasks
	U64 fromBB = 1ULL << fromSq;
	U64 toBB = 1ULL << toSq;
	U64 fromToBB = fromBB ^ toBB;
	U64 captureBB = toBB & board->occupiedBitboards[opposed];
	PieceType pieceToPlace = (promotion == 0) ? piece : promotion;


	// Use XOR to:
	// Change original piece's bitmask;
	// Change arrival pieces's bitmask (could be a different piece, due to promotions);
	// Change occupancy mask.
	board->pieceBitboards[sideToMove][piece] ^= fromBB;
	board->pieceBitboards[sideToMove][pieceToPlace] ^= toBB;
	board->occupiedBitboards[sideToMove] ^= fromToBB;

	// Capture logic
	if (captureBB && !enPassant) {
		PieceType captureBBdPiece = board->pieceOnSquare[toSq];
		board->pieceBitboards[opposed][captureBBdPiece] ^= toBB;
		board->occupiedBitboards[opposed] ^= toBB;
	}

	if (enPassant) {
		// mult to change direction based on WHITE/BLACK turn.
		// and calculate "real" pawn position.
		int mult = (sideToMove == WHITE) ? 1 : -1;
		int realPawnIndex = toSq - mult * 8;
		U64 realPawnBB = 1ULL << realPawnIndex;

		__assume(realPawnIndex >= 0 && realPawnIndex <= 63);

		PieceType captureBBdPiece = board->pieceOnSquare[realPawnIndex];
		board->pieceOnSquare[realPawnIndex] = NONE;
		board->pieceBitboards[opposed][captureBBdPiece] ^= realPawnBB;
		board->occupiedBitboards[opposed] ^= realPawnBB;
	}

	board->enPassant = -1;
	if (piece == PAWN && abs(fromSq - toSq) == 16) {
		int mult = (sideToMove == WHITE) ? 1 : -1;
		int enPassantSq = toSq - mult * 8;

		board->enPassant = enPassantSq;
	}

	// Castling logic
	board->castlingPerms &= castlingMask[fromSq] & castlingMask[toSq];
	if (piece == KING && abs(fromSq - toSq) == 2) {
		int rookFrom = rookCastleFrom[toSq];
		int rookTo = rookCastleTo[toSq];

		U64 rookFromToBB = (1ULL << rookFrom) ^ (1ULL << rookTo);

		board->pieceBitboards[sideToMove][ROOK] ^= rookFromToBB;
		board->occupiedBitboards[sideToMove] ^= rookFromToBB;

		board->pieceOnSquare[rookFrom] = NONE;
		board->pieceOnSquare[rookTo] = ROOK;
	}


	if (piece == KING) board->kingSq[sideToMove] = toSq;

	// Update pieceSquare array and occupancies.
	board->pieceOnSquare[fromSq] = NONE;
	board->pieceOnSquare[toSq] = pieceToPlace;
	board->occupiedBitboards[BOTH] = board->occupiedBitboards[WHITE] | board->occupiedBitboards[BLACK];
	board->sideToMove = 1 - sideToMove;

	if (piece == PAWN || captureBB) board->halfMoves = 0;
	else board->halfMoves++;
	
}

U64 perft(Board* board, int depth) {
	if (depth == 0) return 1ULL;
	
	U64 nodes = 0;

	MoveList list = { 0 };

	// Generate pseudo moves
	if (board->sideToMove == WHITE) white_generate_pseudo_moves(board, &list);
	else black_generate_pseudo_moves(board, &list);

	for (int i = 0; i < list.count; i++) {
		int move = list.moves[i];
		// sideMoved represents the side moved in this turn.
		// After make_move, the side is reversed, so it has to be stored.
		SelectionColor sideMoved = board->sideToMove;


		// Make the move:
		make_move(board, move, sideMoved, depth);

		// Check if square is attacked. The opponent is now board->sideToMove,
		// since make_move changes opponent side.
		if (is_square_attacked(board, board->kingSq[sideMoved], board->sideToMove)) {
			// The move is illegal: unmake move, without increasing the counter.
			unmake_move(board, move, depth);
			continue;
		}
		
		nodes += perft(board, depth - 1);

		unmake_move(board, move, depth);
	}
	return nodes;
}

U64 perft_divide(Board* board, int depth) {
	if (depth == 0) return 1ULL;

	U64 totalNodes = 0;

	MoveList list = { 0 };

	// Generate pseudo moves
	if (board->sideToMove == WHITE) white_generate_pseudo_moves(board, &list);
	else black_generate_pseudo_moves(board, &list);


	printf("\nPERFT DIVIDE AT DEPTH %d\n", depth);

	for (int i = 0; i < list.count; i++) {
		int move = list.moves[i];
		
		// sideMoved represents the side moved in this turn.
		// After make_move, the side is reversed, so it has to be stored.
		SelectionColor sideMoved = board->sideToMove;

		// Make the move
		make_move(board, move, sideMoved, depth);

		// Check if square is attacked. The opponent is now board->sideToMove,
		// since make_move changes opponent side.

		if (is_square_attacked(board, board->kingSq[sideMoved], board->sideToMove)) {
			
			// The move is illegal: unmake move, without increasing the counter.
			unmake_move(board, move, depth);
			continue;
		}

		U64 nodes = perft(board, depth - 1);

		debug_move(move);
		printf(": %llu\n", nodes);
		if(nodes == 20) debug_board_visualizer(board);

		totalNodes += nodes;

		unmake_move(board, move, depth);
	}

	printf("\nTotal nodes: %llu", totalNodes);
	return totalNodes;
}

void generate_legal_moves(Board* board, MoveList* list) {
	list->count = 0;

	MoveList tempList = { 0 };

	// Generate pseudo moves
	if (board->sideToMove == WHITE) white_generate_pseudo_moves(board, &tempList);
	else black_generate_pseudo_moves(board, &tempList);

	for (int i = 0; i < tempList.count; i++) {
		int move = tempList.moves[i];

		SelectionColor sideMoved = board->sideToMove;

		// Make the move:
		make_move(board, move, sideMoved, 0);

		// Check if square is attacked. The opponent is now board->sideToMove,
		// since make_move changes opponent side.
		if (is_square_attacked(board, board->kingSq[sideMoved], board->sideToMove)) {
			// The move is illegal: unmake move, without increasing the counter.
			unmake_move(board, move, 0);
			continue;
		}
		
		list->moves[list->count] = move;
		list->count++;

		unmake_move(board, move, 0);
	}

}

static inline void generate_pseudo_moves(Board* board, MoveList* list) {
	if (board->sideToMove == WHITE) white_generate_pseudo_moves(board, list);
	else black_generate_pseudo_moves(board, list);
}

// --- Move evaluation ---

static inline int mirror_index(int index) {
	return index ^ 56;
}

int material_evaluation(Board* board, SelectionColor side, int *noPawnEval) {
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

	int distance = manhattan_distance(board->kingSq[WHITE], board->kingSq[BLACK]);
	int bonus = (14 - distance) * KING_CLOSENESS_BIAS;


	if (is_endgame(noPawnEval)) {
		if (score > KING_BIAS_SCORE) {
			score += bonus;
		}
		else if (score < -KING_BIAS_SCORE) {
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
		if (captured == NONE) continue; //&& GET_MOVE_EN_PASSANT(move) == 0 && GET_PROMOTION_PIECE(move) == 0) continue;

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

	if (timeOut) return 0;

	if (depth == 0) return quiescence_search(board, alpha, beta, ply);

	int best = -INFINITY;
	int legalMoves = 0;
	MoveList list = { 0 };

	generate_pseudo_moves(board, &list);

	order_moves(board, &list, 0);

	for (int i = 0; i < list.count; i++) {
		int move = list.moves[i];

		SelectionColor sideMoved = board->sideToMove;

		// Make the move
		make_move(board, move, sideMoved, ply);

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
		int score = -nega_max(board, depth-1+currentExtension, -beta, -alpha, extension+currentExtension, ply+1);
		unmake_move(board, move, ply);
		
		// Save score
		if (score > best) best = score;


		// Alpha-beta pruning

		// Better move found, store new alpha
		if(best > alpha) {
			alpha = best;
		}

		// Move too good, opponent would pick any previous branch.
		if (alpha >= beta) {
			break;
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


	return best;
}

// Helps massively alpha-beta pruning, by ordering the moves 
// according to their scores.
void order_moves(Board* board, MoveList* list, int currBest) {

	// Optimization for later: add both white and black attack squares bitboard,
	// to penalize moving into an attacked square

	for (int i = 0; i < list->count; i++) {
		int move = list->moves[i];
		int moveScore = 0;

		if (currBest == move) {
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

	// Selection sort the moves (choose a faster alg later)
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