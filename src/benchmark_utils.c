#include "benchmark_utils.h"

void initialize_probe_array(char* chain_arr)
{
    // initialize each pointer from the probe array with its index
    // -> each pointer is pointing to a byte address being equal
    //    to its current index
#ifndef I_CACHE_ANALYSIS
    for(int i =0 ; i < MAX_ARRAY_SIZE; i++) /*chain_arr*/
        chain_arr[i]=/*(char)*/ MAX_ARRAY_SIZE-i; //copy on write
#else
    for (int i = 0; i < MAX_ARRAY_SIZE*sizeof(char) ; i+=2)
    {
        *(chain_arr+i) = 0x82; // LSB part
        *(chain_arr+i+1) = 0x80; // MSB part
    }
    fencei();
#endif /* ifndef I_CACHE_ANALYSIS */
}

void init_procedure(char** start, char** start_LLC)
{
#if !defined(NO_FLUSH) && !defined(NO_LLC_FLUSH)
#ifdef I_CACHE_ANALYSIS
/* in this case, evict from all cache levels since L1i may not be coherent
 * with other caches and the L2 cache may not be inclusive with it too */
    evict(&start[4*T_STRIDE], L1_C, L1_D, L1_L, L1_S, MACCESS);
    evict_offset(start, T_STRIDE, NIB_OFFSET, L1_C, L1_D, L1_L, L1_S,
            MACCESS);
#endif /* ifdef I_CACHE_ANALYSIS */

    for (int k = 0; k < PROBE_SIZE*T_STRIDE; k++)
    {
        mflush_mfenced(start[k]);
    }
    for (int k = 0; k < T_STRIDE; k++)
    {
        mflush_mfenced(start[k]+NIB_OFFSET);
    }
#else  /* no "all cache levels, per address" flush instruction */

#ifdef L1_FLUSH_INSN
    for (int k = 0; k < PROBE_SIZE*T_STRIDE; k++)
    {
        l1_flush(start[k]);
    }
    for (int k = 0; k < T_STRIDE; k++)
    {
        l1_flush(start[k]+NIB_OFFSET);
    }
#else
/* evicting L1 may also result in cleaner L1d cache, so it is applied in any
 * case, even though LLC eviction might be sufficient
 */
    /* we need to independently evict all candidates since they do not
     * necessarily map to the same LLC cache set */
    // evict A, A_ALIAS, and D candidates
    evict(&start[4*T_STRIDE], L1_C, L1_D, L1_L, L1_S, MACCESS);
    evict_offset(start, T_STRIDE, NIB_OFFSET, L1_C, L1_D, L1_L, L1_S,
            MACCESS);
#endif /* ifdef L1_FLUSH_INSN */

#ifdef L2_FLUSH_INSN
/* we prefer not to use L2_FLUSH_INSN here when operation flushes the
 * complete L2 cache */
    for (int k = 0; k < PROBE_SIZE*T_STRIDE; k++)
    {
        l2_flush(start[k]);
    }
    for (int k = 0; k < T_STRIDE; k++)
    {
        l2_flush(start[k]+NIB_OFFSET);
    }
#else
    /* we need to independently evict all candidates since they do not
     * necessarily map to the same LLC cache set */
    // evict A, A_ALIAS, and D candidates
    for (int k = 0; k < PROBE_SIZE*T_STRIDE; k++)
    {
        evict_offset(start_LLC, 0, k*L1_STRIDE,
                LLC_C, LLC_D, LLC_L, LLC_S, MACCESS);
    }
    // evict candidate NIB
    for (int k = 0; k < T_STRIDE; k++)
    {
        evict_offset(start_LLC, 0, k*L1_STRIDE+NIB_OFFSET ,
                LLC_C, LLC_D, LLC_L, LLC_S, MACCESS);
    }
#endif /* ifdef L2_FLUSH_INSN */
    fence_iorwiorw();
#endif /* ifndef NO_FLUSH */
    /* access-based */
    //// load into L1 L2
    //for(int i=0;i<PROBE_SIZE*T_STRIDE;i++)
    //{
    //    mread_mfenced(start[i]); // tar_block = 0
    //}
    //for(int i=0;i<T_STRIDE;i++)
    //{
    //    mread_mfenced(start[i]+NIB_OFFSET); // untar_block = 0
    //}
}

void build_eviction_set(char** start, char** start_LLC, char* chain_arr)
{
    // initialize eviction set
    for(int i=0;i< START_SIZE;i++)
    {
        start[i]=&chain_arr[ i*L1_STRIDE ];
    }

    // create LLC eviction set to get cold baseline
    for(int i=0; i < START_LLC_SIZE; i++)
    {
        start_LLC[i]=&(chain_arr + (LLC_POOL_OFFSET + LLC_ALIGN_CORR)
                )[ i*LLC_STRIDE ];
    }
    int j; char* tmp_ptr;
    for (int i=0; i < START_LLC_SIZE; i++)
    {
        j = rng_rand() % START_LLC_SIZE;
        tmp_ptr = start_LLC[i];
        start_LLC[i] = start_LLC[j];
        start_LLC[j] = tmp_ptr;
    }

    init_procedure(start, start_LLC);

    // leave time to load data
    uint64_t tmp = 0;
    for (int i = 0; i < 1000000; ++i)
    {
        tmp+=i; // no utility, except leaving time for loading
    }
}

void set_tar_untar_block(tu_blocks* tu_block_p)
{
    tu_block_p->tar_block = (rng_rand()%4)*8;
    tu_block_p->untar_block = (rng_rand()%4)*8;
}

void step0(int i, int j, char** start, tu_blocks* tu_block_p,
           int candidate_rand_seed, int sign_rand_seed, int* random_candidate)
{
    int direction = sign_rand_seed ? -1 : 1;
    *random_candidate = (candidate_rand_seed
                         + direction*j + PROBE_SIZE) % PROBE_SIZE;
    //set_tar_untar_block();
}

/**
 * First step from the three-step model.
 * The vulnerability type is declared before the execution of a
 * generic code part.
 */
void step1(int i, int j, char** start, tu_blocks* tu_block_p)
{
    int tar_block = tu_block_p->tar_block;
    int untar_block = tu_block_p->untar_block;

#define VUL_TYPE    VUL1

    /* GENERIC CODE - START */
#define IF_UNKNOWN 0

#if VUL_TYPE == U || VUL_TYPE == U_INV
#undef IF_UNKNOWN
#define IF_UNKNOWN 1
#endif /* if VUL_TYPE */

#if VUL_TYPE == A || VUL_TYPE == A_ALIAS || VUL_TYPE == D || VUL_TYPE == U
    ACCESS(IF_WRITE_1, IF_UNKNOWN, LAST_STEP, U_LAST_STEP, VUL_TYPE)

#elif VUL_TYPE==INV || VUL_TYPE==A_INV || VUL_TYPE==D_INV || VUL_TYPE==A_ALIAS_INV || VUL_TYPE==U_INV
    FLUSH(IF_WRITE_1, IF_UNKNOWN, LAST_STEP, U_LAST_STEP, VUL_TYPE)

#elif VUL_TYPE==PRE
    PRE_STEP(LAST_STEP)

#elif VUL_TYPE==STAR
    STAR_STEP(LAST_STEP)

#else
    #error "Error: Wrong input in step 1\n"
#endif /* if VUL_TYPE */
#undef IF_UNKNOWN

    /* GENERIC CODE - END */
#undef VUL_TYPE

    //if (j==1 && i==299 && m==30) printf("sched_getcpu_in_step_1 = %d\n", sched_getcpu());
}

/**
 * Second step from the three-step model.
 * The vulnerability type is declared before the execution of a
 * generic code part.
 */
void step2(int i, int j, char** start, tu_blocks* tu_block_p)
{
    int tar_block = tu_block_p->tar_block;
    int untar_block = tu_block_p->untar_block;

#if IF_PLAIN
        //
#else
#define VUL_TYPE    VUL2

        /* GENERIC CODE - START */
#define IF_UNKNOWN 0

#if VUL_TYPE == U || VUL_TYPE == U_INV
#undef IF_UNKNOWN
#define IF_UNKNOWN 1
#endif /* if VUL_TYPE */

#if VUL_TYPE == A || VUL_TYPE == A_ALIAS || VUL_TYPE == D || VUL_TYPE == U
        ACCESS(IF_WRITE_2, IF_UNKNOWN, LAST_STEP, U_LAST_STEP, VUL_TYPE)

#elif VUL_TYPE==INV || VUL_TYPE==A_INV || VUL_TYPE==D_INV || VUL_TYPE==A_ALIAS_INV || VUL_TYPE==U_INV
        FLUSH(IF_WRITE_2, IF_UNKNOWN, LAST_STEP, U_LAST_STEP, VUL_TYPE)

#elif VUL_TYPE==PRE
        PRE_STEP(LAST_STEP)

#elif VUL_TYPE==STAR
        STAR_STEP(LAST_STEP)

#else
        #error "Error: Wrong input in step 2\n"
#endif /* if VUL_TYPE */
#undef IF_UNKNOWN

        /* GENERIC CODE - END */
#undef VUL_TYPE

#endif /* if IF_PLAIN */

    //if (j==1 && i==299 && m==30) printf("sched_getcpu_in_step_2 = %d\n", sched_getcpu());
}

/**
 * Third and last step from the three-step model.
 * The vulnerability type is declared before the execution of a
 * generic code part. Then, measured timings are processed.
 */
void step3(int i, int j, char** start, uint64_t* t, uint64_t* t_r,
        tu_blocks* tu_block_p)
{
    int tar_block = tu_block_p->tar_block;
    int untar_block = tu_block_p->untar_block;

#define VUL_TYPE    VUL3
#undef  LAST_STEP
#define LAST_STEP   1

    /* GENERIC CODE - START */
#define IF_UNKNOWN 0

#if VUL_TYPE == U || VUL_TYPE == U_INV
#undef IF_UNKNOWN
#define IF_UNKNOWN 1
#endif /* if VUL_TYPE */

#if VUL_TYPE == A || VUL_TYPE == A_ALIAS || VUL_TYPE == D || VUL_TYPE == U
    ACCESS(IF_WRITE_3, IF_UNKNOWN, LAST_STEP, U_LAST_STEP, VUL_TYPE)

#elif VUL_TYPE==INV || VUL_TYPE==A_INV || VUL_TYPE==D_INV || VUL_TYPE==A_ALIAS_INV || VUL_TYPE==U_INV
    FLUSH(IF_WRITE_3, IF_UNKNOWN, LAST_STEP, U_LAST_STEP, VUL_TYPE)

#elif VUL_TYPE==PRE
    PRE_STEP(LAST_STEP)

#elif VUL_TYPE==STAR
    STAR_STEP(LAST_STEP)

#else
    #error "Error: Wrong input in step 3\n"
#endif /* if VUL_TYPE */
#undef IF_UNKNOWN

    /* GENERIC CODE - END */
#undef LAST_STEP
#undef VUL_TYPE

    //if (j==1 && i==299 && m==30) printf("sched_getcpu_in_step_3 = %d\n", sched_getcpu());
}
