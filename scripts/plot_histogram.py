#
# This file is the script file for generating histogram
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

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import sys
import argparse

params = {
    'legend.title_fontsize': 15,
    'legend.fontsize': 14,
    'legend.handlelength': 2,
    #'legend.frameon': False,
    'axes.titlesize'      : 15,
    'axes.labelsize'      : 15,
    'xtick.labelsize' : 14,
    'ytick.labelsize' : 14
}
plt.rcParams.update(params)

primary_colors = {
    "READ": "#ff3f3f",
    "WRITE": "#0171CF",
    "FLUSH": "#12d15c",
}
overlay_colors = {
    "READ": "#8e44ad",
    "WRITE": "#e69500",
    "FLUSH": "#008b8b",
}

x_label = "Clock cycles"
y_label = "Timing types"


def expand_df(table):
    freq_dict = {}
    for col in table.columns:
        if col != "Cycles":
            freq_dict[col] = table.loc[table.index.repeat(table[col]),"Cycles"]
            freq_dict[col] = freq_dict[col].to_numpy()

    for col in table.columns:
        if col == "Cycles":
            continue
        else:
            size = freq_dict[col].size
            if size < 10000:
                if size == 0:
                    del freq_dict[col]
                else:
                    print(col)
    return pd.DataFrame(freq_dict)

def load_table(input_file_name):
    results = pd.read_csv(input_file_name, sep=",", header=[0,1])
    results = results.loc[:,results.columns[:-1]]
    cycles = pd.DataFrame(results[:]["Cycles"])

    read = pd.DataFrame(results["READ"])
    read["Cycles"] = cycles
    read = expand_df(read)
    read["Operation"] = "READ"

    write_operation = False
    if "WRITE" in results.columns:
        write = pd.DataFrame(results["WRITE"])
        write["Cycles"] = cycles
        write = expand_df(write)
        write["Operation"] = "WRITE"
        write_operation = True

    flush_operation = False
    if "FLUSH" in results.columns:
        flush = pd.DataFrame(results["FLUSH"])
        flush["Cycles"] = cycles
        flush = expand_df(flush)
        flush["Operation"] = "FLUSH"
        flush_operation = True

    if write_operation and flush_operation:
        table = pd.concat([read, write, flush], axis=0)
    elif write_operation:
        table = pd.concat([read, write], axis=0)
    elif flush_operation:
        table = pd.concat([read, flush], axis=0)
    else:
        table = pd.concat([read], axis=0)

    table = pd.melt(table,
                    id_vars=["Operation"],
                    var_name="Timing type",
                    value_name="Cycles")
    return table

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Plot timing summaries, optionally overlaying another CSV."
    )
    parser.add_argument("input_file")
    parser.add_argument("output_file", help="Output filename without .eps")
    parser.add_argument("--overlay", metavar="CSV",
                        help="Second CSV with the same format")
    parser.add_argument("--overlay2", metavar="CSV",
                        help="Third CSV with the same format")
    parser.add_argument("--primary", type=str,
                        help="Primary label")
    parser.add_argument("--secondary", type=str,
                        help="Secondary label")
    parser.add_argument("--tertiary", type=str,
                        help="Tertiary label")

    args = parser.parse_args()

    table = load_table(args.input_file)
    if not args.overlay:
        operations = [
            op for op in ("READ", "WRITE", "FLUSH")
        ]

        fig, ax = plt.subplots(figsize=(9, 6))
        sns.barplot(
                x="Cycles", y="Timing type",
                data=table, hue="Operation", hue_order=operations,
                orient='h',
                errorbar=("pi",95),
                ax=ax, palette=primary_colors, alpha=.6
        )

        ax.set_xlabel(x_label)
        ax.set_ylabel(y_label)
        ax.xaxis.grid()
        plt.tight_layout()
        fig.savefig(f"{args.output_file}.eps", dpi=1000)#, format="png")
        plt.close(fig)

    else:
        primary_str = args.primary if args.primary is not None else "Primary"
        secondary_str = args.secondary if args.secondary is not None \
                        else "Secondary"
        tertiary_str = args.tertiary if args.tertiary is not None \
                        else "Tertiary"

        overlay = load_table(args.overlay) if args.overlay else None
        overlay2 = load_table(args.overlay2) if args.overlay2 else None

        combined = pd.concat(
            [table, overlay] if args.overlay is not None else [table],
            ignore_index=True,
        )
        if args.overlay2 is not None:
            combined = pd.concat(
                [table, overlay2],
                ignore_index=True,
            )

        timing_order = combined["Timing type"].drop_duplicates().tolist()
        operations = [
            op for op in ("READ", "WRITE", "FLUSH")
            if op in combined["Operation"].unique()
        ]

        fig, ax = plt.subplots(figsize=(9, 6))

        frames = [table.assign(Dataset=primary_str)]
        if overlay is not None:
            frames.append(overlay.assign(Dataset=secondary_str))
        if overlay2 is not None:
            frames.append(overlay2.assign(Dataset=tertiary_str))

        plot_data = pd.concat(frames, ignore_index=True)
        dataset_order = [primary_str]
        if overlay is not None:
            dataset_order.append(secondary_str)
        if overlay2 is not None:
            dataset_order.append(tertiary_str)

        g = sns.catplot(
            data=plot_data,
            kind="bar",
            x="Cycles",
            y="Timing type",
            hue="Dataset",
            col="Operation",
            order=timing_order,
            hue_order=dataset_order,
            col_order=operations,
            palette={
                primary_str: "#0171CF",
                secondary_str: "#12d15c",
                tertiary_str: "#f1c40f",
            },
            orient="h",
            dodge=True,
            errorbar=("pi", 95),
            capsize=0.1,
            height=max(4, 0.6 * len(timing_order)),
            aspect=0.85,
            sharex=True,
            sharey=True,
            #legend_out=True,
        )

        g.legend.set_title("Configuration")
        g.set_axis_labels("Clock cycles", "Timing types")
        g.set_titles("{col_name}")

        for ax in g.axes.flat:
            ax.xaxis.grid()
            ax.set_axisbelow(True)

        # Fixed overall dimensions, independent of facet count
        #g.fig.set_size_inches(11, 6)
        sns.move_legend(
                g, 'upper right', bbox_to_anchor=(0.96, 0.95), frameon=True,
        )

        # Save without automatic bounding-box cropping
        g.fig.savefig(
            f"{args.output_file}.eps",
            dpi=1000,
            bbox_inches=None,
        )

        plt.close(g.fig)
        #g.legend.set_title("Configuration")
        #g.set_axis_labels("Clock cycles", "Timing types")
        #g.set_titles("{col_name}")

        #for ax in g.axes.flat:
        #    ax.xaxis.grid()
        #    ax.set_axisbelow(True)

        #g.savefig(f"{args.output_file}.eps", dpi=1000)
        #plt.close(g.fig)
        exit(0)

        #def draw(data, palette, label, fill):
        #    sns.barplot(
        #            x="Cycles", y="Timing type",
        #            data=table, hue="Operation", hue_order=operations,
        #            orient='h', dodge=True, fill=fill,
        #            errorbar=("pi",95),
        #            linewidth=1.5,
        #            ax=ax, palette=palette, alpha=.6,
        #            err_kws={
        #                "linestyle": "-" if fill else "--",
        #                "linewidth": 1.5,
        #            },
        #    )
        #    handles, labels = ax.get_legend_handles_labels()
        #    # Only collect legend entries added by this call.
        #    count = len(operations)
        #    return handles[-count:], [
        #        f"{op}" for op in labels[-count:]
        #    ]

        #handles, labels = draw(table, primary_colors, primary_str, True)

        #if args.overlay is not None:
        #    extra_handles, extra_labels = draw(
        #        overlay, overlay_colors, secondary_str, False
        #    )
        #    handles.extend(extra_handles)
        #    labels.extend(extra_labels)

        #ax.legend(handles, labels)

        #ax.set_xlabel(x_label)
        #ax.set_ylabel(y_label)
        #ax.xaxis.grid()
        #plt.tight_layout()
        #fig.savefig(f"{args.output_file}.eps", dpi=1000)#, format="png")
        #plt.close(fig)
