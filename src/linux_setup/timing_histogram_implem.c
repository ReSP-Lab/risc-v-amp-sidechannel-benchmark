#include "linux_setup/timing_histogram_implem.h"

#ifdef ENABLE_HPC
#include "enable_hpc.h"
#endif // ENABLE_HPC

#include "errno.h"


/* VARIABLES */

/* Constants */
const char* cycles_str = "Cycles";
const char* calibre_mapping[] = {
    "L1_CLEAN",
    "L2_CLEAN",
    "L3_CLEAN",
    "REMOTE_L1_CLEAN",
    "REMOTE_L2_CLEAN",
    "REMOTE_L3_CLEAN",
    "L1_DIRTY",
    "L2_DIRTY",
    "L3_DIRTY",
    "REMOTE_L1_DIRTY",
    "REMOTE_L2_DIRTY",
    "REMOTE_L3_DIRTY",
    "DRAM",
    "L1_REMOTE_L1_CLEAN",
    "L1_REMOTE_L2_CLEAN",
    "L1_REMOTE_L3_CLEAN",
    "L2_REMOTE_L1_CLEAN",
    "L2_REMOTE_L2_CLEAN",
    "L2_REMOTE_L3_CLEAN",
    "L3_REMOTE_L1_CLEAN",
    "L3_REMOTE_L2_CLEAN",
    "L3_REMOTE_L3_CLEAN",
};

const char* timing_mapping[] = {
    "READ",
#if !defined(I_CACHE_ANALYSIS) && !defined(NO_WRITE)
    "WRITE",
#endif /* ifndef I_CACHE_ANALYSIS */
    "FLUSH"
};


/* Variables */

char* probe;
#ifndef MMAP_ARRAY
char __attribute__((aligned(4096))) probe_arr[SIZE_GT_LLC]; // ensure to have an array longer than LLC size
#else
char* probe_arr;
#endif /* ifndef MMAP_ARRAY */

char* start_L1[START_ARR_SIZE];
char* start_L2[START_ARR_SIZE];
char* start_L3[START_ARR_SIZE];
#ifndef NO_L3_CACHE
char** start_LLC = start_L3;
#elif !defined(NO_L2_CACHE)
char** start_LLC = start_L2;
#else
char** start_LLC = start_L1;
#endif /* ifndef NO_L3_CACHE */
char* start[START_ARR_SIZE];
char* conflict_set[NUM_CANDIDATES];
char* lines[NUM_CANDIDATES];

int histogram[NUM_CALIBRE][MAX_CYCLE]={0};
int coarse_histogram[NUM_CALIBRE][MAX_CYCLE/10]={0};

cpu_set_t mycpuset;

long long ave_time_cycle_arr[NUM_CALIBRE] =  {0};

int min_time_cycle_arr[NUM_CALIBRE] =  {0};
int min_time_freq_arr[NUM_CALIBRE] = {0};

char file_long[100];
char file_name[100];


/* FUNCTIONS */

#define SWITCH_TO_(local_or_remote)  \
        CPU_ZERO(&mycpuset); \
        CPU_SET(local_or_remote, &mycpuset); \
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &mycpuset) == -1) \
        { \
              perror("sched_setaffinity"); \
        }

unsigned long test_delay(char* start[], int type, int measure_type)
{
    unsigned long t = 0;

    /* ensure target line is not in cache */
    {
        flush_from_cache(start, start_L1, start_LLC);
        SWITCH_TO_(REMOTE_CPU1);

        flush_from_cache(start, start_L1, start_LLC);
        SWITCH_TO_(LOCAL_CPU1);
    }

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
        /* see functions for additional `type` related conditions */

        // initially load start[0] in local L1 for all hybrid cases
        load_L1(start);

        // ensure start[0] is in L2 but not in L1 (local)
        // (all "L2_REMOTE_*" cases)
        evict_local_L1(start, start_L1, type);

        // ensure start[0] is in L3 but not in L1 nor L2 (local)
        // (all "L3_REMOTE_*" cases)
        evict_local_L2(start_L2, type);

        /* if access from remote proc., change CPU to REMOTE CPU */
        SWITCH_TO_(REMOTE_CPU1);

        load_L1(start);

        // ensure start[0] is in L2 but not in L1 (remote)
        // (all "*_REMOTE_L2_*" cases)
        evict_remote_L1(start, start_L1, type);

        // ensure start[0] is in L3 but not in L1 nor L2 (remote)
        evict_remote_L2(start_L2, type);

        /* if access was from remote proc., change CPU back to LOCAL CPU */
        SWITCH_TO_(LOCAL_CPU1);
    }



    /* prepare for cache hit by reading data or by writing dirty data */
    if (type == REMOTE_L1_HIT_CLEAN || type == REMOTE_L1_HIT_DIRTY
            || type == REMOTE_L2_HIT_CLEAN || type == REMOTE_L2_HIT_DIRTY
            || type == REMOTE_L3_HIT_CLEAN || type == REMOTE_L3_HIT_DIRTY)
    {
        /* if access from remote proc., change CPU to REMOTE CPU */
        SWITCH_TO_(REMOTE_CPU1);

        // see function for additional `type` related conditions
        exclusive_load_L1(start, type);

        /* if access was from remote proc., change CPU back to LOCAL CPU */
        SWITCH_TO_(LOCAL_CPU1);
    }
    else
    {
        // see function for additional `type` related conditions
        exclusive_load_L1(start, type);
    }


    /* possible eviction to certain level of cache */
    // ensure start[0] is in L2 but not in L1 (local or remote)
    if (type == L2_HIT_CLEAN || type == L2_HIT_DIRTY
            || type == REMOTE_L2_HIT_CLEAN || type == REMOTE_L2_HIT_DIRTY)
    {
        if (type == REMOTE_L2_HIT_CLEAN || type == REMOTE_L2_HIT_DIRTY)
        {
            /* if access from remote proc., change CPU to REMOTE CPU */
            SWITCH_TO_(REMOTE_CPU1);

            evict_L1(start, start_L1);

            /* if access was from remote proc., change CPU back to LOCAL CPU */
            SWITCH_TO_(LOCAL_CPU1);
        }
        else
        {
            evict_L1(start, start_L1);
        }
    }
    // ensure start[0] is in L3 but not in L1 nor L2 (local or remote)
    else if(type == L3_HIT_CLEAN || type == L3_HIT_DIRTY
                || type == REMOTE_L3_HIT_CLEAN || type == REMOTE_L3_HIT_DIRTY)
    {
        /* if access from remote proc., change CPU to CPU 3 */
        if (type == REMOTE_L3_HIT_CLEAN || type == REMOTE_L3_HIT_DIRTY)
        {
            /* if access from remote proc., change CPU to REMOTE CPU */
            SWITCH_TO_(REMOTE_CPU1);

            evict_L2(start_L2);

            /* if access was from remote proc., change CPU back to LOCAL CPU */
            SWITCH_TO_(LOCAL_CPU1);
        }

        else
        {
            evict_L2(start_L2);
        }
    }
    else if (type == DRAM_HIT)
    {
        flush_from_cache(start, start_L1, start_LLC);
    }


    //measure latency
    // - for read
    if (measure_type == READ)
    {
        t = MACCESS_TIMING(start[0]);
    }
    // - for write
    else if (measure_type == WRITE)
    {
        t = mwrite_timing(start[0]);
    }
    // - for flush
    else if (measure_type == FLUSH)
    {
        t = mflush_timing(start[0]);
    }

    return t;
}


/**
 * Calibrate the test for a given memory operation and all possible
 * scenarii, and add results to the different histograms.
 *
 * @param counter_hist current index in histogram arrays;
 * @param operation type of memory operation.
 *
 * @return new histogram index.
 */
int calibrate(int counter_hist, int operation)
{
    unsigned int t;

    for (int ord_calibre = 0; ord_calibre < NUM_CALIBRE_SINGLE; ++ord_calibre)
    {
        printf("Generating histogram for timing type %d ...\n", counter_hist+1);

        // Ignore some scenarii if no L3 cache exist.
#ifdef NO_L3_CACHE
        if (ord_calibre == L3_HIT_CLEAN
                || ord_calibre == REMOTE_L3_HIT_CLEAN
                || ord_calibre == L3_HIT_DIRTY
                || ord_calibre == REMOTE_L3_HIT_DIRTY
                || ord_calibre == L1_REMOTE_L3_HIT_CLEAN
                || ord_calibre == L2_REMOTE_L3_HIT_CLEAN
                || ord_calibre == L3_REMOTE_L1_HIT_CLEAN
                || ord_calibre == L3_REMOTE_L2_HIT_CLEAN
                || ord_calibre == L3_REMOTE_L3_HIT_CLEAN)
        {
            ave_time_cycle_arr[counter_hist] = 0;
            counter_hist++;
            continue;
        }
#endif /* ifdef NO_L3_CACHE */

        // Ignore some scenarii if L1i cache is evaluated
#if defined(I_CACHE_ANALYSIS) || defined(NO_WRITE)
        if (ord_calibre == L1_HIT_DIRTY
                || ord_calibre == L2_HIT_DIRTY
                || ord_calibre == L3_HIT_DIRTY
                || ord_calibre == REMOTE_L1_HIT_DIRTY
                || ord_calibre == REMOTE_L2_HIT_DIRTY
                || ord_calibre == REMOTE_L3_HIT_DIRTY)
        {
            ave_time_cycle_arr[counter_hist] = 0;
            counter_hist++;
            continue;
        }
#endif /* ifdef I_CACHE_ANALYSIS */

        /* removed since we (try to) clear the cache before each measurement
        // Add dummy computation to make sure the data load
        // into L1/L2 from memory
        for(int j=0;j <100;j++)
        {
            t=test_delay(start, ord_calibre, operation);
            for (int i = 0; i < 1000000; ++i)
            {
                t+=i;
            }
        }
        */
        for(int i=0;i<TEST_CNT;i++)
        {
            t=test_delay(start, ord_calibre, operation);
            ave_time_cycle_arr[counter_hist] += t;
            if(t<MAX_CYCLE)
            {
                histogram[counter_hist][t]++;
                coarse_histogram[counter_hist][t/10]++;
            }
            else
            {
                histogram[counter_hist][MAX_CYCLE-1]++;
                coarse_histogram[counter_hist][MAX_CYCLE/10-1]++;
            }
        }
        // Update histogram index for each scenario.
        counter_hist++;
    }
    return counter_hist;
}

void print_to_files(FILE *fp, FILE* fp_coarse)
{
    // header
    // - CSV delimiter
    //fprintf(fp, "sep=,\n");
    //fprintf(fp_coarse, "sep=,\n");
    // - operation
    int timing_type;
    fprintf(fp, ",");
    fprintf(fp_coarse, ",");
    //fprintf(fp, "Timing type,");
    //fprintf(fp_coarse, "Timing type,");
    //fprintf(fp, "Operation,");
    //fprintf(fp_coarse, "Operation,");
    for (int j = 0; j < NUM_CALIBRE; ++j)
    {
        timing_type = j / NUM_CALIBRE_SINGLE;
        fprintf(fp, "%s,", timing_mapping[timing_type]);
        fprintf(fp_coarse, "%s,", timing_mapping[timing_type]);
    }
    fprintf(fp, "\n");
    fprintf(fp_coarse, "\n");
    // - calibre
    int calibre_type;
    fprintf(fp, "%s,", cycles_str);
    fprintf(fp_coarse, "%s,", cycles_str);
    for (int j = 0; j < NUM_CALIBRE; ++j)
    {
        calibre_type = j % NUM_CALIBRE_SINGLE;
        fprintf(fp, "%s,", calibre_mapping[calibre_type]);
        fprintf(fp_coarse, "%s,", calibre_mapping[calibre_type]);
    }
    fprintf(fp, "\n");
    fprintf(fp_coarse, "\n");

    // histogram
    for(int i=0;i<MAX_CYCLE;i++)
    {
        fprintf(fp, "%d,", i);
        for (int j = 0; j < NUM_CALIBRE; ++j)
        {
            fprintf(fp, "%d,", histogram[j][i]);
        }
        fprintf(fp, "\n");

        for (int j = 0; j < NUM_CALIBRE; ++j)
        {
            if (histogram[j][i]>min_time_freq_arr[j])
            {
                min_time_freq_arr[j] = histogram[j][i];
                min_time_cycle_arr[j] = i;
            }
        }
    }
    // coarse histogram
    for(int i=0;i<MAX_CYCLE/10;i++)
    {
        fprintf(fp_coarse, "%d,", i*10);
        for (int j = 0; j < NUM_CALIBRE; ++j)
        {
            fprintf(fp_coarse, "%d,", coarse_histogram[j][i]);
        }
        fprintf(fp_coarse, "\n");
    }
    for (int j = 0; j < NUM_CALIBRE; ++j)
    {
        ave_time_cycle_arr[j] = ave_time_cycle_arr[j]/TEST_CNT;
    }

    fclose(fp);
    fclose(fp_coarse);
}

void print_cli_summary(char* file_info)
{
    printf("The maximum frequency cycle number of %d types of timings are given below \n", NUM_CALIBRE);
    for (int j = 0; j < NUM_CALIBRE; ++j)
    {
        printf("%d,", min_time_cycle_arr[j]);
    }
    printf("\n");

    printf("The average cycle number of %d types of timings are given below \n", NUM_CALIBRE);
    for (int j = 0; j < NUM_CALIBRE; ++j)
    {
        printf("%d,", ave_time_cycle_arr[j]);
    }
    printf("\n");

    printf("Histogram numbers are output to "
           "%s and %s\n", file_long, file_name);
}

int linux_main(int argc, char *argv[])
{
#ifdef ENABLE_HPC
    if (init_perf_struct() != EXIT_SUCCESS)
        exit(EXIT_FAILURE);
    set_perf_event();
#endif /* ifdef ENABLE_HPC */

#ifdef MMAP_ARRAY
    probe_arr = mmap(0, PROBE_MAP_BYTES*sizeof(char),
#ifdef I_CACHE_ANALYSIS
        PROT_READ|PROT_WRITE|PROT_EXEC,
#else
        PROT_READ|PROT_WRITE,
#endif /* ifdef I_CACHE_ANALYSIS */
        MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_2MB, -1, 0);
    if (probe_arr == MAP_FAILED)
    {
        perror("mmap failed for probe_arr\n");
        exit(EXIT_FAILURE);
    }
#endif /* ifdef MMAP_ARRAY */

    // get our CPU: set current CPU to CPU G
    CPU_ZERO(&mycpuset);
    CPU_SET(LOCAL_CPU1, &mycpuset);
    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &mycpuset) == -1)
    {
        perror("sched_setaffinity");
    }

#ifndef I_CACHE_ANALYSIS
    sprintf(file_long, "histogram_output/smp_histogram_dcache_%s.csv",
            argv[1]);
#else
    sprintf(file_long, "histogram_output/smp_histogram_icache_%s.csv",
            argv[1]);
#endif /* ifndef I_CACHE_ANALYSIS */

#ifndef I_CACHE_ANALYSIS
    sprintf(file_name, "histogram_output/smp_coarse_histogram_dcache_%s.csv",
            argv[1]);
#else
    sprintf(file_name, "histogram_output/smp_coarse_histogram_icache_%s.csv",
            argv[1]);
#endif /* ifndef I_CACHE_ANALYSIS */
    char* file_info = argv[1];


    FILE *fp = fopen(file_long, "w");
    FILE *fp_coarse = fopen(file_name, "w");

    initialize_probe_array(&probe, probe_arr);

#ifndef NO_L3_CACHE
    initialize_pointer_arrays(probe, start, start_L1, start_L2, start_L3);
#elif !defined(NO_L2_CACHE)
    initialize_pointer_arrays(probe, start, start_L1, start_L2);
#else
    initialize_pointer_arrays(probe, start, start_L1);
#endif /* ifndef NO_L3_CACHE */
    create_shuffled_eviction_set(start, lines);


    /*
     * NOTE: used only when relying on conflict sets for eviction
     * -> should be adapted if needed
     */
    //build_conflict_set(conflict_set, lines);


    //--- CALIBRATION ---
    // calibration is performed for each type of operation: READ, WRITE, FLUSH
    int counter_hist=0;
    // READ
    counter_hist = calibrate(counter_hist, READ);
#if !defined(I_CACHE_ANALYSIS) && !defined(NO_WRITE)
    // WRITE
    counter_hist = calibrate(counter_hist, WRITE);
#endif /* ifndef I_CACHE_ANALYSIS */
#ifndef NO_FLUSH
    // FLUSH
    counter_hist = calibrate(counter_hist, FLUSH);
#endif /* ifndef NO_FLUSH */


    //--- PRINT RESULTS ---
    // to files
    print_to_files(fp, fp_coarse);
    // to cli
    print_cli_summary(file_info);

#ifdef MMAP_ARRAY
    if (munmap(probe_arr, PROBE_MAP_BYTES*sizeof(char)) == -1) {
        perror("munmap failed for probe_arr\n");
        exit(EXIT_FAILURE);
    }
#endif /* ifdef MMAP_ARRAY */

#ifdef ENABLE_HPC
    clean_perf_struct();
#endif /* ifdef ENABLE_HPC */
}
