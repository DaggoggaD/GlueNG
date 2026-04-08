#include "../include/GlueNG.h"

void initialize() {
	// Initialize attacks for "jumping" pieces
	initialize_pawn_single_attack_squares();
	initialize_knight_attack_squares();
	initialize_king_attack_squares();

	// Initialize masks for sliding pieces
	initialize_rook_masks();
	initialize_bishop_masks();

	// Initialize "magic" bitboards for sliding pieces
	initialize_magic_bitboards();

	// Initialize evaluation masks
	init_evaluation_masks();

	// Initialize random number generator for transposition tables.
	ran_init(&rng, 1070322ULL);	
	init_random_keys();
	init_tt(STANDARD_HASH_SIZE_MB);

	// Initialize late move reduction table
	init_lmr_table();

}

#pragma warning(suppress : 6262)
int main()
{
    setbuf(stdout, NULL);
    initialize();
    
    uci_protocol_handler();

	return 0;
}