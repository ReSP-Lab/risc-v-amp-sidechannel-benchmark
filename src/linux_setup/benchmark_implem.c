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

#include "linux_setup/benchmark_implem.h"

#include "vulnerabilities.h"

#ifdef ENABLE_HPC
#include "enable_hpc.h"
#endif // ENABLE_HPC


#ifdef ATTACKER_L_1
#define CPU_A_L_1     1
#else
#define CPU_A_L_1     0
#endif /* ifdef ATTACKER_L_1 */
#ifdef VICTIM_L_1
#define CPU_V_L_1     1
#else
#define CPU_V_L_1     0
#endif /* ifdef VICTIM_L_1 */
#ifdef ATTACKER_R_1
#define CPU_A_R_1     1
#else
#define CPU_A_R_1     0
#endif /* ifdef ATTACKER_R_1 */
#ifdef VICTIM_R_1
#define CPU_V_R_1     1
#else
#define CPU_V_R_1     0
#endif /* ifdef VICTIM_R_1 */

#ifdef ATTACKER_L_2
#define CPU_A_L_2     1
#else
#define CPU_A_L_2     0
#endif /* ifdef ATTACKER_L_2 */
#ifdef VICTIM_L_2
#define CPU_V_L_2     1
#else
#define CPU_V_L_2     0
#endif /* ifdef VICTIM_L_2 */
#ifdef ATTACKER_R_2
#define CPU_A_R_2     1
#else
#define CPU_A_R_2     0
#endif /* ifdef ATTACKER_R_2 */
#ifdef VICTIM_R_2
#define CPU_V_R_2     1
#else
#define CPU_V_R_2     0
#endif /* ifdef VICTIM_R_2 */

#ifdef ATTACKER_L_3
#define CPU_A_L_3     1
#else
#define CPU_A_L_3     0
#endif /* ifdef ATTACKER_L_3 */
#ifdef VICTIM_L_3
#define CPU_V_L_3     1
#else
#define CPU_V_L_3     0
#endif /* ifdef VICTIM_L_3 */
#ifdef ATTACKER_R_3
#define CPU_A_R_3     1
#else
#define CPU_A_R_3     0
#endif /* ifdef ATTACKER_R_3 */
#ifdef VICTIM_R_3
#define CPU_V_R_3     1
#else
#define CPU_V_R_3     0
#endif /* ifdef VICTIM_R_3 */



#define STEP0_ENTRY() \
    if(maintain_arr[0]==NOONE_RUN) \
    { \
        maintain_arr[0]=STEP0_RUN; \
    } \
    while(maintain_arr[0]!=STEP0_RUN) \
        sched_yield();

#define STEP0_EXIT() \
    maintain_arr[0]=STEP1_RUN;

#define STEP1_ENTRY() \
    while(maintain_arr[0]!=STEP1_RUN) \
        sched_yield();

#define STEP1_EXIT() \
    maintain_arr[0]=STEP2_RUN;

#define STEP2_ENTRY() \
    while(maintain_arr[0]!=STEP2_RUN) \
        sched_yield();

#define STEP2_EXIT() \
    maintain_arr[0] = STEP3_RUN;

#define STEP3_ENTRY() \
    while(maintain_arr[0]!=STEP3_RUN) \
        sched_yield();

#define STEP3_EXIT() \
    maintain_arr[0]=STEP0_RUN;


#define __SET_VAR_PER_SAMPLE_1() \
    set_tar_untar_block(tu_block_p); \
    order_rand_seed = rng_rand() % (PROBE_SIZE*2) /* 3*2*1 permutations */; \
    candidate_rand_seed = order_rand_seed % PROBE_SIZE; \
    sign_rand_seed = order_rand_seed / PROBE_SIZE;


#define __SET_VAR_PER_SAMPLE_0()

#define __SET_VAR_PER_SAMPLE(do_step3) \
    __SET_VAR_PER_SAMPLE_##do_step3()

#define set_var_per_sample(do_step3) \
    __SET_VAR_PER_SAMPLE(do_step3)


#define __INIT0_1() \
    STEP0_ENTRY(); \
    STEP0_EXIT();

#define __INIT0_0()

#define __INIT1_1() \
    STEP1_ENTRY(); \
    init_procedure(start, start_LLC); \
    STEP1_EXIT();

#define __INIT1_0()

#define __INIT2_1() \
    STEP2_ENTRY(); \
    init_procedure(start, start_LLC); \
    STEP2_EXIT();

#define __INIT2_0()

#define __INIT3_1() \
    STEP3_ENTRY(); \
    init_procedure(start, start_LLC); \
    STEP3_EXIT();

#define __INIT3_0()

#define __INIT_TEST(do_step1, do_step2, do_step3) \
    __INIT0_##do_step3() \
    __INIT1_##do_step1() \
    __INIT2_##do_step2() \
    __INIT3_##do_step3()

#define init_test(do_step1, do_step2, do_step3) \
    __INIT_TEST(do_step1, do_step2, do_step3)


#define __STEP0_1() \
    STEP0_ENTRY(); \
    memset(t, 0, T_SIZE*sizeof(size_t)); \
    memset(t_r, 0, T_R_SIZE*sizeof(size_t)); \
    step0(i, j, start, tu_block_p, candidate_rand_seed, sign_rand_seed, \
          rand_candidate); \
    STEP0_EXIT();

#define __STEP0_0()

#define __STEP1_1() \
    STEP1_ENTRY(); \
    step1(i,*rand_candidate, start, tu_block_p); \
    STEP1_EXIT();

#define __STEP1_0()

#define __STEP2_1() \
    STEP2_ENTRY(); \
    step2(i,*rand_candidate, start, tu_block_p); \
    STEP2_EXIT();

#define __STEP2_0()

#define __STEP3_1() \
    STEP3_ENTRY(); \
    step3(i,*rand_candidate, start, t, t_r, tu_block_p); \
    process_timings(i,*rand_candidate); \
    STEP3_EXIT();

#define __STEP3_0()

#define __STEPS(do_step1, do_step2, do_step3) \
    __STEP0_##do_step3() \
    __STEP1_##do_step1() \
    __STEP2_##do_step2() \
    __STEP3_##do_step3()

#define steps(do_step1, do_step2, do_step3) \
    __STEPS(do_step1, do_step2, do_step3)


#define WT_PROB 0.0005

typedef int bool;
enum { false, true };

const int line_size=CACHE_LINE;

char* file_name;
FILE *fp;
FILE *fp_res;
FILE *fp_csv;
enum { max_name_size = 100 };

uint64_t t[T_SIZE];
uint64_t t_r[T_R_SIZE];

int histogram[PROBE_SIZE][MAX_CYCLE]={0};

uint64_t sum[PROBE_SIZE] = {0};
uint64_t counter[PROBE_SIZE] = {0};

double arr[6][NUM_SAMPLES]={0};

int rand_chosen=RANDOM_NUM;

char* chain_arr;
//char* start[L1_ASSOC*CACHE_LINE*L1_CACHE_SETS];
//char* start[10*L1_S];
char** start;
char** start_LLC;

int* maintain_arr;
int status[4];

tu_blocks* tu_block_p;
int order_rand_seed;
int candidate_rand_seed;
int sign_rand_seed;
int* rand_candidate;



void initialize_arrays()
{
    // Map space for shared array
    chain_arr = mmap(0, PROBE_MAP_BYTES*sizeof(char),
#ifdef I_CACHE_ANALYSIS
        PROT_READ|PROT_WRITE|PROT_EXEC,
#else
        PROT_READ|PROT_WRITE,
#endif /* ifdef I_CACHE_ANALYSIS */
        MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_2MB, -1, 0);
    if (chain_arr == MAP_FAILED)
    {
        perror("mmap failed for chain_arr\n");
        exit(EXIT_FAILURE);
    }
    memset(chain_arr, 0, MAX_ARRAY_SIZE*sizeof(char));
    initialize_probe_array(chain_arr);

    start = mmap (0, START_SIZE*sizeof(char*), PROT_READ|PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (start == MAP_FAILED)
    {
        perror("mmap failed for start\n");
        exit(EXIT_FAILURE);
    }
    memset(start, 0, START_SIZE*sizeof(char*));

    start_LLC = mmap (0, START_LLC_SIZE*sizeof(char*), PROT_READ|PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (start_LLC == MAP_FAILED)
    {
        perror("mmap failed for start_LLC\n");
        exit(EXIT_FAILURE);
    }
    memset(start_LLC, 0, START_LLC_SIZE*sizeof(char*));

    //  initiate maintain_arr
    maintain_arr = mmap(0, 1*sizeof(int), PROT_READ|PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (maintain_arr == MAP_FAILED)
    {
        perror("mmap failed for maintain_arr\n");
        exit(EXIT_FAILURE);
    }
    //  initiate tar_block, untar_block
    tu_block_p = mmap(0, 1*sizeof(struct tu_blocks), PROT_READ|PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (tu_block_p == MAP_FAILED)
    {
        perror("mmap failed for tu_block_p\n");
        exit(EXIT_FAILURE);
    }
    tu_block_p->tar_block = 0;
    tu_block_p->untar_block = 0;

    rand_candidate = mmap(0, 1*sizeof(int), PROT_READ|PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (rand_candidate == MAP_FAILED)
    {
        perror("mmap failed for rand_candidate\n");
        exit(EXIT_FAILURE);
    }
    *rand_candidate = 0;
}

/**
 * Unmap the memory mappings.
 */
void free_mappings()
{
    if (munmap(chain_arr, PROBE_MAP_BYTES*sizeof(char)) == -1) {
        perror("munmap failed for chain_arr\n");
        exit(EXIT_FAILURE);
    }

    if (munmap(start, START_SIZE*sizeof(char)) == -1) {
        perror("munmap failed for start\n");
        exit(EXIT_FAILURE);
    }

    if (munmap(start_LLC, START_LLC_SIZE*sizeof(char)) == -1) {
        perror("munmap failed for start_LLC\n");
        exit(EXIT_FAILURE);
    }

    if (munmap(maintain_arr, 1*sizeof(int)) == -1) {
        perror("munmap failed for maintain_arr\n");
        exit(EXIT_FAILURE);
    }

    if (munmap(rand_candidate, 1*sizeof(int)) == -1) {
        perror("munmap failed for rand_candidate\n");
        exit(EXIT_FAILURE);
    }

    if (munmap(tu_block_p, 1*sizeof(struct tu_blocks)) == -1) {
        perror("munmap failed for tu_block_p\n");
        exit(EXIT_FAILURE);
    }
}

void process_timings(int i, int j)
{
    size_t t_sum = 0;
    uint64_t _t;
    for (int t_cnt = 0; t_cnt<T_SIZE; t_cnt++)
    {
        _t = t[t_cnt];
        if (_t != 0)
        {
            t_sum += _t;

            if(_t<MAX_CYCLE)
                histogram[j][_t]++;
            else
                histogram[j][MAX_CYCLE-1]++;
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
    //    histogram[j][t_sum]++;
    //else
    //    histogram[j][MAX_CYCLE-1]++;

    arr[j][i]+=(double)(t_sum);

#if U_LAST_STEP == 1 /* for clarity */
    size_t t_r_sum = 0;
    for (int t_r_cnt = 0; t_r_cnt<T_R_SIZE; t_r_cnt++)
        t_r_sum += t_r[t_r_cnt];

    double delta = (double) t_r_sum - (double) t_sum;
    arr[j+PROBE_SIZE][i]+= delta;
#endif /* if U_LAST_STEP == 1 */
}

void print_m10(int m)
{
#if IF_PLAIN == 0
    if (m==SUMMARY_END_SAMPLE-1)
    {
        for(int i=0; i< PROBE_SIZE; i++)
        {
            fprintf(fp, "%lf\t", counter[i] ? (double)sum[i]/counter[i] : NAN);
        }
    }
#endif /* if IF_PLAIN == 0 */
}

static const char *step_operation(int step, int vul, int write)
{
    if (step == 2 && IF_PLAIN)
        return "SKIPPED";

    switch (vul)
    {
        case A: case A_ALIAS: case D: case U:
            if (write)
                return "WRITE_ACCESS";
#ifdef I_CACHE_ANALYSIS
            return "EXECUTE_ACCESS";
#else
            return "READ_ACCESS";
#endif

        case INV: case A_INV: case D_INV: case U_INV: case A_ALIAS_INV:
            return write ? "WRITE_INVALIDATION"
                         : "FLUSH_INVALIDATION";

        case PRE: case STAR:
            return "DYNAMIC";

        default:
            return "UNKNOWN";
    }
}

int append_arr_csv()
{
    static const char *candidate[PROBE_SIZE] = {"a", "a_alias", "NIB"};

    struct stat st;
    if (fstat(fileno(fp_csv), &st) != 0)
        return -1;

    if (st.st_size == 0)
    {
        fprintf(fp_csv, "vul,pattern,plain,affinity_rel,"
                "write1,write2,write3,"
                "step1,op1,step2,op2,step3,op3,"
                "u_last_step,sample,"
                "a_sum_cycles,a_delta_cycles,"
                "a_alias_sum_cycles,a_alias_delta_cycles,"
                "NIB_sum_cycles,NIB_delta_cycles\n");
    }

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        fprintf(fp_csv,
                "%d,%d,%d,%d,"
                "%d,%d,%d,"
                "%s_%s,%s," "%s_%s,%s," "%s_%s,%s,"
                "%d,%d," "%.0f,%.0f,%.0f,%.0f,%.0f,%.0f\n",
                VUL_N,PATTERN,IF_PLAIN,AFFINITY_REL,
                IF_WRITE_1, IF_WRITE_2, IF_WRITE_3,
                type_match[ENTITY1], type_match[VUL1],
                step_operation(1, VUL1, IF_WRITE_1),
                type_match[ENTITY2], type_match[VUL2],
                step_operation(2, VUL2, IF_WRITE_2),
                type_match[ENTITY3], type_match[VUL3],
                step_operation(3, VUL3, IF_WRITE_3),
                U_LAST_STEP, i,
                arr[0][i], arr[0+PROBE_SIZE][i],
                arr[1][i], arr[1+PROBE_SIZE][i],
                arr[2][i], arr[2+PROBE_SIZE][i]
        );
    }
    return fflush(fp_csv);
}

void timing_analysis()
{
    printf("cycles\ta\ta_alias\tNIB\n");
    for(int i=0;i<MAX_CYCLE;i++)
    {
        printf("%d\t%d\t%d\t%d\n", i,
                histogram[0][i], histogram[1][i], histogram[2][i]);
    }

    // start calculate the p value of mapped and not mapped data
    double pvalue_a_a_alias = Pvalue(
            arr[0], NUM_SAMPLES,
            arr[1], NUM_SAMPLES
    );
    double pvalue_a_NIB = Pvalue(
            arr[0], NUM_SAMPLES,
            arr[2], NUM_SAMPLES
    );
    double pvalue_a_alias_NIB = Pvalue(
            arr[1], NUM_SAMPLES,
            arr[2], NUM_SAMPLES
    );

    double pvalue_a_a_alias_2 = Pvalue(
            arr[3], NUM_SAMPLES,
            arr[4], NUM_SAMPLES
    );
    double pvalue_a_NIB_2 = Pvalue(
            arr[3], NUM_SAMPLES,
            arr[5], NUM_SAMPLES
    );
    double pvalue_a_alias_NIB_2 = Pvalue(
            arr[4], NUM_SAMPLES,
            arr[5], NUM_SAMPLES
    );

    fprintf(fp, "\n%d\t%s_%s\t%s_%s\t%s_%s\n", PATTERN,
            type_match[ENTITY1], type_match[VUL1],
            type_match[ENTITY2], type_match[VUL2],
            type_match[ENTITY3], type_match[VUL3]
    );
    fprintf(fp, "\n%f\t%f\t%f\t%f\t%f\t%f\n",
            pvalue_a_a_alias, pvalue_a_NIB, pvalue_a_alias_NIB,
            pvalue_a_a_alias_2, pvalue_a_NIB_2, pvalue_a_alias_NIB_2
    );

    // first, determine whether an attacker can observe different
    // timings depending on the address pointed by Vu
    uint8_t welch_cond1;
    welch_cond1 =
        (pvalue_a_a_alias<WT_PROB && pvalue_a_NIB<WT_PROB)
        ||(pvalue_a_a_alias<WT_PROB && pvalue_a_alias_NIB<WT_PROB)
        ||(pvalue_a_alias_NIB<WT_PROB && pvalue_a_NIB<WT_PROB);
    // then, determine if there already is a timing difference
    // between candidates for cache hits (second access)
    uint8_t welch_cond2;
    welch_cond2 =
        (pvalue_a_a_alias_2<WT_PROB && pvalue_a_NIB_2<WT_PROB)
        ||(pvalue_a_a_alias_2<WT_PROB && pvalue_a_alias_NIB_2<WT_PROB)
        ||(pvalue_a_alias_NIB_2<WT_PROB && pvalue_a_NIB_2<WT_PROB);

    /*
     * If the first condition is satisfied AND
     * (either the last step is not a U step
     *  OR the second condition is satisfied)
     * -> success
     */
    if (welch_cond1 && ((!U_LAST_STEP) || welch_cond2))
    {
#if IF_PLAIN == 0
        fprintf(fp, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t%s Succeed \n", file_name);
        fprintf(fp_res, "1");
#else /* IF_PLAIN == 1 */
        fprintf(fp, "%s Succeeds \n\n", file_name);
        fprintf(fp_res, "1\n");
#endif /* if IF_PLAIN == 0 */
    }
    /*
     * Otherwise -> fail
     */
    else
    {
#if IF_PLAIN == 0
        fprintf(fp, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t%s Fails \n", file_name);
        fprintf(fp_res, "0");
#else /* IF_PLAIN == 1 */
        fprintf(fp, "%s Fails \n\n", file_name);
        fprintf(fp_res, "0\n");
#endif /* if IF_PLAIN == 0 */
    }

    // NOTE: i.e. when plain and the main 4 "for" loops reach
    // their boundary

    // export arrays to csv
    if (append_arr_csv() != 0)
    {
        perror("CSV export");
        exit(EXIT_FAILURE);
    }
}

int linux_main(int argc, char **argv)
{
#ifdef ENABLE_HPC
    if (init_perf_struct() != EXIT_SUCCESS)
        exit(EXIT_FAILURE);
    set_perf_event();
#endif /* ifdef ENABLE_HPC */

    file_name = argv[0];

    if (argc < 2)
    {
        perror("missing argument: output file prefix");
        exit(EXIT_FAILURE);
    }

    int printed_n;
    char fp_name[max_name_size] = {0};
    printed_n = snprintf(fp_name, max_name_size, "%s.output", argv[1]);
    if (printed_n < 0 || printed_n >= max_name_size)
    {
        perror("too long argument: fp_name");
        exit(EXIT_FAILURE);
    }
    char fp_res_name[max_name_size] = {0};
    printed_n = snprintf(fp_res_name, max_name_size, "%s.res", argv[1]);
    if (printed_n < 0 || printed_n >= max_name_size)
    {
        perror("too long argument: fp_name");
        exit(EXIT_FAILURE);
    }
    char fp_csv_name[max_name_size] = {0};
    printed_n = snprintf(fp_csv_name, max_name_size, "%s.csv", argv[1]);
    if (printed_n < 0 || printed_n >= max_name_size)
    {
        perror("too long argument: fp_name");
        exit(EXIT_FAILURE);
    }

    fp = fopen(fp_name, "a");
    fp_res = fopen(fp_res_name, "a");
    fp_csv = fopen(fp_csv_name, "a");

#if SKIP_CASE == 1
#if IF_PLAIN == 0
    fprintf(fp, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t%s Unsupported \n", file_name);
    // case that should be skipped (e.g.: no flush support)
    // -> will be considered in IF_PLAIN case
#else /* IF_PLAIN == 1 */
    fprintf(fp, "%s Unsupported \n", file_name);
    // case that should be skipped (e.g.: no flush support)
    fprintf(fp_res, "222\n");
#endif /* if IF_PLAIN == 0 */
#elif AFFINITY_REL==1 && (LOCAL_CPU1 == LOCAL_CPU2 \
                      || REMOTE_CPU1 == REMOTE_CPU2)
#if IF_PLAIN == 0
    fprintf(fp, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t%s Ignored \n", file_name);
    // case is undesired: local or remote CPUs are identical
    // -> will be considered in IF_PLAIN case
#else /* IF_PLAIN == 1 */
    fprintf(fp, "%s Ignored \n", file_name);
    // case is undesired: local or remote CPUs are identical
    fprintf(fp_res, "111\n");
#endif /* if IF_PLAIN == 0 */
#else /* SKIP_CASE == 0 */
    /* go further in the code */

    /* get the number of cpu's */
    cpu_set_t mycpuset;
    int numcpu = sysconf( _SC_NPROCESSORS_ONLN );
    int cpu[5];
    cpu[L_1]=LOCAL_CPU1;
    cpu[L_2]=LOCAL_CPU2;
    cpu[R_1]=REMOTE_CPU1;
    cpu[R_2]=REMOTE_CPU2;
    cpu[REST]=REST_CPU;
    // get our CPU

    CPU_ZERO(&mycpuset);
    CPU_SET(cpu[REST], &mycpuset);
    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &mycpuset) == -1)
    {
        perror("sched_setaffinity");
    }
    //print_affinity();
    //printf("sched_getcpu_in_main = %d\n", sched_getcpu());

    memset(t, 0, T_SIZE*sizeof(size_t));
    memset(t_r, 0, T_R_SIZE*sizeof(size_t));

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        arr[0][i] = 0;
        arr[1][i] = 0;
        arr[2][i] = 0;
        arr[3][i] = 0;
        arr[4][i] = 0;
        arr[5][i] = 0;
    }

    int c;
    pid_t pid_n, pid_l_1, pid_l_2, pid_r_1, pid_r_2;
    uint64_t a, b, d, e;

    initialize_arrays();
    build_eviction_set(start, start_LLC, chain_arr);

    maintain_arr[0] = NOONE_RUN;

    // local attacker
    if ((pid_l_1 = fork()) == -1)
    {
        printf("Failed to fork process 1\n");
        exit(EXIT_FAILURE);
    }
    else if (pid_l_1 == 0)
    {
        CPU_ZERO(&mycpuset);

#if LOCAL_ATT && LOCAL_VIC
        CPU_SET(cpu[L_1+AFFINITY_REL], &mycpuset);
#else
        CPU_SET(cpu[L_1], &mycpuset);
#endif
        //set processor affinity
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &mycpuset) == -1)
        {
            perror("sched_setaffinity");
        }

#ifdef ENABLE_HPC
        if (init_perf_struct() != EXIT_SUCCESS)
            exit(EXIT_FAILURE);
        set_perf_event();
#endif /* ifdef ENABLE_HPC */


        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            //probe = probe + 0x10*j;
            set_var_per_sample(CPU_A_L_3);

            for (int nt = 0; nt < NUM_TEST; nt++)
            {
                /* try all "u" candidates: a, a_alias, NIB */
                for (int j = 0; j < PROBE_SIZE; ++j)
                {
                    init_test(CPU_A_L_1, CPU_A_L_2, CPU_A_L_3);
                    steps(CPU_A_L_1, CPU_A_L_2, CPU_A_L_3);
                }
            }
#ifdef ATTACKER_L_3
            print_m10(i);
#endif /* ifdef ATTACKER_L_3 */
        }
#ifdef ATTACKER_L_3
        timing_analysis();
#endif /* ifdef ATTACKER_L_3 */
        exit(EXIT_SUCCESS);
    }

    // local victim
    if ((pid_l_2 = fork()) == -1)
    {
        printf("Failed to fork process 2\n");
        exit(EXIT_FAILURE);
    }
    else if (pid_l_2 == 0)
    {
        CPU_ZERO(&mycpuset);
        CPU_SET(cpu[L_2], &mycpuset);
        //set processor affinity
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &mycpuset) == -1)
        {
           perror("sched_setaffinity");
        }

#ifdef ENABLE_HPC
        if (init_perf_struct() != EXIT_SUCCESS)
            exit(EXIT_FAILURE);
        set_perf_event();
#endif /* ifdef ENABLE_HPC */

        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            //probe = probe + 0x10*j;
            set_var_per_sample(CPU_V_L_3);

            for (int nt = 0; nt < NUM_TEST; nt++)
            {
                /* try all "u" candidates: a, a_alias, NIB */
                for (int j = 0; j < PROBE_SIZE; ++j)
                {
                    init_test(CPU_V_L_1, CPU_V_L_2, CPU_V_L_3);
                    steps(CPU_V_L_1, CPU_V_L_2, CPU_V_L_3);
                }
            }
#ifdef VICTIM_L_3
            print_m10(i);
#endif /* ifdef VICTIM_L_3 */
        }
#ifdef VICTIM_L_3
        timing_analysis();
#endif /* ifdef VICTIM_L_3 */
        exit(EXIT_SUCCESS);
    }

    // remote attacker
    if ((pid_r_1 = fork()) == -1)
    {
        printf("Failed to fork process 3\n");
        exit(EXIT_FAILURE);
    }
    else if (pid_r_1 == 0)
    {
        CPU_ZERO(&mycpuset);

#if REMOTE_ATT && REMOTE_VIC
        CPU_SET(cpu[R_1+AFFINITY_REL], &mycpuset);
#else
        CPU_SET(cpu[R_1] , &mycpuset);
#endif
        //set processor affinity
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &mycpuset) == -1)
        {
            perror("sched_setaffinity");
        }

#ifdef ENABLE_HPC
        if (init_perf_struct() != EXIT_SUCCESS)
            exit(EXIT_FAILURE);
        set_perf_event();
#endif /* ifdef ENABLE_HPC */


        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            //probe = probe + 0x10*j;
            set_var_per_sample(CPU_A_R_3);

            for (int nt = 0; nt < NUM_TEST; nt++)
            {
                /* try all "u" candidates: a, a_alias, NIB */
                for (int j = 0; j < PROBE_SIZE; ++j)
                {
                    init_test(CPU_A_R_1, CPU_A_R_2, CPU_A_R_3);
                    steps(CPU_A_R_1, CPU_A_R_2, CPU_A_R_3);
                }
            }
#ifdef ATTACKER_R_3
            print_m10(i);
#endif /* ifdef ATTACKER_R_3 */
        }
#ifdef ATTACKER_R_3
        timing_analysis();
#endif /* ifdef ATTACKER_R_3 */
        exit(EXIT_SUCCESS);
    }

    // remote victim
    if ((pid_r_2 = fork()) == -1)
    {
        printf("Failed to fork process 4\n");
        exit(EXIT_FAILURE);
    }
    else if (pid_r_2 == 0)
    {
        CPU_ZERO(&mycpuset);
        CPU_SET(cpu[R_2], &mycpuset);
        //set processor affinity
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &mycpuset) == -1)
        {
            perror("sched_setaffinity");
        }

#ifdef ENABLE_HPC
        if (init_perf_struct() != EXIT_SUCCESS)
            exit(EXIT_FAILURE);
        set_perf_event();
#endif /* ifdef ENABLE_HPC */


        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            //probe = probe + 0x10*j;
            set_var_per_sample(CPU_V_R_3);

            for (int nt = 0; nt < NUM_TEST; nt++)
            {
                /* try all "u" candidates: a, a_alias, NIB */
                for (int j = 0; j < PROBE_SIZE; ++j)
                {
                    init_test(CPU_V_R_1, CPU_V_R_2, CPU_V_R_3);
                    steps(CPU_V_R_1, CPU_V_R_2, CPU_V_R_3);
                }
            }
#ifdef VICTIM_R_3
            print_m10(i);
#endif /* ifdef VICTIM_R_3 */
        }
#ifdef VICTIM_R_3
        timing_analysis();
#endif /* ifdef VICTIM_R_3 */
        exit(EXIT_SUCCESS);
    }

    while (wait(NULL)>0);
    free_mappings();

#endif /* SKIP_CASE == 1 */

    // NOTE: i.e. when plain and the main 4 "for" loops reach
    // their boundary
#if IF_PLAIN && IF_WRITE_1 && IF_WRITE_2 && IF_WRITE_3 && \
    AFFINITY_REL
    fprintf(fp_res, "\n");
#endif

    fclose(fp);
    fclose(fp_res);
    fclose(fp_csv);
    return EXIT_SUCCESS;
}
