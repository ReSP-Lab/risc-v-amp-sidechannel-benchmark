#ifndef __TIMING_HISTOGRAM_UTILS_H
#define __TIMING_HISTOGRAM_UTILS_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include "rng.h"
#include "board.h"
#include "asm_functions.h"
#include "evict.h"

#ifndef AMP_SETUP
    #include <stdlib.h>
    #include <time.h>
#else
    #include "amp_setup/stdlib_substitute.h"
#endif // !AMP_SETUP


/* CONSTANTS */

/* All cases leading to the different 66 timings in the article */
#define L1_HIT_CLEAN 0
#define L2_HIT_CLEAN 1
#define L3_HIT_CLEAN 2
#define REMOTE_L1_HIT_CLEAN 3
#define REMOTE_L2_HIT_CLEAN 4
#define REMOTE_L3_HIT_CLEAN 5

#define L1_HIT_DIRTY 6
#define L2_HIT_DIRTY 7
#define L3_HIT_DIRTY 8
#define REMOTE_L1_HIT_DIRTY 9
#define REMOTE_L2_HIT_DIRTY 10
#define REMOTE_L3_HIT_DIRTY 11
#define DRAM_HIT 12

#define L1_REMOTE_L1_HIT_CLEAN 13
#define L1_REMOTE_L2_HIT_CLEAN 14
#define L1_REMOTE_L3_HIT_CLEAN 15
#define L2_REMOTE_L1_HIT_CLEAN 16
#define L2_REMOTE_L2_HIT_CLEAN 17
#define L2_REMOTE_L3_HIT_CLEAN 18
#define L3_REMOTE_L1_HIT_CLEAN 19
#define L3_REMOTE_L2_HIT_CLEAN 20
#define L3_REMOTE_L3_HIT_CLEAN 21

#define READ 0
#define WRITE 1
#define FLUSH 2

#ifndef I_CACHE_ANALYSIS
#define MACCESS          mread
#define MACCESS_TIMING   mread_timing
#else
#define MACCESS          jmp
#define MACCESS_TIMING   jmp_timing
#endif /* ifndef I_CACHE_ANALYSIS */

/* MACROS */
/* Macros used for the experiments */
#ifdef NO_L3_CACHE
#if defined(L2_ASSOC)
#define NUM_CANDIDATES (SIZE_GT_FACTOR*L2_ASSOC-1)   // max. number of candidates
#endif
#else
#if defined(L3_ASSOC)
#define NUM_CANDIDATES (SIZE_GT_FACTOR*L3_ASSOC-1)   // max. number of candidates
#endif
#endif /* ifdef NO_L3_CACHE */
#define START_ARR_SIZE  (NUM_CANDIDATES+1)

#if defined(NO_FLUSH) && (defined(I_CACHE_ANALYSIS) || defined(NO_WRITE))
#define NUM_CALIBRE 22
#elif defined(NO_FLUSH) || (defined(I_CACHE_ANALYSIS) || defined(NO_WRITE))
#define NUM_CALIBRE 44
#else
#define NUM_CALIBRE 66
#endif /* ifdef NO_FLUSH */
#define NUM_CALIBRE_SINGLE 22

#define MAX_CYCLE 3000  // a reasonable maximum number of cycles

#define THRESHOLD   200
#define TEST_CNT    10000

/* FUNCTIONS */

/**
 * Shuffle the given array of given length.
 * It is performed in an undeterministic way, i.e. not reproducible.
 *
 * @param array an array to shuffle.
 * @param n the first `n` elements of the array to shuffle
 *  (should not exceed length).
 * @param length the length of the array.
 */
void shuffle(char **array, int n, int length);

/**
 * Initialize the array used for the delay measurement, i.e. `probe_arr`,
 * and the associated `probe` pointer.
 */
void initialize_probe_array(char** probe, char* probe_arr);

/**
 * Initialize the pointers arrays used for the delay measurement,
 * i.e. the `start` arrays.
 */
void initialize_pointer_arrays(char* probe, char** start, char** start_L1
#ifndef NO_L2_CACHE
        , char** start_L2
#endif /* ifndef NO_L2_CACHE */
#ifndef NO_L3_CACHE
        , char** start_L3
#endif /* ifndef NO_L3_CACHE */
);

/**
 * Create a shuffled eviction set for the Last Level Cache.
 */
void create_shuffled_eviction_set(char** start, char** lines);


/**
 * Probe an array:
 * - first, load `candidate` from memory into register;
 * - second, load each element of `set` from memory into register;
 * - finally, measure timing to load `candidate` from memory again.
 * This probe function was provided by:
 *     F. Liu, Y. Yarom, Q. Ge, G. Heiser and R. B. Lee,
 *     "Last-Level Cache Side-Channel Attacks are Practical,"
 *     2015 IEEE Symposium on Security and Privacy,
 *     San Jose, CA, USA, 2015,
 *     pp. 605-622, doi: 10.1109/SP.2015.43.
 *
 * The goal of this function is to identify if a given set leads
 * to a conflict for a specific candidate.
 *
 * @param set  a set to use to probe the candidate.
 * @param size  the number of elements to probe from the set.
 * @param candidate  the address which should be evicted.
 *
 * @return The probing result: true if a cache miss occurs, false otherwise.
 */
int probe_array(char* set[], int size, char* candidate);

/**
 * Build the conflict set, i.e., a union of eviction sets.
 * It is using the algorithm, without partitioning part, provided by:
 *     F. Liu, Y. Yarom, Q. Ge, G. Heiser and R. B. Lee,
 *     "Last-Level Cache Side-Channel Attacks are Practical,"
 *     2015 IEEE Symposium on Security and Privacy,
 *     San Jose, CA, USA, 2015,
 *     pp. 605-622, doi: 10.1109/SP.2015.43.
 * However, no pruning is performed on the eviction set.
 */
void build_conflict_set(char* conflict_set[], char* lines[]);

void load_L1(char* start[]);
void evict_L1(char* start[], char* start_L1[]);
void evict_L2(char* start_L2[]);
void flush_from_cache(char* start[], char* start_L1[], char* start_LLC[]);

void load_hybrid_L1(char* start[], int type);
void evict_local_L1(char* start[], char* start_L1[], int type);
void evict_local_L2(char* start_L2[], int type);
void evict_remote_L1(char* start[], char* start_L1[], int type);
void evict_remote_L2(char* start_L2[], int type);

void exclusive_load_L1(char* start[], int type);

#endif // !__TIMING_HISTOGRAM_UTILS_H
