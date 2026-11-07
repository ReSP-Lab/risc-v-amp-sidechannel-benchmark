#include "amp_setup/timing_histogram_implem.h"

/* CONSTANTS */
#define MAX_NUM_CALIBRE         3*NUM_CALIBRE_SINGLE
#define HISTOGRAM_SIZE          NUM_CALIBRE*MAX_CYCLE
#define COARSE_HISTOGRAM_SIZE   NUM_CALIBRE*MAX_CYCLE/10

#define NONE_RUN                0x00
#define LOCAL_HART_RUN          0x10
#define REMOTE_HART_RUN         0x20

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

#define LOCAL_ENTRY() \
        wait_flag(LOCAL_HART_RUN);

#define LOCAL_EXIT() \
        switch_flag(REMOTE_HART_RUN);

#define REMOTE_ENTRY() \
       switch_flag(LOCAL_HART_RUN); \
       wait_flag(REMOTE_HART_RUN);

#define REMOTE_EXIT() \
        switch_flag(NONE_RUN);


/* VARIABLES */
uint64_t hartid;

//char* start_L1[START_ARR_SIZE]; // START_ARR_SIZE
//char* start_L2[START_ARR_SIZE]; // START_ARR_SIZE
//char* start_L3[START_ARR_SIZE]; // START_ARR_SIZE
#define START_BASE  (char**) POINTERS_BASE
#define START_L1    START_BASE
#define START_L2    START_BASE + START_ARR_SIZE
#define START_L3    START_BASE + 2*START_ARR_SIZE
char** start_L1 = START_L1; // START_ARR_SIZE
char** start_L2 = START_L2; // START_ARR_SIZE
char** start_L3 = START_L3; // START_ARR_SIZE
#ifndef NO_L3_CACHE
char** start_LLC = START_L3;
#elif !defined(NO_L2_CACHE)
char** start_LLC = START_L2;
#else
char** start_LLC = START_L1;
#endif /* ifndef NO_L3_CACHE */
//char* start[START_ARR_SIZE]; //START_ARR_SIZE
//char* conflict_set[NUM_CANDIDATES]; //NUM_CANDIDATES
//char* lines[NUM_CANDIDATES]; //NUM_CANDIDATES
char** start = START_L3 + START_ARR_SIZE; //START_ARR_SIZE
char** conflict_set = START_L3 + 2*START_ARR_SIZE; //NUM_CANDIDATES
char** lines = START_L3 + 2*START_ARR_SIZE + NUM_CANDIDATES; //NUM_CANDIDATES


char* probe;
char* probe_arr = (char*) PROBE_REGION_BASE;  // SIZE_GT_LCC

#define AMP_METADATA_START  (AMP_BUFFER_BASE+0x120)
uint8_t* num_calibre = (uint8_t*) AMP_METADATA_START;
uint32_t* max_cycle = (uint32_t*) (AMP_BUFFER_BASE+0x128);
uint8_t* synchronization_flag = (uint8_t*) (AMP_BUFFER_BASE+0x130);
uint8_t* timing_type_tracker = (uint8_t*) (AMP_BUFFER_BASE+0x138);
uint8_t* progress_tracker = (uint8_t*) (AMP_BUFFER_BASE+0x140);
uint8_t* end_tracker = (uint8_t*) (AMP_BUFFER_BASE+0x148);
uint64_t* address_tracker = (uint64_t*) (AMP_BUFFER_BASE+0x150);
#define AMP_METADATA_END    (AMP_BUFFER_BASE+0x158)


// size: [NUM_CALIBRE][MAX_CYCLE]
uint32_t* histogram;
// size: [NUM_CALIBRE][MAX_CYCLE/10]
uint32_t* coarse_histogram;

// size: NUM_CALIBRE
uint32_t* min_time_cycle_arr;
uint32_t* min_time_freq_arr;

// size: NUM_CALIBRE
uint64_t* ave_time_cycle_arr;



/* FUNCTIONS */

static inline void wait(int i)
{
    for (int j = 0; j < i; j++)
    {
        __asm__ __volatile__("nop");
    }
}

void switch_flag(uint8_t flag)
{
    fence_iorwiorw();
    *synchronization_flag = flag;
    publish(synchronization_flag);
    //flush(synchronization_flag);
}

void wait_flag(uint8_t flag)
{
    uint8_t acquired;
    while (1)
    {
        acquired = (*synchronization_flag == flag);
        fence_iorwiorw();
        if (acquired)
            break;

        wait(5);
    }
}

void sync_flag()
{
    //wait_flag(NONE_RUN);

    //if (hartid == LOCAL_HART1)
    //    switch_flag(LOCAL_HART_RUN);
}

void update_progress(uint8_t progress)
{
    if (hartid == LOCAL_HART1)
    {
        *progress_tracker = progress;
        //flush(progress_tracker);
    }
    else
    {
        *(progress_tracker+1) = progress;
        //flush(progress_tracker);
    }
}

void update_timing_type_tracker()
{
    if (hartid == LOCAL_HART1)
    {
        (*timing_type_tracker)++;
        //flush(progress_tracker);
    }
    else
    {
        (*(timing_type_tracker+1))++;
        //flush(progress_tracker);
    }
}

void initialize_amp_pointers()
{
    *num_calibre = NUM_CALIBRE;
    *max_cycle = MAX_CYCLE;
    *timing_type_tracker = 0x00;
    *(timing_type_tracker+1) = 0x00;
    *end_tracker = 0x00;
    publish_range((char*) AMP_METADATA_START, (char*) AMP_METADATA_END);

    // initialize output pointers
    histogram = (uint32_t*) OUTPUT_DATA_BASE;
    coarse_histogram = (uint32_t*) (histogram+HISTOGRAM_SIZE);
    min_time_cycle_arr = (uint32_t*) (coarse_histogram+COARSE_HISTOGRAM_SIZE);
    min_time_freq_arr = (uint32_t*) (min_time_cycle_arr+NUM_CALIBRE);
    ave_time_cycle_arr = (uint64_t*) (min_time_freq_arr+NUM_CALIBRE);

    // initialize output regions
    memset(histogram, 0, HISTOGRAM_SIZE*sizeof(uint32_t));
    memset(coarse_histogram, 0, COARSE_HISTOGRAM_SIZE*sizeof(uint32_t));
    memset(min_time_cycle_arr, 0, NUM_CALIBRE*sizeof(uint32_t));
    memset(min_time_freq_arr, 0, NUM_CALIBRE*sizeof(uint32_t));
    memset(ave_time_cycle_arr, 0, NUM_CALIBRE*sizeof(uint64_t));
}

unsigned long test_delay(char* start[], int type, int measure_type)
{
    unsigned long t = 0;

#ifdef TRACK_SAMPLE_PROGRESS
    update_progress(PROGRESS_TESTS_1);
#endif /* ifdef TRACK_SAMPLE_PROGRESS */

    /* ensure target line is not in cache */
    if (hartid == LOCAL_HART1)
    {
        LOCAL_ENTRY();
        flush_from_cache(start, start_L1, start_LLC);
        LOCAL_EXIT();
    }
    else if (hartid == REMOTE_HART1)
    {
        REMOTE_ENTRY();
        flush_from_cache(start, start_L1, start_LLC);
        REMOTE_EXIT();
    }

    /* see functions for additional `type` related conditions */
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
        if (hartid == LOCAL_HART1)
        {
            LOCAL_ENTRY();
            // initially load start[0] in local L1 for all hybrid cases
            load_L1(start);

            // ensure start[0] is in L2 but not in L1 (local)
            // (all "L2_REMOTE_*" cases)
            evict_local_L1(start, start_L1, type);

            // ensure start[0] is in L3 but not in L1 nor L2 (local)
            // (all "L3_REMOTE_*" cases)
            evict_local_L2(start_L2, type);

            LOCAL_EXIT();
        }
        else if (hartid == REMOTE_HART1)
        {
            REMOTE_ENTRY();

            load_L1(start);

            // ensure start[0] is in L2 but not in L1 (remote)
            // (all "*_REMOTE_L2_*" cases)
            evict_remote_L1(start, start_L1, type);

            // ensure start[0] is in L3 but not in L1 nor L2 (remote)
            // (all "*_REMOTE_L3_*" cases)
            evict_remote_L2(start_L2, type);

            REMOTE_EXIT();
        }
    }

#ifdef TRACK_SAMPLE_PROGRESS
    update_progress(PROGRESS_TESTS_2);
#endif /* ifdef TRACK_SAMPLE_PROGRESS */

    /* prepare for cache hit by reading data or by writing dirty data */
    if (type == REMOTE_L1_HIT_CLEAN || type == REMOTE_L1_HIT_DIRTY
            || type == REMOTE_L2_HIT_CLEAN || type == REMOTE_L2_HIT_DIRTY
            || type == REMOTE_L3_HIT_CLEAN || type == REMOTE_L3_HIT_DIRTY)
    {
        if (hartid == REMOTE_HART1)
        {
            REMOTE_ENTRY();
            // see function for additional `type` related conditions
            exclusive_load_L1(start, type);
            REMOTE_EXIT();
        }
        else {
            LOCAL_ENTRY();
            LOCAL_EXIT();
        }
    }
    else
    {
        if (hartid == LOCAL_HART1)
        {
            LOCAL_ENTRY();
            // see function for additional `type` related conditions
            exclusive_load_L1(start, type);
            LOCAL_EXIT();
        }
        else
        {
            REMOTE_ENTRY();
            REMOTE_EXIT();
        }
    }

#ifdef TRACK_SAMPLE_PROGRESS
    update_progress(PROGRESS_TESTS_3);
#endif /* ifdef TRACK_SAMPLE_PROGRESS */

    /* possible eviction to certain level of cache */
    // ensure start[0] is in L2 but not in L1 (local or remote)
    if (type == L2_HIT_CLEAN || type == L2_HIT_DIRTY
            || type == REMOTE_L2_HIT_CLEAN || type == REMOTE_L2_HIT_DIRTY)
    {
        if (type == REMOTE_L2_HIT_CLEAN || type == REMOTE_L2_HIT_DIRTY)
        {
            if (hartid == REMOTE_HART1)
            {
                REMOTE_ENTRY();
                evict_L1(start, start_L1);
                REMOTE_EXIT();
            }
            else {
                LOCAL_ENTRY();
                LOCAL_EXIT();
            }
        }
        else
        {
            if (hartid == LOCAL_HART1)
            {
                LOCAL_ENTRY();
                evict_L1(start, start_L1);
                LOCAL_EXIT();
            }
            else
            {
                REMOTE_ENTRY();
                REMOTE_EXIT();
            }
        }
    }
    // ensure start[0] is in L3 but not in L1 nor L2 (local or remote)
    else if(type == L3_HIT_CLEAN || type == L3_HIT_DIRTY
                || type == REMOTE_L3_HIT_CLEAN || type == REMOTE_L3_HIT_DIRTY)
    {
        if (type == REMOTE_L3_HIT_CLEAN || type == REMOTE_L3_HIT_DIRTY)
        {
            if (hartid == REMOTE_HART1)
            {
                REMOTE_ENTRY();
                evict_L2(start_L2);
                REMOTE_EXIT();
            }
            else {
                LOCAL_ENTRY();
                LOCAL_EXIT();
            }
        }
        else
        {
            if (hartid == LOCAL_HART1)
            {
                LOCAL_ENTRY();
                evict_L2(start_L2);
                LOCAL_EXIT();
            }
            else
            {
                REMOTE_ENTRY();
                REMOTE_EXIT();
            }
        }
    }
    else if (type == DRAM_HIT)
    {
        if (hartid == LOCAL_HART1)
        {
            LOCAL_ENTRY();
            flush_from_cache(start, start_L1, start_LLC);
            LOCAL_EXIT();
        }
        else
        {
            REMOTE_ENTRY();
            REMOTE_EXIT();
        }
    }

#ifdef TRACK_SAMPLE_PROGRESS
    update_progress(PROGRESS_TESTS_4);
#endif /* ifdef TRACK_SAMPLE_PROGRESS */

    if (hartid == LOCAL_HART1)
    {
        LOCAL_ENTRY();
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

        LOCAL_EXIT();
    }
    else {
        REMOTE_ENTRY();
        REMOTE_EXIT();
    }

#ifdef TRACK_SAMPLE_PROGRESS
    update_progress(PROGRESS_TESTS_5);
#endif /* ifdef TRACK_SAMPLE_PROGRESS */

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
            if (hartid == LOCAL_HART1)
            {
                ave_time_cycle_arr[counter_hist] = 0;
            }
            counter_hist++;
            update_timing_type_tracker();
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
            if (hartid == LOCAL_HART1)
            {
                ave_time_cycle_arr[counter_hist] = 0;
            }
            counter_hist++;
            update_timing_type_tracker();
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

            if (hartid == LOCAL_HART1)
            {
                ave_time_cycle_arr[counter_hist] += t;
                // NOTE: data are stored differently than for SMP Linux-based
                // implementation to facilitate data collection
                // (we use the transpose of the previous matrix)
                if(t<MAX_CYCLE)
                {
                    histogram[t*NUM_CALIBRE + counter_hist]++;
                    coarse_histogram[(t/10)*NUM_CALIBRE + counter_hist]++;
                }
                else
                {
                    histogram[(MAX_CYCLE-1)*NUM_CALIBRE + counter_hist]++;
                    coarse_histogram[
                        (MAX_CYCLE/10-1)*NUM_CALIBRE + counter_hist]++;
                }
            }
        }
        // Update histogram index for each scenario.
        counter_hist++;

        update_timing_type_tracker();
    }
    return counter_hist;
}

void postprocessing()
{
    // histogram
    for (int j = 0; j < NUM_CALIBRE; ++j)
    {
        for(int i=0;i<MAX_CYCLE;i++)
        {
            if (histogram[i*NUM_CALIBRE+j]>min_time_freq_arr[j])
            {
                min_time_freq_arr[j] = histogram[i*NUM_CALIBRE+j];
                min_time_cycle_arr[j] = i;
            }
        }
    }
    for (int j = 0; j < NUM_CALIBRE; ++j)
    {
        ave_time_cycle_arr[j] = ave_time_cycle_arr[j]/TEST_CNT;
    }
}

int amp_main(int argc, char *argv[])
{
    update_progress(PROGRESS_NONE);

    hartid = get_hartid();
    platform_init();


    if (hartid == LOCAL_HART1)
    {
        LOCAL_ENTRY();
        update_progress(PROGRESS_INIT_AMP);
        initialize_amp_pointers();

        update_progress(PROGRESS_INIT_PTRS);

        initialize_probe_array(&probe, probe_arr);
        publish_range(probe_arr, probe_arr+SIZE_GT_LLC*sizeof(char));

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

        publish_range((char*) start, (char*) (start+START_ARR_SIZE));
        publish_range((char*) lines, (char*) (lines+NUM_CANDIDATES));

        publish_range((char*) start_L1, (char*) (start_L1+START_ARR_SIZE));
#ifndef NO_L2_CACHE
        publish_range((char*) start_L2, (char*) (start_L2+START_ARR_SIZE));
#endif /* ifndef NO_L2_CACHE */
#ifndef NO_L3_CACHE
        publish_range((char*) start_L3, (char*) (start_L3+START_ARR_SIZE));
#endif /* ifndef NO_L3_CACHE */

        LOCAL_EXIT();
    }
    else {
        REMOTE_ENTRY();
        fencei();
        REMOTE_EXIT();
    }

    update_progress(PROGRESS_TESTS);

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

    //--- POSTPROCESS RESULTS ---
    // to memory
    update_progress(PROGRESS_POSTPROC);

    if (hartid == LOCAL_HART1)
    {
        postprocessing();

        publish_range((char*) OUTPUT_DATA_BASE,
                      (char*)(ave_time_cycle_arr+NUM_CALIBRE));

        *end_tracker = 1;
    }

    update_progress(PROGRESS_END);

    publish(end_tracker);

    return 0;
}
