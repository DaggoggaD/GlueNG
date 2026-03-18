#include "PseudoMovesCalc.h"

const U64 rank4 = 0x00000000FF000000ULL;
const U64 rank5 = 0x000000FF00000000ULL;
const U64 rank8 = 0xFF00000000000000ULL;

const U64 castlingBlockers[4] = {
	96ULL, // 2	   0 0 0 0 *(0) 1 1 0
	14ULL,	// 3      0 1 1 1 *(0) 0 0 0
	0x6000000000000000ULL,
	0x0E00000000000000ULL,
}; 

const int castlingDestinations[4] = {
	G1,
	C1,
	G8,
	C8
};


U64 get_knight_pseudo_moves(int square, U64 ownPieces) {
	if (square < 0 || square > 63) {
		fprintf(stderr, "Error: Invalid square input. Square must be between 0 and 63.\n");
		return 0ULL;
	}
	return knightAttackSquares[square] & ~ownPieces;
}

U64 get_king_pseudo_moves(int square, U64 ownPieces) {
	if (square < 0 || square > 63) {
		fprintf(stderr, "Error: Invalid square input. Square must be between 0 and 63.\n");
		return 0ULL;
	}
	return kingAttackSquares[square] & ~ownPieces;
}

U64 get_rook_pseudo_moves(int square, U64 blockers, U64 ownPieces) {
	if (square < 0 || square > 63) {
		fprintf(stderr, "Error: Invalid square input. Square must be between 0 and 63.\n");
		return 0ULL;
	}
	int magicIndex = get_magic_index(blockers, &RookMagics[square]);
	U64 attacks = RookMoves[square][magicIndex];

	return attacks & ~ownPieces;
}

U64 get_bishop_pseudo_moves(int square, U64 blockers, U64 ownPieces) {
	if (square < 0 || square > 63) {
		fprintf(stderr, "Error: Invalid square input. Square must be between 0 and 63.\n");
		return 0ULL;
	}
	int magicIndex = get_magic_index(blockers, &BishopMagics[square]);
	U64 attacks = BishopMoves[square][magicIndex];

	return attacks & ~ownPieces;
}

U64 get_queen_pseudo_moves(int square, U64 blockers, U64 ownPieces) {
	if (square < 0 || square > 63) {
		fprintf(stderr, "Error: Invalid square input. Square must be between 0 and 63.\n");
		return 0ULL;
	}

	return get_rook_pseudo_moves(square, blockers, ownPieces) | get_bishop_pseudo_moves(square, blockers, ownPieces);
}


//								--- Pawns ---

// Pawn movements directions. SET-WISE.

U64 no_ea_pawns(U64 b) { return (b & notHFile) << 9; }
U64 so_ea_pawns(U64 b) { return (b & notHFile) >> 7; }
U64 no_we_pawns(U64 b) { return (b & notAFile) << 7; }
U64 so_we_pawns(U64 b) { return (b & notAFile) >> 9; }

U64 north_one_pawns(U64 pawns) {
	return (pawns << 8);
}

U64 south_one_pawns(U64 pawns) {
	return (pawns >> 8);
}

//							 ### Pawn pushes ###


// "Single/double push" functions, "1" on every destination square.

U64 single_white_pawn_pushes(U64 pawns, U64 allPieces) {
	return north_one_pawns(pawns) & ~allPieces;
}

U64 single_black_pawn_pushes(U64 pawns, U64 allPieces) {
	return south_one_pawns(pawns) & ~allPieces;
}

U64 double_white_pawn_pushes(U64 pawns, U64 allPieces) {
	U64 singlePushes = single_white_pawn_pushes(pawns, allPieces);

	return north_one_pawns(singlePushes) & ~allPieces & rank4;
}

U64 double_black_pawn_pushes(U64 pawns, U64 allPieces) {
	U64 singlePushes = single_black_pawn_pushes(pawns, allPieces);

	return south_one_pawns(singlePushes) & ~allPieces & rank5;
}


// "Able to push" functions, "1" on every departure square 
// that CAN be pushed or double psuhed from.


U64 white_pawns_able_to_push(U64 pawns, U64 allPieces) {

	// To be able to push, the square in front of the pawn must be empty.
	// We get the empty squares, move them south (as white pawns move north),
	// and then we get the pawns that can push to those squares.
	return south_one_pawns(~allPieces) & pawns;
}

U64 black_pawns_able_to_push(U64 pawns, U64 allPieces) {

	// Same logic as for white pawns, but with north instead of south.
	return north_one_pawns(~allPieces) & pawns;
}

U64 white_pawns_able_to_double_push(U64 pawns, U64 allPieces) {
	
	// In rank 4, to be a valid target square for a double push,
	// it must be empty, and so must be the one in rank 3.
	// We get the empty squares, and it with rank4 to get the valid target squares,
	// then we get the pawns that can push to those squares.
	U64 emptyRank3 = south_one_pawns(~allPieces & rank4) & ~allPieces;

	return white_pawns_able_to_push(pawns, ~emptyRank3);
}

U64 black_pawns_able_to_double_push(U64 pawns, U64 allPieces) {
	
	// Same logic as for white pawns, but with rank 5 and rank 6.
	U64 emptyRank6 = north_one_pawns(~allPieces & rank5) & ~allPieces;
	return black_pawns_able_to_push(pawns, ~emptyRank6);
}

//							 ### Pawn attacks ###

// Attacks function for all the white pawns (set-wise)
U64 white_pawns_all_attacks(U64 pawns) {
	return no_ea_pawns(pawns) | no_we_pawns(pawns);
}

U64 black_pawns_all_attacks(U64 pawns) {
	return so_ea_pawns(pawns) | so_we_pawns(pawns);
}

// Double - single attack functions for evaluation later.
// view https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)
// for more infos.


// Gets the index of the least significant bit in a bitboard,
// learnt from https://www.chessprogramming.org/BitScan.
static inline int get_lsb_index(U64 b) {
	unsigned long index;
	_BitScanForward64(&index, b);
	return index; 
}

// Bit packing to save space:
// 0 - 5: fromSquare (0-63) 6 - 11: toSquare (0-63) 12 - 15: piece type (0-5)
// 16 - 19: promotion 20: enPassant (0 - 1)
// 
// Note: promotion: 0 = no promotion, != 0 PieceType piece
//		 enPassant: 0 = not enPassant move, 1 = enPassant move
// 
// Decode it with GET_FROM_SQUARE(move), GET_TO_SQUARE(move), GET_PIECE_TYPE(move) macros.
void add_move_to_list(MoveList *list, int fromSquare, int toSquare, PieceType piece, PieceType promoted_to, bool enPassant) {
	if (list->count >= 256) {
		fprintf(stderr, "Error: Move list is full. Cannot add more moves.\n");
		return;
	}
	
	int move = fromSquare | (toSquare << 6) | (piece << 12) | (promoted_to << 16) | (enPassant << 20);
	
	list->moves[list->count] = move;
	list->count++;
}

void white_generate_pseudo_moves(Board *board, MoveList *list) {

	// PAWNS - Single pushes
	U64 singlePushes = single_white_pawn_pushes(board->pieceBitboards[WHITE][PAWN], board->occupiedBitboards[BOTH]);
	
	U64 promotions = singlePushes & rank8;
	U64 normalPushes = singlePushes & ~rank8;
	
	while (normalPushes) {
		int toSquare = get_lsb_index(normalPushes);
		int fromSquare = toSquare - 8; 

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);
		normalPushes &= normalPushes - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare - 8;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT, 0);

		promotions &= promotions - 1;
	}

	// PAWNS - Double pushes
	U64 doublePushes = double_white_pawn_pushes(board->pieceBitboards[WHITE][PAWN], board->occupiedBitboards[BOTH]);
	while (doublePushes) {
		int toSquare = get_lsb_index(doublePushes);
		int fromSquare = toSquare - 16; 

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0,0);
		doublePushes &= doublePushes - 1;
	}

	// PAWNS - Attacks (East)
	U64 pawnAttacksEast = no_ea_pawns(board->pieceBitboards[WHITE][PAWN]) & board->occupiedBitboards[BLACK];

	promotions = pawnAttacksEast & rank8;
	U64 normalEastAttacks = pawnAttacksEast & ~rank8;
	
	while (normalEastAttacks)
	{
		int toSquare = get_lsb_index(normalEastAttacks);
		int fromSquare = toSquare - 9;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0,0);

		normalEastAttacks &= normalEastAttacks - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare - 9;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN,0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK,0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP,0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT,0);

		promotions &= promotions - 1;
	}


	// PAWNS - Attacks (West)
	U64 pawnAttacksWest = no_we_pawns(board->pieceBitboards[WHITE][PAWN]) & board->occupiedBitboards[BLACK];
	
	promotions = pawnAttacksWest & rank8;
	U64 normalWestAttacks = pawnAttacksWest & ~rank8;
	
	while (normalWestAttacks){
		int toSquare = get_lsb_index(normalWestAttacks);
		int fromSquare = toSquare - 7;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0,0);

		normalWestAttacks &= normalWestAttacks - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare - 7;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN,0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK,0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP,0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT,0);
		
		promotions &= promotions - 1;
	}

	
	// KNIGHTS
	U64 knight = board->pieceBitboards[WHITE][KNIGHT];
	while (knight) {
		int square = get_lsb_index(knight);
		U64 attacks = get_knight_pseudo_moves(square, board->occupiedBitboards[WHITE]);
				
		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KNIGHT, 0,0);

			attacks &= attacks - 1;
		}


		knight &= knight - 1;
	}

	// BISHOPS
	U64 bishops = board->pieceBitboards[WHITE][BISHOP];
	while (bishops) {
		int square = get_lsb_index(bishops);
		U64 attacks = get_bishop_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, BISHOP, 0,0);

			attacks &= attacks - 1;

		}

		bishops &= bishops - 1;
	}

	// ROOKS
	U64 rooks = board->pieceBitboards[WHITE][ROOK];
	while (rooks) {
		int square = get_lsb_index(rooks);
		U64 attacks = get_rook_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, ROOK, 0,0);

			attacks &= attacks - 1;

		}

		rooks &= rooks - 1;
	}

	// QUEENS
	U64 queens = board->pieceBitboards[WHITE][QUEEN];
	while (queens) {
		int square = get_lsb_index(queens);
		U64 attacks = get_queen_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, QUEEN, 0,0);

			attacks &= attacks - 1;

		}


		queens &= queens - 1;
	}

	// KING
	U64 king = board->pieceBitboards[WHITE][KING];
	if (king) { // Usiamo if perché c'è un solo Re
		int square = get_lsb_index(king);
		U64 attacks = get_king_pseudo_moves(square, board->occupiedBitboards[WHITE]);
	
		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KING, 0,0);

			attacks &= attacks - 1;

		}

	}

	// Castling
	int castling = board->castlingPerms & (WKCA | WQCA) ;
	while (castling) {
		int index = get_lsb_index(castling);
		U64 castlingBitMask = castlingBlockers[index] & board->occupiedBitboards[BOTH];
		if (castlingBitMask == 0) {
			add_move_to_list(list, E1, castlingDestinations[index], KING, 0,0);
		}


		castling &= castling - 1;
	}


	if (board->enPassant != -1) {
		U64 enPassantSquare = 1ULL << board->enPassant;

		U64 attacksToEnPassant = (so_ea_pawns(enPassantSquare) | so_we_pawns(enPassantSquare)) & board->pieceBitboards[WHITE][PAWN];
		
		while (attacksToEnPassant) {
			int fromSquare = get_lsb_index(attacksToEnPassant);

			add_move_to_list(list, fromSquare, board->enPassant, PAWN, 0, true);

			attacksToEnPassant &= attacksToEnPassant - 1;
		}

	}

}

void black_generate_pseudo_moves(Board* board, MoveList* list) {
	// PAWNS - Single pushes
	U64 singlePushes = single_black_pawn_pushes(board->pieceBitboards[BLACK][PAWN], board->occupiedBitboards[BOTH]);
	while (singlePushes) {
		int toSquare = get_lsb_index(singlePushes);
		int fromSquare = toSquare + 8;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0,0);
		singlePushes &= singlePushes - 1;
	}

	// PAWNS - Double pushes
	U64 doublePushes = double_black_pawn_pushes(board->pieceBitboards[BLACK][PAWN], board->occupiedBitboards[BOTH]);
	while (doublePushes) {
		int toSquare = get_lsb_index(doublePushes);
		int fromSquare = toSquare + 16;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0,0);
		doublePushes &= doublePushes - 1;
	}

	// PAWNS - Attacks (East)
	U64 pawnAttacksEast = so_ea_pawns(board->pieceBitboards[BLACK][PAWN]) & board->occupiedBitboards[WHITE];
	while (pawnAttacksEast)
	{
		int toSquare = get_lsb_index(pawnAttacksEast);
		int fromSquare = toSquare + 7;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0,0);

		pawnAttacksEast &= pawnAttacksEast - 1;
	}

	// PAWNS - Attacks (West)
	U64 pawnAttacksWest = so_we_pawns(board->pieceBitboards[BLACK][PAWN]) & board->occupiedBitboards[WHITE];
	while (pawnAttacksWest)
	{
		int toSquare = get_lsb_index(pawnAttacksWest);
		int fromSquare = toSquare + 9;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0,0);

		pawnAttacksWest &= pawnAttacksWest - 1;
	}


	// KNIGHTS
	U64 knight = board->pieceBitboards[BLACK][KNIGHT];
	while (knight) {
		int square = get_lsb_index(knight);
		U64 attacks = get_knight_pseudo_moves(square, board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KNIGHT, 0,0);

			attacks &= attacks - 1;
		}


		knight &= knight - 1;
	}

	// BISHOPS
	U64 bishops = board->pieceBitboards[BLACK][BISHOP];
	while (bishops) {
		int square = get_lsb_index(bishops);
		U64 attacks = get_bishop_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, BISHOP, 0,0);

			attacks &= attacks - 1;

		}

		bishops &= bishops - 1;
	}

	// ROOKS
	U64 rooks = board->pieceBitboards[BLACK][ROOK];
	while (rooks) {
		int square = get_lsb_index(rooks);
		U64 attacks = get_rook_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, ROOK, 0,0);

			attacks &= attacks - 1;

		}

		rooks &= rooks - 1;
	}

	// QUEENS
	U64 queens = board->pieceBitboards[BLACK][QUEEN];
	while (queens) {
		int square = get_lsb_index(queens);
		U64 attacks = get_queen_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, QUEEN, 0,0);

			attacks &= attacks - 1;

		}


		queens &= queens - 1;
	}

	// KING
	U64 king = board->pieceBitboards[BLACK][KING];
	if (king) { // Usiamo if perché c'è un solo Re
		int square = get_lsb_index(king);
		U64 attacks = get_king_pseudo_moves(square, board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KING, 0,0);

			attacks &= attacks - 1;

		}

	}

	


}