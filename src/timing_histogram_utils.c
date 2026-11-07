#include "timing_histogram_utils.h"

int t = 0;

void shuffle(char **array, int n, int length)
{
    long i, j;
    char* tmp;
    rng_srand(time(NULL));
    for(i = 0; i < n; i++){
        j = rng_rand() % length;
        tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

void initialize_probe_array(char** probe, char* probe_arr)
{
    // initialize each pointer from the probe array with its index
    // -> each pointer is pointing to a byte address being equal
    //    to its current index
#ifndef I_CACHE_ANALYSIS
    for (int i = 0; i < SIZE_GT_LLC; ++i)
        probe_arr[i] = i;
#else
    for (uint64_t i = 0; i < SIZE_GT_LLC*sizeof(char) ; i+=2)
    {
        *(probe_arr+i) = 0x82; // LSB part
        *(probe_arr+i+1) = 0x80; // MSB part
    }
    fencei();
#endif /* ifndef I_CACHE_ANALYSIS */

    // probe initialization creates a double indirection:
    // - first level of indirection: it points to the beginning of probe_arr
    // - second level of indirection: each element of probe_arr is a pointer
    *probe = probe_arr;  // probe: char*, probe_arr: char []
}

void initialize_pointer_arrays(char* probe, char** start, char** start_L1
#ifndef NO_L2_CACHE
        , char** start_L2
#endif /* ifndef NO_L2_CACHE */
#ifndef NO_L3_CACHE
        , char** start_L3
#endif /* ifndef NO_L3_CACHE */
)
{
    for(int i=0; i < START_ARR_SIZE; i++)
    {
        // initialize each pointer from start array with
        // the address of the (i*l1_stride)th element from the
        // prob_arr array
        // -> segment bytes into blocks of "l1_stride" by
        //    only tracing these addresses
        start[i]=&probe[i*L1_STRIDE];
    }
    for(int i=0; i < START_ARR_SIZE; i++)
    {
        start_L1[i]=&probe[i*L1_STRIDE];
    }
#ifndef NO_L2_CACHE
    for(int i=0; i < START_ARR_SIZE; i++)
    {
        start_L2[i]=&probe[i*L2_STRIDE];
    }
#endif /* ifndef NO_L2_CACHE */
#ifndef NO_L3_CACHE
    for(int i=0; i < START_ARR_SIZE; i++)
    {
        start_L3[i]=&probe[i*L3_STRIDE];
    }
#endif /* ifndef NO_L3_CACHE */
}

void create_shuffled_eviction_set(char** start, char** lines)
{
    // create eviction set for LLC
    for (int i = 0; i < NUM_CANDIDATES; ++i)
    {
        lines[i]=start[i+1];
    }

    // randomize it
    shuffle(lines, NUM_CANDIDATES, NUM_CANDIDATES);
}

int probe_array(char* set[], int size, char *candidate)
{
    // read candidate
    MACCESS(candidate);

    // read l
    for (int i = 0; i < size; ++i)
    {
        MACCESS(set[i]);
    }

    // measure time to read candidate
    t = MACCESS_TIMING(candidate);

    return (t > THRESHOLD );
}

void build_conflict_set(char* conflict_set[], char* lines[])
{
    int conflict_bar = 0;
    for (int candidate = 0; candidate < NUM_CANDIDATES; ++candidate)
    {
        // For each candidate from `lines`, probe the candidate:
        // - conflict found if probing reveals a cache miss;
        // - otherwise, no conflict yet.
        // We only enter the if-statement if the cache has hit,
        // i.e. we need at least one more elements to have
        // conflicts.
        // It is necessary to not stop the loop in case
        // there are multiple slices.
        if(!(probe_array(conflict_set, conflict_bar, lines[candidate])))
        {
            // add evicting candidate to the conflict set
            conflict_set[conflict_bar++] = lines[candidate];
        }
    }
    //printf("conflict_bar=%d\n", conflict_bar);
    for (int i = 0; i < conflict_bar; ++i)
    {
        //printf("conflict_set[%d]=%p\n", i, conflict_set[i]);
    }
}


inline void load_L1(char* start[])
{
    MACCESS(start[0]);
}

inline void evict_L1(char* start[], char* start_L1[])
{
//#if defined(I_CACHE_ANALYSIS) && defined(FENCEI)
//    fencei();
//#else
#ifdef L1_FLUSH_INSN
    l1_flush(start[0]);
#else
    evict(start_L1, L1_C, L1_D, L1_L, L1_S, MACCESS);
#endif /* ifdef L1_FLUSH_INSN */
//#endif /* ifdef I_CACHE_ANALYSIS */
}

inline void evict_L2(char* start_L2[])
{
    evict(start_L2, L2_C, L2_D, L2_L, L2_S, MACCESS);
}

inline void flush_from_cache(char* start[],char* start_L1[],char* start_LLC[])
{
    // require the Zicbom extension or a processor specific instr.
#if !defined(NO_FLUSH) && !defined(NO_LLC_FLUSH)
#ifdef I_CACHE_ANALYSIS
/* for L1i cache, evict from all cache levels since L1i may not be coherent
 * with other caches and the L2 cache may not be inclusive with it too */
    evict(start_L1, L1_C, L1_D, L1_L, L1_S, MACCESS);
#endif /* ifdef I_CACHE_ANALYSIS */
    mflush(start[0]);
#else  /* no "all cache levels, per address" flush instruction */

#ifdef L1_FLUSH_INSN
    l1_flush(start[0]);
#else
/* for L1i cache, evict from all cache levels since L1i may not be coherent
 * with other caches and the L2 cache may not be inclusive with it too
 * evicting L1 may also result in cleaner L1d cache, so it is applied in any
 * case, even though LLC eviction might be sufficient
 */
    evict(start_L1, L1_C, L1_D, L1_L, L1_S, MACCESS);
#endif /* ifdef L1_FLUSH_INSN */
    fence_iorwiorw();

#ifdef L2_FLUSH_INSN
    l2_flush(start[0]);
#else
    evict(start_LLC, LLC_C, LLC_D, LLC_L, LLC_S, MACCESS);
#endif /* ifdef L2_FLUSH_INSN */
    fence_iorwiorw();
#endif /* ifndef NO_FLUSH */
}


void hybrid_load_L1(char* start[], int type)
{
    if (type == L1_REMOTE_L1_HIT_CLEAN
            || type == L1_REMOTE_L2_HIT_CLEAN
            || type == L1_REMOTE_L3_HIT_CLEAN
        || type == L2_REMOTE_L1_HIT_CLEAN
            || type == L2_REMOTE_L2_HIT_CLEAN
            || type == L2_REMOTE_L3_HIT_CLEAN
        || type == L3_REMOTE_L1_HIT_CLEAN
            || type == L3_REMOTE_L2_HIT_CLEAN
            || type == L3_REMOTE_L3_HIT_CLEAN)
    {
        // initially load start[0] in local and remote L1 for all hybrid cases
        load_L1(start);
    }
}

void evict_local_L1(char* start[], char* start_L1[], int type)
{
    if (type == L2_REMOTE_L1_HIT_CLEAN
            || type == L2_REMOTE_L2_HIT_CLEAN
            || type == L2_REMOTE_L3_HIT_CLEAN)
    {
        evict_L1(start, start_L1);
    }
}

void evict_local_L2(char* start_L2[], int type)
{
    if (type == L3_REMOTE_L1_HIT_CLEAN
            || type == L3_REMOTE_L2_HIT_CLEAN
            || type == L3_REMOTE_L3_HIT_CLEAN)
    {
        evict_L2(start_L2);
    }
}

void evict_remote_L1(char* start[], char* start_L1[], int type)
{
    if (type == L1_REMOTE_L2_HIT_CLEAN
            || type == L2_REMOTE_L2_HIT_CLEAN
            || type == L3_REMOTE_L2_HIT_CLEAN)
    {
        evict_L1(start, start_L1);
    }
}

void evict_remote_L2(char* start_L2[], int type)
{
    if (type == L1_REMOTE_L3_HIT_CLEAN
            || type == L2_REMOTE_L3_HIT_CLEAN
            || type == L3_REMOTE_L3_HIT_CLEAN)
    {
        evict_L2(start_L2);
    }
}

void exclusive_load_L1(char* start[], int type)
{
    // prepare for cache hit either by reading data...
    if (type == L1_HIT_CLEAN
            || type == L2_HIT_CLEAN
            || type == L3_HIT_CLEAN
        || type == REMOTE_L1_HIT_CLEAN
            || type == REMOTE_L2_HIT_CLEAN
            || type == REMOTE_L3_HIT_CLEAN
        || type == DRAM_HIT)
    {
        load_L1(start);
    }
    // or by writing dirty data
    else if(type == L1_HIT_DIRTY
                || type == L2_HIT_DIRTY
                || type == L3_HIT_DIRTY
            || type == REMOTE_L1_HIT_DIRTY
                || type == REMOTE_L2_HIT_DIRTY
                || type == REMOTE_L3_HIT_DIRTY)
    {
        mwrite_dirty(start[0]);
    }
}
