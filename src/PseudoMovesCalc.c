#include "../include/PseudoMovesCalc.h"

const U64 rank1 = 0x00000000000000FFULL;
const U64 rank2 = 0x000000000000FF00ULL;
const U64 rank4 = 0x00000000FF000000ULL;
const U64 rank5 = 0x000000FF00000000ULL;
const U64 rank8 = 0xFF00000000000000ULL;

const U64 CastlingBlockers[4] = {
	96ULL,
	14ULL,
	0x6000000000000000ULL,
	0x0E00000000000000ULL,
}; 

const int CastlingDestinations[4] = {
	G1,
	C1,
	G8,
	C8
};


//						  --- Non sliding pieces ---

U64 get_knight_pseudo_moves(int square, U64 ownPieces) {
	assert(square >= 0 && square <= 63);

	return knightAttackSquares[square] & ~ownPieces;
}

U64 get_king_pseudo_moves(int square, U64 ownPieces) {
	assert(square >= 0 && square <= 63);

	return kingAttackSquares[square] & ~ownPieces;
}

inline U64 get_rook_pseudo_moves(int square, U64 blockers, U64 ownPieces) {
	assert(square >= 0 && square <= 63);
	int magicIndex = get_magic_index(blockers, &RookMagics[square]);
	U64 attacks = RookMoves[square][magicIndex];

	return attacks & ~ownPieces;
}

inline U64 get_bishop_pseudo_moves(int square, U64 blockers, U64 ownPieces) {
	assert(square >= 0 && square <= 63);
	int magicIndex = get_magic_index(blockers, &BishopMagics[square]);
	U64 attacks = BishopMoves[square][magicIndex];
/*	printf("%d\n", square);
	debug_bitmask_visualizer(attacks & ~ownPieces);
	printf("\n");
	*/
	return attacks & ~ownPieces;
}

inline U64 get_queen_pseudo_moves(int square, U64 blockers, U64 ownPieces) {
	assert(square >= 0 && square <= 63);
	return get_rook_pseudo_moves(square, blockers, ownPieces) | get_bishop_pseudo_moves(square, blockers, ownPieces);
}


//							--- Pawns specifics---

// Pawn movements directions. SET-WISE.
U64 no_ea_pawns(U64 b) { return (b & notHFile) << 9; }
U64 so_ea_pawns(U64 b) { return (b & notHFile) >> 7; }
U64 no_we_pawns(U64 b) { return (b & notAFile) << 7; }
U64 so_we_pawns(U64 b) { return (b & notAFile) >> 9; }
U64 north_one_pawns(U64 pawns) { return (pawns << 8); }
U64 south_one_pawns(U64 pawns) { return (pawns >> 8); }

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



void add_move_to_list(MoveList *list, int fromSquare, int toSquare, PieceType piece, PieceType promoted_to, bool enPassant) {
	if (list->count >= 256) {
		fprintf(stderr, "Error: Move list is full. Cannot add more moves.\n");
		return;
	}
	
	int move = fromSquare | (toSquare << 6) | (piece << 12) | (promoted_to << 16) | (enPassant << 20);
	
	list->moves[list->count] = move;
	list->count++;
}

//								--- PseudoLegal Move Generation ---

// Add all possible moves from a square by a specific piece type, 
// for all the pieces of that type.

static inline void white_generate_pawn_pseudo(Board* board, MoveList* list) {
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

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);
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

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);

		normalEastAttacks &= normalEastAttacks - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare - 9;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT, 0);

		promotions &= promotions - 1;
	}


	// PAWNS - Attacks (West)
	U64 pawnAttacksWest = no_we_pawns(board->pieceBitboards[WHITE][PAWN]) & board->occupiedBitboards[BLACK];

	promotions = pawnAttacksWest & rank8;
	U64 normalWestAttacks = pawnAttacksWest & ~rank8;

	while (normalWestAttacks) {
		int toSquare = get_lsb_index(normalWestAttacks);
		int fromSquare = toSquare - 7;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);

		normalWestAttacks &= normalWestAttacks - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare - 7;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT, 0);

		promotions &= promotions - 1;
	}
}

static inline void white_generate_knight_pseudo(Board* board, MoveList* list) {
	U64 knight = board->pieceBitboards[WHITE][KNIGHT];
	while (knight) {
		int square = get_lsb_index(knight);
		U64 attacks = get_knight_pseudo_moves(square, board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KNIGHT, 0, 0);

			attacks &= attacks - 1;
		}


		knight &= knight - 1;
	}
}

static inline void white_generate_bishop_pseudo(Board* board, MoveList* list) {
	U64 bishops = board->pieceBitboards[WHITE][BISHOP];
	while (bishops) {
		int square = get_lsb_index(bishops);
		U64 attacks = get_bishop_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, BISHOP, 0, 0);

			attacks &= attacks - 1;

		}

		bishops &= bishops - 1;
	}
}

static inline void white_generate_rook_pseudo(Board* board, MoveList* list) {
	U64 rooks = board->pieceBitboards[WHITE][ROOK];
	while (rooks) {
		int square = get_lsb_index(rooks);
		U64 attacks = get_rook_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, ROOK, 0, 0);

			attacks &= attacks - 1;

		}

		rooks &= rooks - 1;
	}
}

static inline void white_generate_queen_pseudo(Board* board, MoveList* list) {
	U64 queens = board->pieceBitboards[WHITE][QUEEN];
	while (queens) {
		int square = get_lsb_index(queens);
		U64 attacks = get_queen_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, QUEEN, 0, 0);

			attacks &= attacks - 1;

		}


		queens &= queens - 1;
	}
}

static inline void white_generate_king_pseudo(Board* board, MoveList* list) {
	U64 king = board->pieceBitboards[WHITE][KING];
	if (king) { // Usiamo if perché c'è un solo Re
		int square = get_lsb_index(king);
		U64 attacks = get_king_pseudo_moves(square, board->occupiedBitboards[WHITE]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KING, 0, 0);

			attacks &= attacks - 1;

		}

	}
}

static inline void white_generate_castling_legal(Board* board, MoveList* list) {

	if (board->castlingPerms & WKCA) {
		U64 castlingBitMask = CastlingBlockers[0] & board->occupiedBitboards[BOTH];

		if (castlingBitMask == 0) {

			if (!is_square_attacked(board, E1, BLACK) && !is_square_attacked(board, F1, BLACK) &&
				!is_square_attacked(board, G1, BLACK)) {
				add_move_to_list(list, E1, G1, KING, 0, 0);

			}
		}

	}

	if (board->castlingPerms & WQCA) {
		U64 castlingBitMask = CastlingBlockers[1] & board->occupiedBitboards[BOTH];


		if (castlingBitMask == 0) {
			if (!is_square_attacked(board, E1, BLACK) && !is_square_attacked(board, D1, BLACK) &&
				!is_square_attacked(board, C1, BLACK)) {
				add_move_to_list(list, E1, C1, KING, 0, 0);
			}
		}

	}
}

static inline void white_generate_enp_pseudo(Board* board, MoveList* list) {
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



static inline void black_generate_pawn_pseudo(Board* board, MoveList* list){
	// PAWNS - Single pushes
	U64 singlePushes = single_black_pawn_pushes(board->pieceBitboards[BLACK][PAWN], board->occupiedBitboards[BOTH]);
	U64 promotions = singlePushes & rank1;
	U64 normalPushes = singlePushes & ~rank1;

	while (normalPushes) {
		int toSquare = get_lsb_index(normalPushes);
		int fromSquare = toSquare + 8;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);
		normalPushes &= normalPushes - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare + 8;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT, 0);

		promotions &= promotions - 1;
	}

	// PAWNS - Double pushes
	U64 doublePushes = double_black_pawn_pushes(board->pieceBitboards[BLACK][PAWN], board->occupiedBitboards[BOTH]);
	while (doublePushes) {
		int toSquare = get_lsb_index(doublePushes);
		int fromSquare = toSquare + 16;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);
		doublePushes &= doublePushes - 1;
	}

	// PAWNS - Attacks (East)
	U64 pawnAttacksEast = so_ea_pawns(board->pieceBitboards[BLACK][PAWN]) & board->occupiedBitboards[WHITE];
	promotions = pawnAttacksEast & rank1;
	U64 normalEastAttacks = pawnAttacksEast & ~rank1;

	while (normalEastAttacks)
	{
		int toSquare = get_lsb_index(normalEastAttacks);
		int fromSquare = toSquare + 7;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);

		normalEastAttacks &= normalEastAttacks - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare + 7;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT, 0);

		promotions &= promotions - 1;
	}

	// PAWNS - Attacks (West)
	U64 pawnAttacksWest = so_we_pawns(board->pieceBitboards[BLACK][PAWN]) & board->occupiedBitboards[WHITE];
	promotions = pawnAttacksWest & rank1;
	U64 normalWestAttacks = pawnAttacksWest & ~rank1;

	while (normalWestAttacks)
	{
		int toSquare = get_lsb_index(normalWestAttacks);
		int fromSquare = toSquare + 9;

		add_move_to_list(list, fromSquare, toSquare, PAWN, 0, 0);

		normalWestAttacks &= normalWestAttacks - 1;
	}

	while (promotions) {
		int toSquare = get_lsb_index(promotions);
		int fromSquare = toSquare + 9;

		add_move_to_list(list, fromSquare, toSquare, PAWN, QUEEN, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, ROOK, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, BISHOP, 0);
		add_move_to_list(list, fromSquare, toSquare, PAWN, KNIGHT, 0);

		promotions &= promotions - 1;
	}
}

static inline void black_generate_knight_pseudo(Board* board, MoveList* list){
	// KNIGHTS
	U64 knight = board->pieceBitboards[BLACK][KNIGHT];
	while (knight) {
		int square = get_lsb_index(knight);
		U64 attacks = get_knight_pseudo_moves(square, board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KNIGHT, 0, 0);

			attacks &= attacks - 1;
		}


		knight &= knight - 1;
	}
}

static inline void black_generate_bishop_pseudo(Board* board, MoveList* list){
	// BISHOPS
	U64 bishops = board->pieceBitboards[BLACK][BISHOP];
	while (bishops) {
		int square = get_lsb_index(bishops);
		U64 attacks = get_bishop_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, BISHOP, 0, 0);

			attacks &= attacks - 1;

		}

		bishops &= bishops - 1;
	}
}

static inline void black_generate_rook_pseudo(Board* board, MoveList* list){
	// ROOKS
	U64 rooks = board->pieceBitboards[BLACK][ROOK];
	while (rooks) {
		int square = get_lsb_index(rooks);
		U64 attacks = get_rook_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, ROOK, 0, 0);

			attacks &= attacks - 1;

		}

		rooks &= rooks - 1;
	}
}

static inline void black_generate_queen_pseudo(Board* board, MoveList* list){
	// QUEENS
	U64 queens = board->pieceBitboards[BLACK][QUEEN];
	while (queens) {
		int square = get_lsb_index(queens);
		U64 attacks = get_queen_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, QUEEN, 0, 0);

			attacks &= attacks - 1;

		}


		queens &= queens - 1;
	}
}

static inline void black_generate_king_pseudo(Board* board, MoveList* list){
	// KING
	U64 king = board->pieceBitboards[BLACK][KING];
	if (king) {
		int square = get_lsb_index(king);
		U64 attacks = get_king_pseudo_moves(square, board->occupiedBitboards[BLACK]);

		while (attacks) {
			int targetSquare = get_lsb_index(attacks);

			add_move_to_list(list, square, targetSquare, KING, 0, 0);

			attacks &= attacks - 1;

		}

	}
}

static inline void black_generate_castling_legal(Board* board, MoveList* list){

	if (board->castlingPerms & BKCA) {
		U64 castlingBitMask = CastlingBlockers[2] & board->occupiedBitboards[BOTH];

		if (castlingBitMask == 0) {
			if (!is_square_attacked(board, E8, WHITE) && !is_square_attacked(board, F8, WHITE) &&
				!is_square_attacked(board, G8, WHITE)) {
				add_move_to_list(list, E8, G8, KING, 0, 0);

			}
		}

	}

	if (board->castlingPerms & BQCA) {
		U64 castlingBitMask = CastlingBlockers[3] & board->occupiedBitboards[BOTH];
		
		if (castlingBitMask == 0) {
			if (!is_square_attacked(board, E8, WHITE) && !is_square_attacked(board, D8, WHITE) &&
				!is_square_attacked(board, C8, WHITE)) {
				add_move_to_list(list, E8, C8, KING, 0, 0);

			}
		}

	}
}

static inline void black_generate_enp_pseudo(Board* board, MoveList* list){
	if (board->enPassant != -1) {
		U64 enPassantSquare = 1ULL << board->enPassant;

		U64 attacksToEnPassant = (no_ea_pawns(enPassantSquare) | no_we_pawns(enPassantSquare)) & board->pieceBitboards[BLACK][PAWN];

		while (attacksToEnPassant) {
			int fromSquare = get_lsb_index(attacksToEnPassant);

			add_move_to_list(list, fromSquare, board->enPassant, PAWN, 0, true);

			attacksToEnPassant &= attacksToEnPassant - 1;
		}

	}
}


void white_generate_pseudo_moves(Board *board, MoveList *list) {
	// Generate a moveList entry for any possible
	// pseudo legal move from current board position.
	white_generate_pawn_pseudo(board, list);
	white_generate_knight_pseudo(board, list);
	white_generate_bishop_pseudo(board, list);
	white_generate_rook_pseudo(board, list);
	white_generate_queen_pseudo(board, list);
	white_generate_king_pseudo(board, list);


	// Handle castling and enPassant. Note that castling
	// is checked to be legal, so to not pass illegal castling
	// position and speed calculations.
	white_generate_castling_legal(board, list);
	white_generate_enp_pseudo(board, list);
}

void black_generate_pseudo_moves(Board* board, MoveList* list) {
	// For non specified code explanations, see "white_generate_pseudo_moves"
	// It offers the same code, with more detailed info, just mirrored
	// to the white side.

	black_generate_pawn_pseudo(board, list);
	black_generate_knight_pseudo(board, list);
	black_generate_bishop_pseudo(board, list);
	black_generate_rook_pseudo(board, list);
	black_generate_queen_pseudo(board, list);
	black_generate_king_pseudo(board, list);
	

	black_generate_castling_legal(board, list);
	black_generate_enp_pseudo(board, list);
}

bool is_square_attacked(Board* board, int square, SelectionColor attackingSide) {
	SelectionColor defending = 1-attackingSide;

	// Other side pawns
	U64 pawns = board->pieceBitboards[attackingSide][PAWN];

	if (pawnSingleAttacks[defending][square] & pawns) return true;

	U64 knights = board->pieceBitboards[attackingSide][KNIGHT];
	if (knightAttackSquares[square] & knights) return true;

	U64 bishopQueens = board->pieceBitboards[attackingSide][BISHOP] | board->pieceBitboards[attackingSide][QUEEN];
	if (bishopQueens && (get_bishop_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[defending])
		& bishopQueens)) return true;

	U64 rookQueens = board->pieceBitboards[attackingSide][ROOK] | board->pieceBitboards[attackingSide][QUEEN];
	if (rookQueens && (get_rook_pseudo_moves(square, board->occupiedBitboards[BOTH], board->occupiedBitboards[defending])
		& rookQueens)) return true;

	U64 king = board->pieceBitboards[attackingSide][KING];
	if (kingAttackSquares[square] & king) return true;

	return false;
}