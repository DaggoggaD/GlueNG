#include "GlueNG.h"

// LSF (least significant file) translation to index
void index_from_rank_file(const int rank, const int file, int *index)
{
	// Invalid index for out of bounds input
	if (rank < 0 || rank > 7 || file < 0 || file > 7) {
		fprintf(stderr, "Error: Invalid rank or file input. Rank and file must be between 0 and 7.\n");
		*index = -1; 
		return;
	}

	*index = rank * 8 + file;
}

// LSF little endian rank and file translation from index
void rank_file_from_index(const int index, int* rank, int* file)
{
	if (index < 0 || index > 63) {
		fprintf(stderr, "Error: Invalid index input. Index must be between 0 and 63.\n");
		*rank = -1; 
		*file = -1; 
		return;
	}

	*rank = index / 8;
	*file = index % 8;
}

// Adds a piece to ALL relevant bitboards. Checks for valid index input.
void add_piece_to_board(const PieceType piece, const SelectionColor color, int index, Board* board)
{
	if (index < 0 || index > 63) {
		fprintf(stderr, "Error: Invalid index input. Index must be between 0 and 63.\n");
		return;
	}

	board->pieceBitboards[color][piece] |= (1ULL << index);
	board->occupiedBitboards[color] |= (1ULL << index);
	board->occupiedBitboards[BOTH] |= (1ULL << index);
}

// Chebyshev distance between two squares, for king move generation and evaluation.
int chebyshev_distance(int square1, int square2) {
	int rank1, file1, rank2, file2;
	
	rank_file_from_index(square1, &rank1, &file1);
	rank_file_from_index(square2, &rank2, &file2);
	
	int rankDiff = (rank1 > rank2) ? rank1 - rank2 : rank2 - rank1;
	int fileDiff = (file1 > file2) ? file1 - file2 : file2 - file1;
	
	return (rankDiff > fileDiff) ? rankDiff : fileDiff;
}


void initialize() {
	initialize_knight_attack_squares();
	initialize_king_attack_squares();

	initialize_rook_masks();
	initialize_bishop_masks();

	initialize_magic_bitboards();
}


int main()
{
	initialize();
	Board board = { 0 };
	load_fen_board("4r3/P6P/8/8/8/8/8/R3K2R w KQ - 0 1", &board);
	debug_board_visualizer(&board);

	MoveList list = { 0 };
	white_generate_pseudo_moves(&board, &list);
	printf("Generated %d pseudo-legal moves for white:\n", list.count);
}