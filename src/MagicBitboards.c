#include "../include/MagicBitboards.h"

// MAGIC NUMBERS: used directly from 
// https://github.com/maksimKorzh/bbc/blob/master/src/bbc_nnue/bbc.c

MagicEntry RookMagics[BRD_SIZE];
MagicEntry BishopMagics[BRD_SIZE];
U64 RookMoves[BRD_SIZE][4096];
U64 BishopMoves[BRD_SIZE][512];

U64 RookMmagicNumbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

U64 BishopMagicNumbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};


int get_magic_index(U64 b, MagicEntry* entry) {
	U64 occupancies = b & entry->occupanciesMask;
	U64 index = (occupancies * entry->magicN) >> (64 - entry->indexBits);
	return index;
}

int count_active_bits(U64 b) {
	int count = 0;
	for (short i = 0; i < 64; i++)
	{
		if ((b >> i) & 1ULL) {
			count++;
		}
	}

	return count;
}

U64 set_occupancies_mapping(U64 mask, int patternID) {
	U64 occupancies = 0ULL;
	short indexes[64];
	short found = 0;


	// Search for the active bits in the mask and
    // store their indexes in an array
	for (short i = 0; i < 64; i++)
	{
		short currentBit = (mask >> i) & 1ULL;
		if (currentBit) {
			indexes[found] = i;
			found++;
		}
	}


    // According to the patternID, set the corresponding bits in the
    // occupancies bitboard:
	// this is done to populate the attack tables with all the possible 
	// blockers configurations for the given mask.
    // This is then used to find the magic index, hashing the configuration.
	for (int i = 0; i < found; i++) {
		
		if (patternID & (1 << i)) {
			occupancies |= (1ULL << indexes[i]);
		}

	}

	return occupancies;
}

U64 calc_rook_attack_rays(int square, U64 blockers) {
	U64 attacks = 0ULL;
	int rank;
	int file;
	int currIndex = 0;
	rank_file_from_index(square, &rank, &file);

	// North
	for (int r = rank + 1; r < 8; r++) {
		index_from_rank_file(r, file, &currIndex);
		
		attacks |= (1ULL << currIndex);

		if (blockers & (1ULL << currIndex)) break;
	}
	// South
	for (int r = rank - 1; r >= 0; r--) {
		index_from_rank_file(r, file, &currIndex);
		
		attacks |= (1ULL << currIndex);

		if (blockers & (1ULL << currIndex)) break;
	}
	// East
	for (int f = file + 1; f < 8; f++) {
		index_from_rank_file(rank, f, &currIndex);
		
		attacks |= (1ULL << currIndex);

		if (blockers & (1ULL << currIndex)) break;
	}
	// West
	for (int f = file - 1; f >= 0; f--) {
		index_from_rank_file(rank, f, &currIndex);

		attacks |= (1ULL << currIndex);
		
		if (blockers & (1ULL << currIndex)) break;
	}
	return attacks;
}

U64 calc_bishop_attack_rays(int square, U64 blockers) {
    U64 attacks = 0ULL;
    int rank;
    int file;
    int currIndex = 0;
    rank_file_from_index(square, &rank, &file);

    // North-East
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        index_from_rank_file(r, f, &currIndex);
        
        attacks |= (1ULL << currIndex);
        if (blockers & (1ULL << currIndex)) break;
    }
    // South-East
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        index_from_rank_file(r, f, &currIndex);
        
        attacks |= (1ULL << currIndex);
        if (blockers & (1ULL << currIndex)) break;
    }
    // South-West
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        index_from_rank_file(r, f, &currIndex);
        
        attacks |= (1ULL << currIndex);
        if (blockers & (1ULL << currIndex)) break;
    }
    // North-West
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        index_from_rank_file(r, f, &currIndex);
        
        attacks |= (1ULL << currIndex);
        if (blockers & (1ULL << currIndex)) break;
    }
    return attacks;
}

void initialize_magic_bitboards() {
    for (int square = 0; square < BRD_SIZE; square++) {

		// Rooks magic entries initialization
		RookMagics[square].occupanciesMask = rookOccupanciesMasks[square];
		RookMagics[square].magicN = RookMmagicNumbers[square];
		RookMagics[square].indexBits = count_active_bits(rookOccupanciesMasks[square]);

		// Bishops magic entries initialization
		BishopMagics[square].occupanciesMask = bishopOccupanciesMasks[square];
		BishopMagics[square].magicN = BishopMagicNumbers[square];
		BishopMagics[square].indexBits = count_active_bits(bishopOccupanciesMasks[square]);


		// Rooks attack tables initialization
		U64 rookMask = rookOccupanciesMasks[square];
		int activeBits = count_active_bits(rookMask);
		int maxPatterns = 1 << activeBits;


		// Cycle though all the possible blockers configurations for the given mask, 
        // and calculate the corresponding attack rays, storing them in the attack table.
        // (more info in the appropriate functions).
		for (int patternID = 0; patternID < maxPatterns; patternID++) {
            U64 rookBlockers = set_occupancies_mapping(rookMask, patternID);

			U64 magicIndex = get_magic_index(rookBlockers, &RookMagics[square]);
			RookMoves[square][magicIndex] = calc_rook_attack_rays(square, rookBlockers);
		}


		// Bishops attack tables initialization
		U64 bishopMask = bishopOccupanciesMasks[square];
		activeBits = count_active_bits(bishopMask);
		maxPatterns = 1 << activeBits;

		// Same as for rooks.
        for (int patternID = 0; patternID < maxPatterns; patternID++) {
            U64 bishopBlockers = set_occupancies_mapping(bishopMask, patternID);

            U64 magicIndex = get_magic_index(bishopBlockers, &BishopMagics[square]);
            BishopMoves[square][magicIndex] = calc_bishop_attack_rays(square, bishopBlockers);
        }

	}
}