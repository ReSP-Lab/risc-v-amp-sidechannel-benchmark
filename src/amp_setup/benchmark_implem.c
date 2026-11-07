/*
* This file generates benchmark binary based on provided compilation flags
*
* Copyright (C) 2024-2025 Cédrick Austa <cedrick.austa@ulb.be>
* Copyright (C) 2020 Shuwen Deng <shuwen.deng@yale.edu>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*
*/

#include "amp_setup/benchmark_implem.h"
#include "vulnerabilities.h"

#define AMP_ARRAY_ROWS          6ULL
#define AMP_BYTES_PER_SAMPLE    (AMP_ARRAY_ROWS * AMP_U64_BYTES)

#if NUM_SAMPLES > (OUTPUT_DATA_SIZE / AMP_BYTES_PER_SAMPLE)
#error "Timing arrays exceed AMP output region"
#endif
#if MAX_CYCLE > (OUTPUT_DATA_SIZE / (AMP_U32_BYTES * PROBE_SIZE))
#error "Histogram exceeds AMP output region"
#endif

#if MAX_ARRAY_SIZE > PROBE_REGION_SIZE
#error "AMP probe region overflow"
#endif

#define HISTOGRAM_BASE          OUTPUT_DATA_BASE
#define HISTOGRAM_SIZE          (AMP_U32_BYTES*PROBE_SIZE*MAX_CYCLE)
#define ARR_BASE                (OUTPUT_DATA_BASE+HISTOGRAM_SIZE)
#define ARR_SIZE                (AMP_BYTES_PER_SAMPLE*NUM_SAMPLES)
#define T_BASE                  (ARR_BASE+ARR_SIZE)
#define T_R_BASE                (T_BASE+ AMP_U64_BYTES*T_SIZE)
#define SUM_BASE                (T_R_BASE+ AMP_U64_BYTES*T_R_SIZE)
#define COUNTER_BASE            (SUM_BASE+ AMP_U64_BYTES*PROBE_SIZE)
#define M10_BASE                (COUNTER_BASE+ AMP_U64_BYTES*PROBE_SIZE)

#if START_SIZE > POINTERS_SIZE / AMP_PTR_BYTES
#error "AMP start pointer array exceeds pointer region"
#elif START_LLC_SIZE > (POINTERS_SIZE / AMP_PTR_BYTES) - START_SIZE
#error "AMP pointer arrays exceed pointer region"
#endif
#if (HISTOGRAM_SIZE % AMP_U64_BYTES) != 0
#error "Histogram size must be a multiple of 8 bytes to preserve alignment" \
    "(misalignment occur due to odd MAX_CYCLE values)"
#endif

#define OUTPUT_USED \
    (M10_BASE - OUTPUT_DATA_BASE + AMP_U64_BYTES * PROBE_SIZE)
#if OUTPUT_USED > OUTPUT_DATA_SIZE
#error "Complete AMP output layout exceeds reserved region"
#endif



#define PROGRESS_NONE           0x00
#define PROGRESS_INIT_AMP       0x10
#define PROGRESS_INIT_PTRS      0x11
#define PROGRESS_TESTS          0x20
#define PROGRESS_TESTS_1        0x21
#define PROGRESS_TESTS_2        0x22
#define PROGRESS_TESTS_3        0x23
#define PROGRESS_TESTS_4        0x24
#define PROGRESS_TESTS_5        0x25
#define PROGRESS_TESTS_6        0x26
#define PROGRESS_TESTS_7        0x27
#define PROGRESS_TESTS_8        0x28
#define PROGRESS_TESTS_9        0x29
#define PROGRESS_POSTPROC       0x30
#define PROGRESS_END            0x40

#define NONE_RUN                0x00
#define LOCAL_HART_RUN          0x10
#define REMOTE_HART_RUN         0x20

#if LOCAL_ATT && LOCAL_VIC && AFFINITY_REL
#define L_ATT_HART      LOCAL_HART2  // for SMT, with AFFINITY_REL=1
#else
#define L_ATT_HART      LOCAL_HART1  // otherwise
#endif
#if REMOTE_ATT && REMOTE_VIC && AFFINITY_REL
#define R_ATT_HART      REMOTE_HART2  // for SMT, with AFFINITY_REL=1
#else
#define R_ATT_HART      REMOTE_HART1  // otherwise
#endif

#ifdef ATTACKER_L_1
#define HART_STEP_1     L_ATT_HART
#elif defined(VICTIM_L_1)
#define HART_STEP_1     LOCAL_HART2
#elif defined(ATTACKER_R_1)
#define HART_STEP_1     R_ATT_HART
#elif defined(VICTIM_R_1)
#define HART_STEP_1     REMOTE_HART2
#endif /* ifdef ATTACKER_L_1 */

#ifdef ATTACKER_L_2
#define HART_STEP_2     L_ATT_HART
#elif defined(VICTIM_L_2)
#define HART_STEP_2     LOCAL_HART2
#elif defined(ATTACKER_R_2)
#define HART_STEP_2     R_ATT_HART
#elif defined(VICTIM_R_2)
#define HART_STEP_2     REMOTE_HART2
#endif /* ifdef ATTACKER_L_2 */

#ifdef ATTACKER_L_3
#define HART_STEP_3     L_ATT_HART
#elif defined(VICTIM_L_3)
#define HART_STEP_3     LOCAL_HART2
#elif defined(ATTACKER_R_3)
#define HART_STEP_3     R_ATT_HART
#elif defined(VICTIM_R_3)
#define HART_STEP_3     REMOTE_HART2
#endif /* ifdef ATTACKER_L_3 */


#define WAITING_STATS   0x13
#define STATS_SENT      0x55
#define IGNORED         111
#define UNSUPPORTED     222



#define STEP0_ENTRY() \
    if(maintain_arr[0]==NOONE_RUN) \
    { \
        maintain_arr[0]=STEP0_RUN; \
        publish(maintain_arr); \
    } \
    while(maintain_arr[0]!=STEP0_RUN) \
        wait(5);

#define STEP0_EXIT() \
    maintain_arr[0]=STEP1_RUN; \
    publish(maintain_arr);

#define STEP1_ENTRY() \
    while(maintain_arr[0]!=STEP1_RUN) \
        wait(5);

#define STEP1_EXIT() \
    maintain_arr[0]=STEP2_RUN; \
    publish(maintain_arr);

#define STEP2_ENTRY() \
    while(maintain_arr[0]!=STEP2_RUN) \
        wait(5);

#define STEP2_EXIT() \
    maintain_arr[0] = STEP3_RUN; \
    publish(maintain_arr);

#define STEP3_ENTRY() \
    while(maintain_arr[0]!=STEP3_RUN) \
        wait(5);

#define STEP3_EXIT() \
    maintain_arr[0]=STEP0_RUN; \
    publish(maintain_arr);


/* VARIABLES */
uint64_t hartid;

typedef int bool;
enum { false, true };

const int line_size=CACHE_LINE;
int rand_chosen=RANDOM_NUM;

/* Output data pointers */
uint32_t* histogram = (uint32_t*) HISTOGRAM_BASE;
int64_t* arr = (int64_t*) ARR_BASE;
uint64_t* t = (uint64_t*) T_BASE;
uint64_t* t_r = (uint64_t*) T_R_BASE;
uint64_t* sum = (uint64_t*) SUM_BASE;
uint64_t* counter = (uint64_t*) COUNTER_BASE;
double* m10 = (double*) M10_BASE;

/* Probe pointers */
#define CHAIN_ARR_BASE          PROBE_REGION_BASE
#define START_BASE              (char**) (POINTERS_BASE)
char* chain_arr = (char*) CHAIN_ARR_BASE;  // size: MAX_ARRAY_SIZE
char** start = (char**) START_BASE;
char** start_LLC = (char**) START_BASE + START_SIZE;

/* AMP  pointers */
uint8_t* maintain_arr = (uint8_t*) (AMP_BUFFER_BASE+0x120);

#define AMP_METADATA_START  (AMP_BUFFER_BASE+0x128)

uint8_t* entity = (uint8_t*) AMP_METADATA_START;  // size: 3
uint8_t* vul = (uint8_t*) AMP_BUFFER_BASE+0x12c;  // size: 3
uint8_t* outcome = (uint8_t*) AMP_BUFFER_BASE+0x130;
uint8_t* plain = (uint8_t*) AMP_BUFFER_BASE+0x131;
uint8_t* u_last_step = (uint8_t*) AMP_BUFFER_BASE+0x132;
uint8_t* boundary_reached = (uint8_t*) AMP_BUFFER_BASE+0x133;
uint8_t* data_ready = (uint8_t*) AMP_BUFFER_BASE+0x134;

uint32_t* pattern = (uint32_t*) (AMP_BUFFER_BASE+0x140);
uint32_t* vul_num = (uint32_t*) (AMP_BUFFER_BASE+0x144);
uint32_t* num_samples = (uint32_t*) (AMP_BUFFER_BASE+0x148);
uint32_t* num_test = (uint32_t*) (AMP_BUFFER_BASE+0x150);
uint32_t* max_cycle = (uint32_t*) (AMP_BUFFER_BASE+0x154);
uint64_t* sum_offset = (uint64_t*) (AMP_BUFFER_BASE+0x158);
uint64_t* m10_offset = (uint64_t*) (AMP_BUFFER_BASE+0x160);
uint8_t* csv_metadata_offset = (uint8_t*) (AMP_BUFFER_BASE+0x168);

#define AMP_METADATA_END    (AMP_BUFFER_BASE+0x170)

uint8_t* progress_tracker = (uint8_t*) AMP_BUFFER_BASE+0x170;
int* rand_candidate = (int*) (AMP_BUFFER_BASE+0x178);
tu_blocks* tu_block_p = (tu_blocks*) (AMP_BUFFER_BASE+0x180);

int order_rand_seed;
int candidate_rand_seed;
int sign_rand_seed;



/* FUNCTIONS */

static inline void wait(int i)
{
    for (int j = 0; j < i; j++)
    {
        __asm__ __volatile__("nop");
    }
}

void update_progress(uint8_t progress)
{
    *progress_tracker = progress;
    //flush(progress_tracker);
}

void metadata_init()
{
    *pattern = (uint32_t) PATTERN;
    *vul_num = (uint32_t) VUL_N;
    entity[0] = ENTITY1;
    entity[1] = ENTITY2;
    entity[2] = ENTITY3;
    vul[0] = VUL1;
    vul[1] = VUL2;
    vul[2] = VUL3;
    *u_last_step = U_LAST_STEP;
    *data_ready = 0;
    *num_samples = (uint32_t) NUM_SAMPLES;
    *num_test = (uint32_t) NUM_TEST;
    *max_cycle = (uint32_t) MAX_CYCLE;
    *sum_offset = (uint64_t) (SUM_BASE - OUTPUT_DATA_BASE);
    *m10_offset = (uint64_t) (M10_BASE - OUTPUT_DATA_BASE);
    csv_metadata_offset[0] = IF_WRITE_1;
    csv_metadata_offset[1] = IF_WRITE_2;
    csv_metadata_offset[2] = IF_WRITE_3;
    csv_metadata_offset[3] = AFFINITY_REL;
#ifdef I_CACHE_ANALYSIS
    csv_metadata_offset[4] = 1;
#else
    csv_metadata_offset[4] = 0;
#endif /* ifdef I_CACHE_ANALYSIS */

#if IF_PLAIN == 0
    *plain = 0;
#else /* IF_PLAIN == 1 */
    *plain = 1;
#endif /* if IF_PLAIN == 0 */

    /* NOTE: i.e. when plain and the main 4 "for" loops reach
     * their boundary;  signal border case reached (breakline in output file)
     */
#if IF_PLAIN && IF_WRITE_1 && IF_WRITE_2 && IF_WRITE_3 && \
    AFFINITY_REL
    *boundary_reached = 1;
#else
    *boundary_reached = 0;
#endif

    publish_range((char*) AMP_METADATA_START, (char*) AMP_METADATA_END);
}

void initialization()
{
    memset(chain_arr, 0, MAX_ARRAY_SIZE*sizeof(char));
    initialize_probe_array(chain_arr);
    publish_range(chain_arr, chain_arr+MAX_ARRAY_SIZE*sizeof(char));

    memset(start, 0, START_SIZE*sizeof(char*));
    memset(start_LLC, 0, START_LLC_SIZE*sizeof(char*));

    //  initiate tar_block, untar_block
    tu_block_p->tar_block = 0;
    tu_block_p->untar_block = 0;

    memset(histogram, 0, HISTOGRAM_SIZE);  // type already taken into account
    memset(arr, 0, ARR_SIZE);  // type already taken into account
    memset(sum, 0, PROBE_SIZE*sizeof(uint64_t));
    memset(counter, 0, PROBE_SIZE*sizeof(uint64_t));
    memset(m10, 0, PROBE_SIZE*sizeof(double));

    *outcome = WAITING_STATS;
}

void init_icache()
{
    if (hartid == HART_STEP_3)
    {
        STEP0_ENTRY();
        STEP0_EXIT();
    }
    if (hartid == HART_STEP_1)
    {
        STEP1_ENTRY();
        fencei();
        STEP1_EXIT();
    }
    if (hartid == HART_STEP_2)
    {
        STEP2_ENTRY();
        fencei();
        STEP2_EXIT();
    }
    if (hartid == HART_STEP_3)
    {
        STEP3_ENTRY();
        fencei();
        STEP3_EXIT();
    }
}

void init_test()
{
    if (hartid == HART_STEP_3)
    {
        STEP0_ENTRY();
        STEP0_EXIT();
    }
    if (hartid == HART_STEP_1)
    {
        STEP1_ENTRY();
        init_procedure(start, start_LLC);
        STEP1_EXIT();
    }
    if (hartid == HART_STEP_2)
    {
        STEP2_ENTRY();
        init_procedure(start, start_LLC);
        STEP2_EXIT();
    }
    if (hartid == HART_STEP_3)
    {
        STEP3_ENTRY();
        init_procedure(start, start_LLC);
        STEP3_EXIT();
    }
}

void handle_unsupported_case()
{
    /*
     * NOTE: plain = 0: case that should be skipped (e.g.: no flush support)
     * -> will be considered in IF_PLAIN case
     * -> Unsupported, ""
     * NOTE: plain = 1: case that should be skipped (e.g.: no flush support)
     * -> will be considered now
     * -> Unsupported, "222\n"
     */
    *outcome = UNSUPPORTED;
}

void handle_ignored_case()
{
    /*
     * NOTE: plain = 0: case is undesired: local or remote CPUs are identical
     * -> will be considered in IF_PLAIN case
     * -> Ignored, ""
     * NOTE: plain = 1: case is undesired: local or remote CPUs are identical,
     * SMT runs are ignored
     * -> will be considered now
     * -> Ignored, "111\n"
     */
    *outcome = IGNORED;
}

void process_timings(int i, int j)
{
    uint64_t t_sum = 0;
    uint64_t _t;
    for (int t_cnt = 0; t_cnt<T_SIZE; t_cnt++)
    {
        _t = t[t_cnt];
        if (_t != 0)
        {
            t_sum += _t;

            if(_t<MAX_CYCLE)
                histogram[j*MAX_CYCLE+_t]++;
            else
                histogram[j*MAX_CYCLE+MAX_CYCLE-1]++;
        }
    }

    if (i >= SUMMARY_FIRST_SAMPLE && i < SUMMARY_END_SAMPLE &&
            t_sum < SUMMARY_MAX_SUM_CYCLES)
    {
        sum[j] += t_sum;
        counter[j]++;
    }

    /* original histogram update */
    //if(t_sum<MAX_CYCLE)
    //    histogram[j*MAX_CYCLE+t_sum]++;
    //else
    //    histogram[j*MAX_CYCLE+MAX_CYCLE-1]++;

    arr[j*NUM_SAMPLES+i] += (int64_t)(t_sum);

#if U_LAST_STEP == 1 /* for clarity */
    uint64_t t_r_sum = 0;
    for (int t_r_cnt = 0; t_r_cnt<T_R_SIZE; t_r_cnt++)
        t_r_sum += t_r[t_r_cnt];

    int64_t delta = (int64_t) t_r_sum - (int64_t) t_sum;
    arr[(j+PROBE_SIZE)*NUM_SAMPLES+i] += delta;
#endif /* if U_LAST_STEP == 1 */
}

void transmit_m10(int m)
{
#if IF_PLAIN == 0
    if (m==SUMMARY_END_SAMPLE-1)
    {
        for(int i=0; i < PROBE_SIZE; i++)
        {
            m10[i] = counter[i] ? (double)sum[i] / counter[i] : NAN;
        }
    }
#endif /* if IF_PLAIN == 0 */
}


int amp_main(int argc, char **argv)
{
    hartid = get_hartid();
    platform_init();

    if (hartid == HART_STEP_3)
    {
        metadata_init();
    }
#if SKIP_CASE == 1
    if (hartid == HART_STEP_3)
    {
        handle_unsupported_case();
    }
#elif AFFINITY_REL==1 && (LOCAL_CPU1 == LOCAL_CPU2 \
                          || REMOTE_CPU1 == REMOTE_CPU2)
    if (hartid == HART_STEP_3)
    {
        handle_ignored_case();
    }
#else /* SKIP_CASE == 0 */

    /* go further in the code */
    if (hartid == HART_STEP_3)
    {
        initialization();

        build_eviction_set(start, start_LLC, chain_arr);
        publish_range((char*) start, (char*) (start+START_SIZE));
        publish_range((char*) start_LLC, (char*) (start_LLC+START_LLC_SIZE));

        maintain_arr[0] = NOONE_RUN;
        publish(maintain_arr);
    }

#ifdef I_CACHE_ANALYSIS
    init_icache();
#endif /* ifdef I_CACHE_ANALYSIS */

    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        //probe = probe + 0x10*j;
        if (hartid == HART_STEP_3)
        {
            set_tar_untar_block(tu_block_p);
            publish((char*) tu_block_p);
            order_rand_seed = rng_rand()%(PROBE_SIZE*2) /*3*2*1 permutations*/;
            candidate_rand_seed = order_rand_seed % PROBE_SIZE;
            sign_rand_seed = order_rand_seed / PROBE_SIZE;
        }

        for (int nt = 0; nt < NUM_TEST; nt++)
        {
            /* try all "u" candidates: a, a_alias, NIB */
            for (int j = 0; j < PROBE_SIZE; ++j)
            {
                init_test();

                // all active harts shall wait the HART responsible of
                // step 3 to exit step 0 to continue
                if (hartid == HART_STEP_3)
                {
                    STEP0_ENTRY();
                    memset(t, 0, T_SIZE*sizeof(uint64_t));
                    memset(t_r, 0, T_R_SIZE*sizeof(uint64_t));
                    step0(i, j, start, tu_block_p, candidate_rand_seed,
                          sign_rand_seed, rand_candidate);
                    publish((char*) rand_candidate);
                    STEP0_EXIT();
                }
                if (hartid == HART_STEP_1)
                {
                    STEP1_ENTRY();
                    step1(i,*rand_candidate, start, tu_block_p);
                    STEP1_EXIT();
                }
                if (hartid == HART_STEP_2)
                {
                    STEP2_ENTRY();
                    step2(i,*rand_candidate, start, tu_block_p);
                    STEP2_EXIT();
                }
                if (hartid == HART_STEP_3)
                {
                    STEP3_ENTRY();
                    step3(i,*rand_candidate, start, t, t_r, tu_block_p);
                    process_timings(i,*rand_candidate);
                    STEP3_EXIT();
                }
            }
        }
        if (hartid == HART_STEP_3)
        {
            transmit_m10(i);
        }
    }
    if (hartid == HART_STEP_3)
    {
        *outcome = STATS_SENT;
    }
#endif /* SKIP_CASE == 1 */

    if (hartid != HART_STEP_3)
    {
        return EXIT_SUCCESS;
    }

    publish(outcome);
    publish_range((char*) OUTPUT_DATA_BASE, (char*)(m10+PROBE_SIZE));

    // Communicate end of execution, and that data is ready for extraction
    *data_ready = 1;
    publish(data_ready);

    return EXIT_SUCCESS;
}
