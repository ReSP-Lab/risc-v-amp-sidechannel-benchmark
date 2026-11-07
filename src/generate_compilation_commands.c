/*
* This file generates compilation commands to get benchmark binaries
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "vulnerabilities.h"
#include "board.h"

#define RANDOM_NUM 1000

const char* help =
"USAGE: %s\n"
"   -m [machine_name]   should correspond to associated macro"
                            " (case sensitive)\n"
"   -c [max_cycle]      maximum cycles for a memory related operation\n"
"   -n [number_tests]   number of tests per sample\n"
"   -s [samples]        number of samples\n"
"   -g [gcc_toolchain]  name or path of the GCC compiler toolchain to use"
    " (no gcc suffix)\n"
"   -f [flags]          additional compilation flags to provide\n"
"   -a                  use AMP-based setup\n";
//"   -p [probe size]     number of different candidate of \"u\"\n"

const int required_options = 6;
int option;
int option_num = 0;

char* attack_machine = NULL;
int max_cycle = 8000;
int num_test = 1;
//int probe_size = 3;
int samples = 300;
char* gcc_toolchain = "";
char* compl_flags = "";
int amp_setup = 0;

void parse_options(int argc, char const *argv[])
{
    //while (( option = getopt(argc, argv, "m:c:n:p:s:g:")) != -1)
    while (( option = getopt(argc, argv, "m:c:n:s:g:f:ah")) != -1)
    {
        switch (option)
        {
            case 'm':
                attack_machine = optarg;
                option_num++;
                break;
            case 'c':
                max_cycle = atoi(optarg);
                option_num++;
                break;
            case 'n':
                num_test = atoi(optarg);
                option_num++;
                break;
            /*
            case 'p':
                probe_size = atoi(optarg);
                option_num++;
                break;
            */
            case 's':
                samples = atoi(optarg);
                option_num++;
                break;
            case 'g':
                gcc_toolchain = optarg;
                option_num++;
                break;
            case 'f':
                compl_flags = optarg;
                option_num++;
                break;
            case 'a':
                amp_setup = 1;
                break;
            case 'h':
                fprintf(stdout, help, argv[0]);
                exit(EXIT_SUCCESS);
            case '?':
                if (optopt == 'm' || optopt == 'c' || optopt == 'n'
                        || optopt == 's' || optopt == 'r' || optopt == 'g'
                        || optopt == 'f')
                {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n\n",
                            optopt);
                    fprintf(stderr, help, argv[0]);
                }
                else if (isprint (optopt))
                {
                    fprintf (stderr,
                            "Unknown option `-%c'.\n\n",
                            optopt);
                    fprintf(stderr, help, argv[0]);
                }
                else
                {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n\n",
                            optopt);
                    fprintf(stderr, help, argv[0]);
                }

                exit(EXIT_FAILURE);

            default:
                abort();
        }
    }

    if (attack_machine == NULL)
    {
        fprintf(stderr,
                "Missing machine name.\n\n");
        fprintf(stderr, help, argv[0]);
        exit(EXIT_FAILURE);
    }
    /*
    if (option_num != required_options)
    {
        fprintf(stderr,
                "Missing options\n%s'.\n",
                help);
    }
    */
}

int find_index(int aa[], int num_elements, int value)
{
    int i;
    for (i=0; i<num_elements; i++)
    {
        if (aa[i] == value)
        {
            return(value);  /* it was found */
        }
    }
    return(-1);  /* if it was not found */
}

int main(int argc, char const *argv[])
{
    parse_options(argc, argv);

    char* file_name[75];
    //sprintf(file_name, "benchmark_scripts/%s_NumSamples%d_NumTest%d_ProbeSize%d_MaxCycle%d", attack_machine, samples, num_test, probe_size, max_cycle);

#ifndef I_CACHE_ANALYSIS
    sprintf(file_name,
            "benchmark_scripts/%s_NumSamples%d_NumTest%d_MaxCycle%d",
            attack_machine, samples, num_test, max_cycle);
#else
    sprintf(file_name,
            "benchmark_scripts/%s_NumSamples%d_NumTest%d_MaxCycle%d"
            "_l1icache",
            attack_machine, samples, num_test, max_cycle);
#endif /* ifndef I_CACHE_ANALYSIS */

    FILE *fout = fopen(strcat(file_name,".sh"), "w");
    if(fout == NULL) {
        printf("\nCould not open file fout\n");
        return -1;
    }

    char command_basis[350];
    char object_folder[125];
    char binary_folder[100];
    char binary_name[75];
    char binary_path[450];
    char plain[10];

    sprintf(command_basis,
            "%sgcc -Iinclude/ %s -DBOARD=%s -DON_MULTIPLE_LINES=%d -O0 ",
            gcc_toolchain, compl_flags, attack_machine, ON_MULTIPLE_LINES
    );

#ifdef I_CACHE_ANALYSIS
    const char* cache_suffix = "_l1icache";
#else
    const char* cache_suffix = "";
#endif

    sprintf(
            binary_folder,
            "benchmark_output/%s%s%s",
            attack_machine, amp_setup ? "_amp" : "_smp", cache_suffix
    );

    sprintf(object_folder, "%s/objects", binary_folder);

    fprintf(fout, "#!/bin/sh\n");
    fprintf(fout, "set -xe\n");
    fprintf(fout, "mkdir -p %s\n",
            object_folder);
    fprintf(fout, "echo \"Starting compilation...\"\n");
    if (!amp_setup)
    {
        fprintf(fout, "%s -c src/rng.c -o %s/rng.o\n",
                command_basis, object_folder);
        fprintf(fout, "%s -c src/evict.c -o %s/evict.o\n",
                command_basis, object_folder);
        fprintf(fout, "%s -c src/stats.c -o %s/stats.o\n",
                command_basis, object_folder);
    }

    srand(1234);
    int grids[4913]= {0};
    int eff_grids[88]= {0};
    // 1 internal 2 with attacker but no two process on the same core
    // 3 time-slicing 4 hyper-threading

    int pattern_counter = 0;
    for (int vul_num_order = 0; vul_num_order < VUL_NUM; ++vul_num_order)
    {
        int eff_flag = false;
        pattern_counter = 0;
        for (int vul1 = 0; vul1 < TOTAL_STATES; ++vul1)
        {
            for (int vul2 = 0; vul2 < TOTAL_STATES; ++vul2)
            {
                for (int vul3 = 0; vul3 < TOTAL_STATES; ++vul3)
                {
                    /*
                     * each vul. is used to select one access from the 17
                     * possible ones, and select active entity and
                     * address+access type
                     */
                    total_arr[pattern_counter][0] = pattern_counter;
                    total_arr[pattern_counter][1] = possi_access[vul1][0];
                    total_arr[pattern_counter][2] = possi_access[vul1][1];
                    total_arr[pattern_counter][3] = possi_access[vul2][0];
                    total_arr[pattern_counter][4] = possi_access[vul2][1];
                    total_arr[pattern_counter][5] = possi_access[vul3][0];
                    total_arr[pattern_counter][6] = possi_access[vul3][1];
                    total_arr[pattern_counter][7] = 0;

                    if (eff_flag==false && total_arr[pattern_counter][1]==three_step_arr[vul_num_order][1] && total_arr[pattern_counter][2]==three_step_arr[vul_num_order][2] && total_arr[pattern_counter][3]==three_step_arr[vul_num_order][3] && total_arr[pattern_counter][4]==three_step_arr[vul_num_order][4] && total_arr[pattern_counter][5]==three_step_arr[vul_num_order][5] && total_arr[pattern_counter][6]==three_step_arr[vul_num_order][6])
                    {
                        printf("%d\t\t%d\t\t%s_%s\t\t%s_%s\t\t%s_%s\n", pattern_counter, total_arr[pattern_counter][0], type_match[total_arr[pattern_counter][1]], type_match[total_arr[pattern_counter][2]], type_match[total_arr[pattern_counter][3]], type_match[total_arr[pattern_counter][4]], type_match[total_arr[pattern_counter][5]], type_match[total_arr[pattern_counter][6]]);
                        eff_flag = true;
                    }
                    else {
                        pattern_counter = pattern_counter + 1;
                        continue;
                    }

                    // TODO: check impact if not DIFF_CORE_ONLY
                    int skip_case = false;

                    // assign steps
                    for (int if_plain = 0; if_plain < 2; ++if_plain)
                    {
                    for (int if_write_1 = 0; if_write_1 < 2; ++if_write_1)
                    {
                    for (int if_write_2 = 0; if_write_2 < 2; ++if_write_2)
                    {
                    for (int if_write_3 = 0; if_write_3 < 2; ++if_write_3)
                    {
                    for (int affinity_rel = 0; affinity_rel < 2; ++affinity_rel)
                    {
                        skip_case = false;

#ifdef I_CACHE_ANALYSIS
                        if (if_write_1 || if_write_2 || if_write_3)
                            skip_case = true;
#endif /* ifdef I_CACHE_ANALYSIS */

                        // assign steps
                        int attacker_l_list[3] = {0};
                        int victim_l_list[3] = {0};
                        int attacker_r_list[3] = {0};
                        int victim_r_list[3] = {0};

                        // local list
                        int L_list[10] = {
                            A, A_ALIAS, D, U, STAR,
                            INV, A_INV, D_INV, U_INV, A_ALIAS_INV
                        };
                        // remote list
                        int R_list[5] = {
                            INV, A_INV, D_INV, U_INV, A_ALIAS_INV
                        };
                        int lr_num[4]= {0};
                        int star_choice = RANDOM_NUM;

                        for (int list_num = 0; list_num < 3; ++list_num)
                        {
                            // inv and flush
                            int inner_type=0;
                            // - define access based on `list_num`
                            // - use `inner_type` to determine if access
                            //   based on write operation or not
                            if (list_num==0) inner_type=if_write_1;
                            else if (list_num==1) inner_type=if_write_2;
                            else if (list_num==2) inner_type=if_write_3;

                            // - `2*list_num+1` is for active entity
                            //   (attacker or victim)
                            // - `2*list_num+2` is for location + access type

                            /*
                             * if local invalidation without write (i.e. flush
                             * operation):
                             *     distinction with write operation because
                             *     flush on local core only affect 5 elements
                             *     from L_list,
                             *     while write is OK for all of them
                             */
                            if ((total_arr[pattern_counter][2*list_num+2]==INV||total_arr[pattern_counter][2*list_num+2]==A_INV
                                    ||total_arr[pattern_counter][2*list_num+2]==D_INV||total_arr[pattern_counter][2*list_num+2]==U_INV
                                    ||total_arr[pattern_counter][2*list_num+2]==A_ALIAS_INV)&&inner_type==0/*flush*/)
                            {
#ifdef NO_FLUSH
                                skip_case = true;
#endif /* ifdef NO_FLUSH */

                                // if location+access is in local list
                                if (find_index(L_list, 10, total_arr[pattern_counter][2*list_num+2])!=-1)
                                {
                                    if (total_arr[pattern_counter][2*list_num+1]==ATTACKER)
                                    {
                                        lr_num[0]++;
                                        attacker_l_list[list_num]=list_num+1;
                                    }
                                    else if (total_arr[pattern_counter][2*list_num+1]==VICTIM)
                                    {
                                        lr_num[1]++;
                                        victim_l_list[list_num]=list_num+1;
                                    }
                                }
                            }
                            /* if remote invalidation with write operation */
                            /* commented out because of redundancy
                            else if ((total_arr[pattern_counter][2*list_num+2]==INV||total_arr[pattern_counter][2*list_num+2]==A_INV
                                ||total_arr[pattern_counter][2*list_num+2]==D_INV||total_arr[pattern_counter][2*list_num+2]==U_INV
                                ||total_arr[pattern_counter][2*list_num+2]==A_ALIAS_INV)&&inner_type==1)
                            {
                                if (find_index(R_list, 5, total_arr[pattern_counter][2*list_num+2])!=-1)
                                {
                                    if (total_arr[pattern_counter][2*list_num+1]==ATTACKER)
                                    {
                                        lr_num[2]++;
                                        attacker_r_list[list_num]=list_num+1;
                                    }
                                    else if (total_arr[pattern_counter][2*list_num+1]==VICTIM)
                                    {
                                        lr_num[3]++;
                                        victim_r_list[list_num]=list_num+1;
                                    }
                                }
                            }
                            */
                            /* if remote invalidation */
                            else if (find_index(R_list, 5, total_arr[pattern_counter][2*list_num+2])!=-1)
                            {
                                if (total_arr[pattern_counter][2*list_num+1]==ATTACKER)
                                {
                                    lr_num[2]++;
                                    attacker_r_list[list_num]=list_num+1;
                                }
                                else if (total_arr[pattern_counter][2*list_num+1]==VICTIM)
                                {
                                    lr_num[3]++;
                                    victim_r_list[list_num]=list_num+1;
                                }
                            }
                            /* if local access excluding invalidation
                             * (i.e., only read or write operation),
                             * no distinction is required at this step */
                            else if (find_index(L_list, 5, total_arr[pattern_counter][2*list_num+2])!=-1)
                            {
                                if (total_arr[pattern_counter][2*list_num+2]==STAR)
                                {
                                    star_choice = rand()%2;
                                    if (star_choice==ATTACKER)
                                    {
                                        lr_num[0]++;
                                        attacker_l_list[list_num]=list_num+1;
                                    }
                                    else if (star_choice==VICTIM)
                                    {
                                        lr_num[1]++;
                                        victim_l_list[list_num]=list_num+1;
                                    }
                                    else
                                    {
                                        printf("ERROR: no star matched");
                                        exit(1);
                                    }
                                }
                                else if (total_arr[pattern_counter][2*list_num+1]==ATTACKER)
                                {
                                    lr_num[0]++;
                                    attacker_l_list[list_num]=list_num+1;
                                }
                                else if (total_arr[pattern_counter][2*list_num+1]==VICTIM)
                                {
                                    lr_num[1]++;
                                    victim_l_list[list_num]=list_num+1;
                                }
                            }
                            else
                            {
                                printf("ERROR: variable cannot be matched");
                                exit(1);
                            }
                        }

                        bool shares_core =
                            (lr_num[0] > 0 && lr_num[1] > 0) ||
                            (lr_num[2] > 0 && lr_num[3] > 0);

                        // filter when SMT is used but core is not shared
                        if (affinity_rel == 1 && !shares_core)
                            skip_case = true;

                        char* flags[200];
                        memset(flags, 0, sizeof(flags));
                        char* buffer[50];
                        for (int list_num = 0; list_num < 3; ++list_num)
                        {
                            if (attacker_l_list[list_num])
                            {
                                sprintf(buffer, "-DATTACKER_L_%d ", list_num+1);
                                strcat(flags, buffer);
                            }
                            else if (victim_l_list[list_num])
                            {
                                sprintf(buffer, "-DVICTIM_L_%d ", list_num+1);
                                strcat(flags, buffer);
                            }
                            else if (attacker_r_list[list_num])
                            {
                                sprintf(buffer, "-DATTACKER_R_%d ", list_num+1);
                                strcat(flags, buffer);
                            }
                            else if (victim_r_list[list_num])
                            {
                                sprintf(buffer, "-DVICTIM_R_%d ", list_num+1);
                                strcat(flags, buffer);
                            }

                            sprintf(buffer, "-DENTITY%d=%d ",
                                    list_num+1,
                                    total_arr[pattern_counter][2*list_num+1]
                            );
                            strcat(flags, buffer);
                            sprintf(buffer, "-DVUL%d=%d ",
                                    list_num+1,
                                    total_arr[pattern_counter][2*list_num+2]
                            );
                            strcat(flags, buffer);
                        }
                        sprintf(buffer,
                                "-DIF_WRITE_1=%d "
                                "-DIF_WRITE_2=%d "
                                "-DIF_WRITE_3=%d ",
                                if_write_1, if_write_2, if_write_3);
                        strcat(flags, buffer);
                        sprintf(buffer, "-DAFFINITY_REL=%d ", affinity_rel);
                        strcat(flags, buffer);
                        sprintf(buffer, "-DIF_PLAIN=%d ", if_plain);
                        strcat(flags, buffer);
                        sprintf(buffer, "-DSKIP_CASE=%d ", skip_case);
                        strcat(flags, buffer);
                        sprintf(buffer, "-DVUL_N=%d ", vul_num_order+1);
                        strcat(flags, buffer);
                        sprintf(buffer, "-DPATTERN=%d ", pattern_counter);
                        strcat(flags, buffer);

                        sprintf(buffer, "-DNUM_SAMPLES=%d ", samples);
                        strcat(flags, buffer);
                        sprintf(buffer, "-DNUM_TEST=%d ", num_test);
                        strcat(flags, buffer);
                        sprintf(buffer, "-DMAX_CYCLE=%d ", max_cycle);
                        strcat(flags, buffer);
#ifdef I_CACHE_ANALYSIS
                        sprintf(buffer, "-DI_CACHE_ANALYSIS ");
                        strcat(flags, buffer);
#endif /* ifdef I_CACHE_ANALYSIS */

                        if (if_plain)
                        {
                            sprintf(plain, "_plain");
                        }
                        else
                        {
                            sprintf(plain, "");
                        }

#ifndef I_CACHE_ANALYSIS
                        sprintf(binary_name,
                                "%02d_AccOne%d_AccTwo%d_AccThr%d_Aff%d%s"
                                "_%04d",
                                vul_num_order,
                                if_write_1, if_write_2, if_write_3,
                                affinity_rel, plain, pattern_counter
                        );
#else
                        sprintf(binary_name,
                                "%02d_AccOne%d_AccTwo%d_AccThr%d_Aff%d%s"
                                "_%04d_l1icache",
                                vul_num_order,
                                if_write_1, if_write_2, if_write_3,
                                affinity_rel, plain, pattern_counter
                        );
#endif /* ifndef I_CACHE_ANALYSIS */

                        if (!amp_setup)
                        {
                            sprintf(binary_path, "%s/%s",
                                    binary_folder, binary_name);
                            fprintf(fout, "%s %s -c src/%s.c -o %s/%s.o\n",
                                command_basis, flags, "benchmark_utils",
                                object_folder, "benchmark_utils");
                            fprintf(fout, "%s %s -c "
                                          "src/linux_setup/%s.c "
                                          "-o %s/%s.o\n",
                                command_basis, flags, "benchmark_implem",
                                object_folder, "benchmark_implem");
                            fprintf(fout, "%s %s -c src/%s.c -o %s/%s.o\n",
                                command_basis, flags, "benchmark",
                                object_folder, "benchmark");
                            fprintf(fout, "%s %s/*.o -o %s -lm\n",
                                command_basis, object_folder, binary_path);
                        }
                        else
                        {
                            sprintf(binary_path, "%s/%s",
                                    binary_folder, binary_name);
                            fprintf(fout,
                                    "%s %s "
                                    "-nostdlib -T amp-setup/linker_payload.ld "
                                    "src/rng.c "
                                    "src/evict.c "
                                    "src/benchmark_utils.c "
                                    "src/amp_setup/stdlib_substitute.c "
                                    "src/amp_setup/benchmark_implem.c "
                                    "src/benchmark.c "
                                    "-o %s/payload.elf\n",
                                    command_basis, flags,
                                    object_folder);
                            fprintf(fout,
                                    "%sobjcopy -O binary "
                                    "%s/payload.elf %s\n",
                                    gcc_toolchain, object_folder, binary_path);
                            fprintf(fout,
                                    "%sobjdump -S "
                                    "%s/payload.elf > %s/%s.s\n",
                                    gcc_toolchain, object_folder,
                                    object_folder, binary_name);
                        }
                    }
                    }
                    }
                    }
                    }
                    pattern_counter = pattern_counter + 1;
                }
            }
        }
    }

    fprintf(fout, "echo \"Compilation done.\"\n");
    fclose(fout);

    return 0;
}
