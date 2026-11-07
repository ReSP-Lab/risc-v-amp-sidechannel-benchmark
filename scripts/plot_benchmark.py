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
import argparse

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

#CPU_LIST = ["K1", "TH1520", "FU740", "MPFS025T"]
CPU_LIST = [
        "K1 - SMP",
        "K1 - AMP",
        #"K1 - SMP RC",
        "K1 - AMP RC",
        "TH1520 - SMP",
        "TH1520 - AMP",
        "FU740 - SMP",
        "FU740 - AMP",
        #"MPFS025T"
]
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
                elif int(row[0], 2)==2:
                    entry.append(1)
                else:
                    entry.append(0)
                inner_counter += 1
            else:
                inner_counter = 0
                counter += 1
    return entry

def add_target_case(y, target_name, target_file_name):
    results = build_dict_entry(target_file_name, VUL_NUM)
    if len(results) != TOTAL_CASES:
        raise ValueError(
            f"{target_file_name}: expected {TOTAL_CASES} cases, "
            f"got {len(results)}"
        )
    y[CPU_LABEL].extend([target_name] * TOTAL_CASES)
    y["Result"].extend(results)


# main execution
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Plot vulnerability types summaries."
    )
    parser.add_argument("output_file", help="Output filename without .eps")
    parser.add_argument("--i-cache-analysis", action="store_true",
                        help="I cache analysis")

    args = parser.parse_args()
    if args.i_cache_analysis:
        sfx = "-icache"
        smp_sfx = "_icache"
        #dodge = False
        dodge = True
    else:
        sfx = ""
        smp_sfx = ""
        dodge = True

    t = [i+1 for _ in range(VUL_NUM) for i in range(CONFIG_NUM)]
    v = [i+1 for i in range(VUL_NUM) for _ in range(CONFIG_NUM)]

    y = {}
    y[CONFIG_LABEL] = []
    y["Vulnerability"] = []
    y[CPU_LABEL] = []
    y["Result"] = []

    cpu_name = "FU740 - AMP"
    cpu_file = f"results/benchmark-outputs/unmatched_amp_s5000-n1{sfx}/unmatched0.res"
    add_target_case(y, cpu_name, cpu_file)
    y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
    y["Vulnerability"] = [*y["Vulnerability"], *v]

    cpu_name = "FU740 - SMP"
    cpu_file = "results/benchmark-outputs/unmatched_smp_s5000-n1_results/" \
            + f"unmatched_smp_s5000-n1{smp_sfx}.res"
    add_target_case(y, cpu_name, cpu_file)
    y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
    y["Vulnerability"] = [*y["Vulnerability"], *v]

    cpu_name = "K1 - AMP"
    cpu_file = f"results/benchmark-outputs/bpi_f3_amp_s5000-n1{sfx}/bpi_f31.res"
    #cpu_file = f"results/benchmark-outputs/tmp-res/bpi_f3-s5000-n1{sfx}/0.res"
    add_target_case(y, cpu_name, cpu_file)
    y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
    y["Vulnerability"] = [*y["Vulnerability"], *v]

    cpu_name = "K1 - AMP RC"
    cpu_file = "results/benchmark-outputs/" \
               + f"bpi_f3_remote-cluster_s5000-n1{sfx}/bpi_f30.res"
    add_target_case(y, cpu_name, cpu_file)
    y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
    y["Vulnerability"] = [*y["Vulnerability"], *v]

    cpu_name = "K1 - SMP"
    cpu_file = "results/benchmark-outputs/bpi_f3_smp_s5000-n1_results/" \
               + f"bpi_f3_smp_s5000-n1{smp_sfx}.res"
    add_target_case(y, cpu_name, cpu_file)
    y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
    y["Vulnerability"] = [*y["Vulnerability"], *v]

    cpu_name = "TH1520 - AMP"
    cpu_file = f"results/benchmark-outputs/bv_ahead_amp_s5000-n1{sfx}/bv_ahead0.res"
    add_target_case(y, cpu_name, cpu_file)
    y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
    y["Vulnerability"] = [*y["Vulnerability"], *v]

    cpu_name = "TH1520 - SMP"
    cpu_file = f"results/benchmark-outputs/bv_ahead_smp_s5000-n1_results/" \
               +f"bv_ahead_smp_s5000-n1{smp_sfx}.res"
    add_target_case(y, cpu_name, cpu_file)
    y[CONFIG_LABEL] = [*y[CONFIG_LABEL], *t]
    y["Vulnerability"] = [*y["Vulnerability"], *v]

    original_table = pd.DataFrame(y)

    keys = [CPU_LABEL, "Vulnerability", CONFIG_LABEL]
    expected = pd.MultiIndex.from_product(
        [CPU_LIST, range(1, VUL_NUM + 1), range(1, CONFIG_NUM + 1)],
        names=keys,
    )
    actual = pd.MultiIndex.from_frame(original_table[keys])

    print(expected.difference(actual))
    print(actual.difference(expected))

    if (
        actual.has_duplicates
        or len(expected.difference(actual))
        or len(actual.difference(expected))
    ):
        raise ValueError("Missing, duplicated, or unexpected benchmark cases")

    if not original_table["Result"].isin([-2, -1, 0, 1]).all():
        raise ValueError("Unexpected result code")


    table = original_table[original_table["Result"] > 0].copy()
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

    palette = sns.color_palette(
            "blend:#000000,#6a5acd,#df73ff,#cec8ef,#c6dbef,#a6e0ff,#3182bd,#004953,#00a86b,#98fb98,#e8f48c,#fff44f,#fdd0a2,#ffa700,#ff0000,#c41e3a",
            as_cmap=True
    )
    #print(palette.as_hex())
    fig, ax = plt.subplots(figsize=(14, 7))
    # get discrete equivalent of the CMAP as a palette
    ncolors = len(config_mapping.values())
    custom_palette = [ palette(i/(ncolors-1))
                for i in range(0, ncolors) ]

    # When SMT is not enabled, useless to consider it
    sub_table = table[table[CONFIG_LABEL].str.endswith("_TS")]
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
            #data=table,
            data=sub_table,
            hue=CONFIG_LABEL,
            #hue_order=config_mapping.values(),
            hue_order=sub_config_mapping.values(),
            dodge=dodge,
            order = [
                *CPU_LIST,
                #AND_CPU_NAME,
                #NONE_CPU_NAME,
            ],
            orient='h', ax=ax,
            palette=custom_palette[::2],
    )

    # Select the same time-slicing configurations as the main plot
    unsupported_sub_table = unsupported_table.loc[
        unsupported_table[CONFIG_LABEL].isin(sub_config_mapping)
    ].copy()
    unsupported_sub_table[CONFIG_LABEL] = (
        unsupported_sub_table[CONFIG_LABEL].map(sub_config_mapping)
    )

    if not unsupported_sub_table.empty:
        sns.swarmplot(
            data=unsupported_sub_table,
            x="Vulnerability",
            y=CPU_LABEL,
            hue=CONFIG_LABEL,
            hue_order=list(sub_config_mapping.values()),
            order=CPU_LIST,
            palette=["#A0B0B0"],
            dodge=dodge,
            orient="h",
            marker="x",
            linewidth=0.5,
            size=5,
            legend=False,
            ax=ax,
        )

        # Add a legend entry explaining the cross marker.
        ax.plot(
            [], [], marker="x", linestyle="none",
            color="#A0B0B0", label="Unsupported",
        )
        ax.legend(title=CONFIG_LABEL)


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
    for i in range(len(CPU_LIST) - 1):
        ax.axhline(
            y=i + 0.5,
            color='#CCCCCC',
            linewidth=0.7,
            linestyle="--",
            zorder=0,
    )

    plt.tight_layout(pad=1)
    fig.savefig(f"figures/{args.output_file}.eps", dpi=1000)#, format="png")
    plt.close(fig)


    # - Result Analysis -
    # Detection and availability are separate properties.
    evaluated = original_table.assign(
        Detected=original_table["Result"].eq(1),
        Available=original_table["Result"].ge(0),
    )

    per_cpu = evaluated.groupby(
        [CPU_LABEL, "Vulnerability"], as_index=False
    ).agg(
        Result=("Detected", "any"),
        Available=("Available", "any"),
    )

    across_cpus = per_cpu.groupby(
        "Vulnerability", as_index=False
    ).agg(
        AllDetected=("Result", "all"),
        AnyDetected=("Result", "any"),
        AllAvailable=("Available", "all"),
    )

    all_rows = across_cpus[["Vulnerability"]].copy()
    all_rows[CPU_LABEL] = AND_CPU_NAME
    all_rows["Result"] = across_cpus["AllDetected"]

    none_rows = across_cpus[["Vulnerability"]].copy()
    none_rows[CPU_LABEL] = NONE_CPU_NAME
    none_rows["Result"] = (
        across_cpus["AllAvailable"] & ~across_cpus["AnyDetected"]
    )

    summary_table = pd.concat(
        [
            per_cpu[[CPU_LABEL, "Vulnerability", "Result"]],
            all_rows,
            none_rows,
        ],
        ignore_index=True,
    )

    compressed_table = summary_table.loc[
        summary_table["Result"]
    ].copy()


    palette = [
            *[sns.mpl_palette('viridis', 7)[0] for _ in range(7)],
            #'#fc801f',
            '#fc3a2f', '#fc801f']
    fig, ax = plt.subplots(figsize=(12, 4))
    sns.swarmplot(
            x="Vulnerability", y=CPU_LABEL,
            data=compressed_table,
            order = [
                *CPU_LIST,
                #AND_CPU_NAME,
                #NONE_CPU_NAME,
            ],
            hue_order = [
                *CPU_LIST,
                #AND_CPU_NAME,
                #NONE_CPU_NAME,
            ],
            orient='h', ax=ax,
            hue = CPU_LABEL,
            palette=palette,
    )
    plt.axvline(x=44.5, color='black', lw=1, alpha=0.5)
    x_label = "Vulnerability"
    y_label = CPU_LABEL
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_xlim(0, 89)
    ax.xaxis.set_major_locator(MultipleLocator(5))
    ax.xaxis.set_minor_locator(AutoMinorLocator(5))
    ax.grid(which='major', color='#CCCCCC', linestyle='--')
    ax.grid(which='minor', color='#CCCCCC', linestyle=':')
    plt.tight_layout(pad=1)
    fig.savefig(f"figures/compressed_benchmark_output{sfx}.eps", dpi=1000)#, format="png")
    plt.close(fig)


    # COMPUTE CTVS
    groups = {
        "All": range(1, VUL_NUM + 1),
        "IA": IA_BASE,
        "IS": IS_BASE,
        "ISA": ISA_BASE,
        "EA": EA_BASE,
        "ES": ES_BASE,
        "ESA": ESA_BASE,
    }
    row_order = [*CPU_LIST, AND_CPU_NAME, NONE_CPU_NAME]

    ctvs_dict = {}
    for name, vulnerabilities in groups.items():
        counts = (
            summary_table.loc[
                summary_table["Vulnerability"].isin(vulnerabilities)
            ]
            .groupby(CPU_LABEL)["Result"]
            .sum()
            .reindex(row_order, fill_value=0)
            .astype(int)
        )
        ctvs_dict[name] = counts.rename("Result").reset_index()

    category_sum = sum(
        ctvs_dict[name].set_index(CPU_LABEL)["Result"]
        for name in groups
        if name != "All"
    )
    assert category_sum.equals(
        ctvs_dict["All"].set_index(CPU_LABEL)["Result"]
    ), "Category counts do not sum to the total"

    print(f"\nAll the vulnerability types found vulnerable for the machines (/88): ")
    print(ctvs_dict["All"])
    print(f"I-A Vulnerability types found vulnerable for the machines (/{len(IA_BASE)}):")
    print(ctvs_dict["IA"])
    print(f"I-S Vulnerability types found vulnerable for the machines (/{len(IS_BASE)}):")
    print(ctvs_dict["IS"])
    print(f"I-SA Vulnerability types found vulnerable for the machines (/{len(ISA_BASE)}):")
    print(ctvs_dict["ISA"])
    print(f"E-A Vulnerability types found vulnerable for the machines (/{len(EA_BASE)}):")
    print(ctvs_dict["EA"])
    print(f"E-S Vulnerability types found vulnerable for the machines (/{len(ES_BASE)}):")
    print(ctvs_dict["ES"])
    print(f"E-SA Vulnerability types found vulnerable for the machines (/{len(ESA_BASE)}):")
    print(ctvs_dict["ESA"])


    def get_case(vulnerability, config):
        return case_array[(vulnerability-1)*16+config-1]

    def get_access_inv(vulnerability, config):
        if vulnerability <= VUL_NUM/2:
            if config % 4 in (1,2):
                return READ_ACC
            else:
                return WRITE_ACC
        else:
            if config % 4 in (1,2):
                return FLUSH_INV
            else:
                return WRITE_INV

    table = original_table.copy()
    table["Max. cases"] = table["Result"]>-1
    table["Result"] = table["Result"]>0
    table["Case"] = table.apply(
            lambda x: get_case(x["Vulnerability"],x[CONFIG_LABEL]),
            axis = 1
    )
    table["Access/inv."] = table.apply(
            lambda x: get_access_inv(x["Vulnerability"],x[CONFIG_LABEL]),
            axis = 1
    )
    table["Acc./Inv. Operation"] = table["Access/inv."].map(
            operation_mapping)
    rej_table = table.groupby(["Vulnerability", "Test config."]).filter(
            lambda x: ~x["Max. cases"].all()
    )
    acc_table = table.groupby(["Vulnerability", "Test config."],
                          as_index=False).filter(
            lambda x: x["Max. cases"].all()
    )


    tmp_table = acc_table.copy()
    tmp_table2 = rej_table.copy()
    tmp_count = tmp_table.groupby([CPU_LABEL, "Case"], as_index=False).agg(
            {"Result": 'sum', "Max. cases": 'sum'}
    )
    tmp_count2 = tmp_table2.groupby([CPU_LABEL, "Case"], as_index=False).agg(
            {"Result": 'sum', "Max. cases": 'sum'}
    )
    #tmp_count["Case"] = tmp_count["Case"].map(case_mapping)
    #print(tmp_count)
    count_cases = np.unique(np.array(case_array),return_counts=True)
    for i, count in zip(count_cases[0], count_cases[1]):
        if i == ONLY_VICTIM:
            print(f"\nVulnerability cases running with victim found vulnerable for the machines (/{count}):")
            print("- Common:")
            print(tmp_count.loc[tmp_count["Case"] == ONLY_VICTIM,
                                [CPU_LABEL,"Result","Max. cases"]])
            print("- Exclusive:")
            print(tmp_count2.loc[tmp_count2["Case"] == ONLY_VICTIM,
                                [CPU_LABEL,"Result","Max. cases"]])
        elif i == HAS_ATT:
            print(f"Vulnerability cases with victim and attacker running on different cores found vulnerable (/{count}):")
            print("- Common:")
            print(tmp_count.loc[tmp_count["Case"] == HAS_ATT,
                                [CPU_LABEL,"Result","Max. cases"]])
            print("- Exclusive:")
            print(tmp_count2.loc[tmp_count2["Case"] == HAS_ATT,
                                [CPU_LABEL,"Result","Max. cases"]])
        elif i == TIME_SLICING:
            print(f"Vulnerability cases with victim and attacker running time-slicing found vulnerable (/{count}):")
            print("- Common:")
            print(tmp_count.loc[tmp_count["Case"] == TIME_SLICING,
                                [CPU_LABEL,"Result","Max. cases"]])
            print("- Exclusive:")
            print(tmp_count2.loc[tmp_count2["Case"] == TIME_SLICING,
                                [CPU_LABEL,"Result","Max. cases"]])
        elif i == HYPERTHREADING:
            print(f"Vulnerability cases with victim and attacker running hyper-threading found vulnerable (/{count}):")
            print("- Common:")
            print(tmp_count.loc[tmp_count["Case"] == HYPERTHREADING,
                                [CPU_LABEL,"Result","Max. cases"]])
            print("- Exclusive:")
            print(tmp_count2.loc[tmp_count2["Case"] == HYPERTHREADING,
                                [CPU_LABEL,"Result","Max. cases"]])


    print("")
    tmp_table = acc_table.copy()
    tmp_table2 = rej_table.copy()
    tmp_count = tmp_table.groupby(
            [CPU_LABEL,"Acc./Inv. Operation","Access/inv."],
            as_index=False).agg(
                    {"Max. cases": 'sum', "Result": 'sum'}
    )
    tmp_count2 = tmp_table2.groupby(
            [CPU_LABEL,"Acc./Inv. Operation","Access/inv."],
            as_index=False).agg(
                    {"Max. cases": 'sum', "Result": 'sum'}
    )

    tmp_max = table.groupby([CPU_LABEL, "Access/inv."],
                                as_index=False).agg(
                {"Result": 'count'}
    )

    max_count = tmp_max.loc[tmp_max["Access/inv."] == READ_ACC,
        "Result"].max()
    print(f"Vulnerability cases using local read as observation steps found vulnerable (/{max_count}):")
    print("- Common:")
    print(tmp_count.loc[tmp_count["Access/inv."] == READ_ACC,
                        [CPU_LABEL,"Result","Max. cases"]])
    print("- Exclusive:")
    print(tmp_count2.loc[tmp_count2["Access/inv."] == READ_ACC,
                        [CPU_LABEL,"Result","Max. cases"]])

    max_count = tmp_max.loc[tmp_max["Access/inv."] == WRITE_ACC,
        "Result"].max()
    print(f"Vulnerability cases using local write as observation steps found vulnerable (/{max_count}):")
    print("- Common:")
    print(tmp_count.loc[tmp_count["Access/inv."] == WRITE_ACC,
                        [CPU_LABEL,"Result","Max. cases"]])
    print("- Exclusive:")
    print(tmp_count2.loc[tmp_count2["Access/inv."] == WRITE_ACC,
                        [CPU_LABEL,"Result","Max. cases"]])

    max_count = tmp_max.loc[tmp_max["Access/inv."] == FLUSH_INV,
        "Result"].max()
    print(f"Vulnerability cases using flush as observation steps found vulnerable (/{max_count}):")
    print("- Common:")
    print(tmp_count.loc[tmp_count["Access/inv."] == FLUSH_INV,
                        [CPU_LABEL,"Result","Max. cases"]])
    print("- Exclusive:")
    print(tmp_count2.loc[tmp_count2["Access/inv."] == FLUSH_INV,
                        [CPU_LABEL,"Result","Max. cases"]])

    max_count = tmp_max.loc[tmp_max["Access/inv."] == WRITE_INV,
        "Result"].max()
    print(f"Vulnerability cases using remote write to invalidate as observation steps found vulnerable (/{max_count}):")
    print("- Common:")
    print(tmp_count.loc[tmp_count["Access/inv."] == WRITE_INV,
                        [CPU_LABEL,"Result","Max. cases"]])
    print("- Exclusive:")
    print(tmp_count2.loc[tmp_count2["Access/inv."] == WRITE_INV,
                        [CPU_LABEL,"Result","Max. cases"]])

    print("\n--- /!\\ ---\n"+
          "DISCLAIMER: ignore \"Max. cases\" values for the "+
          "\"Absent from all CPUs\" or \"Present in all CPUs\" cases."+
          "\n--- /!\\ ---")
