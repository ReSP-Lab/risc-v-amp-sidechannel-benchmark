#
# This file plots and prints results for the benchmark
#
# Copyright (C) 2024-2025 Cédrick Austa <cedrick.austa@ulb.be>
# Copyright (C) 2020 Shuwen Deng <shuwen.deng@yale.edu>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
#
#


import csv
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import (AutoMinorLocator, MultipleLocator, FixedLocator)
import numpy as np
import seaborn as sns
import sys

case_array = [
        3,4,3,4,3,4,3,4,2,0,2,0,2,0,2,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,3,4,3,4,2,0,2,0,2,0,2,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,2,0,3,4,3,4,2,0,2,0,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,2,0,3,4,3,4,2,0,2,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,2,0,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,2,0,2,0,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,2,0,2,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,2,0,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,2,0,2,0,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,2,0,2,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,2,0,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,2,0,2,0,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,2,0,2,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,2,0,3,4,3,4,3,4,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,2,0,3,4,3,4,3,4,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,2,0,3,4,2,0,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,3,4,3,4,2,0,3,4,2,0,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,3,4,3,4,3,4,2,0,3,4,2,0,
        3,4,2,0,3,4,2,0,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,2,0,3,4,2,0,
        3,4,2,0,3,4,2,0,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,2,0,3,4,2,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,3,4,3,4,3,4,2,0,3,4,
        3,4,3,4,3,4,2,0,3,4,3,4,3,4,2,0,
        3,4,2,0,3,4,3,4,3,4,2,0,3,4,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,3,4,3,4,3,4,2,0,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,3,4,3,4,2,0,3,4,3,4,
        3,4,3,4,3,4,2,0,2,0,3,4,3,4,3,4,
        3,4,2,0,3,4,3,4,3,4,3,4,2,0,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,3,4,3,4,2,0,3,4,3,4,
        3,4,3,4,3,4,2,0,2,0,3,4,3,4,3,4,
        3,4,2,0,3,4,3,4,3,4,3,4,2,0,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,3,4,3,4,2,0,3,4,3,4,
        3,4,3,4,3,4,2,0,2,0,3,4,3,4,3,4,
        3,4,2,0,3,4,3,4,3,4,3,4,2,0,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,3,4,3,4,2,0,3,4,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,3,4,2,0,3,4,3,4,2,0,3,4,3,4,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        3,4,2,0,3,4,2,0,3,4,2,0,3,4,2,0,
        3,4,2,0,3,4,2,0,3,4,2,0,3,4,2,0,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4
]
ONLY_VICTIM=1
HAS_ATT=2
TIME_SLICING=3
HYPERTHREADING=4
NONE=0
case_mapping = {
        NONE: "NONE",
        ONLY_VICTIM: "ONLY_VICTIM",
        HAS_ATT: "HAS_ATT",
        TIME_SLICING: "TIME_SLICING",
        HYPERTHREADING: "HYPERTHREADING"
}
READ_ACC=1
WRITE_ACC=2
FLUSH_INV=3
WRITE_INV=4
operation_mapping = {
        READ_ACC: "READ_ACCESS",
        WRITE_ACC: "WRITE_ACCESS",
        FLUSH_INV: "FLUSH_INVALIDATION",
        WRITE_INV: "WRITE_INVALIDATION"
}

VUL_NUM = 88
#TIMING_NUM = 16
CONFIG_NUM = 16
TOTAL_CASES = 88*16
TARGET_NUM = 3

#CPU_LIST = ["C910", "U54", "U74"]
CPU_LIST = ["C910", "C910 AMP", "U74", "U74 AMP"]
#CPU_LIST = ["C910", "U74", "U74 AMP"]
CPU_LABEL = "CPUs"
#CPU_LABEL = "CPU type"
#TIMING_LABEL= "Timing type"
CONFIG_LABEL= "Test config."
OR_CPU_NAME = "In at least one CPU"
#AND_CPU_NAME = "In all CPUs"
AND_CPU_NAME = "Present in\nall CPUs"
NONE_CPU_NAME = "Absent from\nall CPUs"

params = {
    'legend.title_fontsize': 13,
    'legend.fontsize': 13,
    'legend.handlelength': 2,
    #'legend.frameon': False,
    'axes.titlesize'      : 18,
    'axes.labelsize'      : 16,
    'xtick.labelsize'      : 13,
    'ytick.labelsize'      : 13
}
plt.rcParams.update(params)


IA_BASE=[1,2,3,4,10,12,14,16,18,20,45,46,47,48,54,56,58,60,62,64]
IS_BASE=[26,28,32,34,35,39,70,72,76,78,79,83]
ISA_BASE=[22,24,30,33,36,40,66,68,74,77,80,84]
EA_BASE=[5,6,7,8,9,11,13,15,17,19,49,50,51,52,53,55,57,59,61,63,]
ES_BASE=[25,27,31,37,41,43,69,71,75,81,85,87]
ESA_BASE=[21,23,29,38,42,44,65,67,73,82,86,88]


def build_dict_entry(file_name, vul_num):
    entry = []
    inner_counter = 0
    counter = 0
    with open(file_name,'r') as csv_file:
        plots = csv.reader(csv_file, delimiter=' ')
        for row in plots:
            if row!=[]:
                if row[0]=='222':
                    entry.append(-2)
                elif row[0]=='111':
                    entry.append(-1)
                #elif int(row[0], 2)==2:
                #    entry.append(1)
                else:
                    entry.append(int(row[0], 2))
                    #entry.append(0)
                inner_counter += 1
            else:
                inner_counter = 0
                counter += 1
    return entry

def add_target_case(y, target_name, target_file_name):
    y[CPU_LABEL] = [*y[CPU_LABEL],
                     *[cpu_name for _ in range(TOTAL_CASES)]]
    y["Result"] = [*y["Result"],
                   *build_dict_entry(cpu_file, VUL_NUM)]


# main execution
if __name__ == "__main__":
    cpu = sys.argv[1]
    input_file_names = sys.argv[2:-1]
    output_file_name = sys.argv[-1]

    t = [i+1 for _ in range(VUL_NUM) for i in range(CONFIG_NUM)]
    v = [i+1 for i in range(VUL_NUM) for _ in range(CONFIG_NUM)]

    y = {}
    y[CONFIG_LABEL] = []
    y["Vulnerability"] = []
    y[CPU_LABEL] = []
    y["Result"] = []


    input_file_names.sort()
    print(input_file_names)
    CPU_LIST = []
    for i in range(len(input_file_names)):
        cpu_name = f"{cpu}_{i}"
        CPU_LIST = [*CPU_LIST, cpu_name]
        cpu_file = input_file_names[i]
        add_target_case(y, cpu_name, cpu_file)
        y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
        y["Vulnerability"] = [*y["Vulnerability"], *v]

    TARGET_NUM = len(input_file_names)


    y_or = []
    y_and = []
    y_none = []

    original_table = pd.DataFrame(y)

    table = original_table[original_table["Result"] >= 0].copy()
    redundant_table = original_table[
            original_table["Result"] == -1
    ]
    unsupported_table = original_table[
            original_table["Result"] == -2
    ]

    config_mapping = {
            1: "RF_RF_RF_TS", 2: "RF_RF_RF_SMT", 3: "RF_RF_W_TS",
            4: "RF_RF_W_SMT", 5: "RF_W_RF_TS", 6: "RF_W_RF_SMT",
            7: "RF_W_W_TS", 8: "RF_W_W_SMT",
            9: "W_RF_RF_TS", 10: "W_RF_RF_SMT", 11: "W_RF_W_TS",
            12: "W_RF_W_SMT", 13: "W_W_RF_TS", 14: "W_W_RF_SMT",
            15: "W_W_W_TS", 16: "W_W_W_SMT"
    }
    table[CONFIG_LABEL] = table[CONFIG_LABEL].map(config_mapping)

    filtering_cols = ["Vulnerability", "Test config."]
    # keep groups with more than one possible value in "Result"
    conflict_table = table.groupby(filtering_cols, as_index=False).filter(
                  lambda x: x["Result"].nunique() > 1)
    conflict_table["Result Freq."] = conflict_table.groupby(
            filtering_cols, as_index=False)["Result"].transform(
                    lambda x: x.map(x.value_counts()))
    conflict_table2 = conflict_table[
            conflict_table["Result Freq."] < (TARGET_NUM-1)]
    instable_table2 = conflict_table2.drop_duplicates(
            subset=[*filtering_cols, "Result"], keep="last")
    instable_table = table.drop_duplicates(subset=[*filtering_cols, "Result"],
                                           keep=False)
    instable_table = instable_table.sort_values(by=filtering_cols)
    print(f"Unique results: dimensions = {instable_table.shape}")
    print(instable_table)

    palette = sns.color_palette(
            "blend:#000000,#6a5acd,#df73ff,#cec8ef,#c6dbef,#a6e0ff,#3182bd,#004953,#00a86b,#98fb98,#e8f48c,#fff44f,#fdd0a2,#ffa700,#ff0000,#c41e3a",
            as_cmap=True
    )
    #print(palette.as_hex())
    fig, ax = plt.subplots(figsize=(14, 6))
    # get discrete equivalent of the CMAP as a palette
    ncolors = len(config_mapping.values())
    custom_palette = [ palette(i/(ncolors-1))
                for i in range(0, ncolors) ]

    # When SMT is not enabled, useless to consider it
    sub_table = instable_table[instable_table[CONFIG_LABEL].str.endswith("_TS")]
    sub_table[CONFIG_LABEL] = sub_table[CONFIG_LABEL].str.replace(
            "_TS", "", regex=False)
    sub_config_mapping = {
            1: "RF_RF_RF", 3: "RF_RF_W",
            5: "RF_W_RF", 7: "RF_W_W",
            9: "W_RF_RF", 11: "W_RF_W",
            13: "W_W_RF", 15: "W_W_W",
    }
    sns.swarmplot(
            x="Vulnerability", y=CPU_LABEL,
            data=sub_table,
            hue=CONFIG_LABEL,
            #hue_order=config_mapping.values(),
            hue_order=sub_config_mapping.values(),
            dodge=True,
            order = [
                *CPU_LIST,
                #AND_CPU_NAME,
                #NONE_CPU_NAME,
            ],
            orient='h', ax=ax,
            palette=custom_palette[::2],
    )

    # Optional separation on 3rd step: access (left) or invalidation (right)
    #plt.axvline(x=44.5, color='black', lw=0.75, alpha=0.5)
    x_label = "Vulnerability"
    y_label = CPU_LABEL
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    fixed_loc = np.array([1, *np.arange(5,87,5), 88])
    ax.xaxis.set_major_locator(FixedLocator(fixed_loc))
    minors = np.setdiff1d(np.arange(0, 88), fixed_loc)
    ax.xaxis.set_minor_locator(FixedLocator(minors))
    ax.set_xlim(0, 89)
    #ax.xaxis.set_major_locator(MultipleLocator(5))
    #ax.xaxis.set_minor_locator(AutoMinorLocator(5))
    ax.grid(which='major', color='#CCCCCC', linestyle='--')
    ax.grid(which='minor', color='#CCCCCC', linestyle=':')
    sns.move_legend(
            ax, 'best', bbox_to_anchor=(1, 1), frameon=False,
            title=CONFIG_LABEL
    )
    plt.tight_layout(pad=1)
    fig.savefig(f"{output_file_name}.eps", dpi=1000)#, format="png")
    plt.close(fig)

    print("All instable results, without redundancy on per-variant results: "+
          f"dimensions = {instable_table2.shape}")
    instable_table2 = instable_table2.sort_values(by=filtering_cols)
    instable_table2.to_csv(f"{output_file_name}.csv")
    instable_table2 = instable_table2.drop_duplicates(
            subset=filtering_cols, keep="last")
    print("All instable results, without redundancy on variant: "+
          f"dimensions = {instable_table2.shape}")
    instable_table2 = instable_table2[filtering_cols]
    instable_table2.to_csv(f"{output_file_name}-variant-vuln.csv")
    instable_table2 = instable_table2.drop_duplicates(
            subset=["Vulnerability"], keep="last")
    instable_table2 = instable_table2["Vulnerability"]
    print("All instable vulnerabilities, without redundancy: "+
          f"dimensions = {instable_table2.shape}")
    instable_table2.to_csv(f"{output_file_name}-vuln.csv")


