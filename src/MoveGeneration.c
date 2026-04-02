#include "MoveGeneration.h"


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

// --- Move generation ---

void unmake_move(Board* board, int move, int currDepth) {
	// Restore board propreties
	board->halfMoves = board->history[currDepth].halfMoves;
	board->castlingPerms = board->history[currDepth].castlingPerms;
	board->enPassant = board->history[currDepth].enPassant;
	board->sideToMove = 1-board->sideToMove;
	board->hashKey = board->history[currDepth].hashKey;


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
	board->history[currDepth].hashKey = board->hashKey;

	
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

	board->hashKey ^= pieceKeys[sideToMove][piece][fromSq];
	board->hashKey ^= pieceKeys[sideToMove][pieceToPlace][toSq];



	// Capture logic
	if (captureBB && !enPassant) {
		PieceType captureBBdPiece = board->pieceOnSquare[toSq];
		board->pieceBitboards[opposed][captureBBdPiece] ^= toBB;
		board->occupiedBitboards[opposed] ^= toBB;
		board->hashKey ^= pieceKeys[opposed][captureBBdPiece][toSq];

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
		board->hashKey ^= pieceKeys[opposed][captureBBdPiece][realPawnIndex];
	}

	if (board->enPassant != -1) {
		board->hashKey ^= enPassantKeys[board->enPassant % 8];
	}

	board->enPassant = -1;
	if (piece == PAWN && abs(fromSq - toSq) == 16) {
		int mult = (sideToMove == WHITE) ? 1 : -1;
		int enPassantSq = toSq - mult * 8;

		board->enPassant = enPassantSq;
		board->hashKey ^= enPassantKeys[enPassantSq % 8];

	}

	// Castling logic
	board->hashKey ^= castleKeys[board->castlingPerms];
	board->castlingPerms &= castlingMask[fromSq] & castlingMask[toSq];
	board->hashKey ^= castleKeys[board->castlingPerms];
	if (piece == KING && abs(fromSq - toSq) == 2) {
		int rookFrom = rookCastleFrom[toSq];
		int rookTo = rookCastleTo[toSq];

		U64 rookFromToBB = (1ULL << rookFrom) ^ (1ULL << rookTo);

		board->pieceBitboards[sideToMove][ROOK] ^= rookFromToBB;
		board->occupiedBitboards[sideToMove] ^= rookFromToBB;

		board->pieceOnSquare[rookFrom] = NONE;
		board->pieceOnSquare[rookTo] = ROOK;

		board->hashKey ^= pieceKeys[sideToMove][ROOK][rookFrom];
		board->hashKey ^= pieceKeys[sideToMove][ROOK][rookTo];
	}


	if (piece == KING) board->kingSq[sideToMove] = toSq;

	// Update pieceSquare array and occupancies.
	board->pieceOnSquare[fromSq] = NONE;
	board->pieceOnSquare[toSq] = pieceToPlace;
	board->occupiedBitboards[BOTH] = board->occupiedBitboards[WHITE] | board->occupiedBitboards[BLACK];
	board->sideToMove = 1 - sideToMove;
	board->hashKey ^= sideKey;

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