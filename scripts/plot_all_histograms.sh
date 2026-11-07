python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_unmatched.csv figures/histograms/u74-histogram_icache_cmp --overlay results/histogram-outputs/smp_histogram_icache_unmatched.csv --primary "AMP" --secondary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_unmatched.csv figures/histograms/u74-histogram_dcache_cmp --overlay results/histogram-outputs/smp_histogram_dcache_unmatched.csv --primary "AMP" --secondary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_bv_ahead.csv figures/histograms/c910-histogram_icache_cmp --overlay results/histogram-outputs/smp_histogram_icache_bv_ahead.csv --primary "AMP" --secondary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bv_ahead.csv figures/histograms/c910-histogram_dcache_cmp --overlay results/histogram-outputs/smp_histogram_dcache_bv_ahead.csv --primary "AMP" --secondary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bv_fire.csv figures/histograms/u54-histogram_dcache_cmp --overlay results/histogram-outputs/smp_histogram_dcache_bv_fire.csv --primary "AMP" --secondary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_bv_fire.csv figures/histograms/u54-histogram_icache_cmp --overlay results/histogram-outputs/smp_histogram_icache_bv_fire.csv --primary "AMP" --secondary "SMP"

python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bpi_f3.csv figures/histograms/amp/amp-x60-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_bpi_f3.csv figures/histograms/amp/amp-x60-histogram_icache
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_unmatched.csv figures/histograms/amp/amp-u74-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_unmatched.csv figures/histograms/amp/amp-u74-histogram_icache
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_bv_fire.csv figures/histograms/amp/amp-u54-histogram_icache
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bv_fire.csv figures/histograms/amp/amp-u54-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bv_ahead.csv figures/histograms/amp/amp-c910-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_bv_ahead.csv figures/histograms/amp/amp-c910-histogram_icache

python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_dcache_bpi_f3.csv figures/histograms/smp/smp-x60-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_icache_bpi_f3.csv figures/histograms/smp/smp-x60-histogram_icache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_icache_bpi_f3.csv figures/histograms/smp/smp-x60-histogram_icache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_dcache_unmatched.csv figures/histograms/smp/smp-u74-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_icache_unmatched.csv figures/histograms/smp/smp-u74-histogram_icache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_icache_bv_fire.csv figures/histograms/smp/smp-u54-histogram_icache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_dcache_bv_fire.csv figures/histograms/smp/smp-u54-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_dcache_bv_ahead.csv figures/histograms/smp/smp-c910-histogram_dcache
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_icache_bv_ahead.csv figures/histograms/smp/smp-c910-histogram_icache

python scripts/plot_histogram.py results/histogram-outputs/cluster1/amp_histogram_dcache_bpi_f3.csv figures/histograms/x60-histogram_dcache_cluster1_cmp --overlay results/histogram-outputs/cluster1/smp_histogram_dcache_bpi_f3.csv --primary "AMP" --secondary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/cluster1/amp_histogram_icache_bpi_f3.csv figures/histograms/x60-histogram_icache_cluster1_cmp --overlay results/histogram-outputs/cluster1/smp_histogram_icache_bpi_f3.csv --primary "AMP" --secondary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bpi_f3.csv figures/histograms/x60-histogram_dcache_amp_cluster1_cmp --overlay results/histogram-outputs/cluster1/amp_histogram_dcache_bpi_f3.csv --primary "AMP" --secondary "AMP - RC"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_bpi_f3.csv figures/histograms/x60-histogram_icache_amp_cluster1_cmp --overlay results/histogram-outputs/cluster1/amp_histogram_icache_bpi_f3.csv --primary "AMP" --secondary "AMP - RC"
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_dcache_bpi_f3.csv figures/histograms/x60-histogram_dcache_smp_cluster1_cmp --overlay results/histogram-outputs/cluster1/smp_histogram_dcache_bpi_f3.csv --primary "SMP" --secondary "SMP - RC"
python scripts/plot_histogram.py results/histogram-outputs/smp_histogram_icache_bpi_f3.csv figures/histograms/x60-histogram_icache_smp_cluster1_cmp --overlay results/histogram-outputs/cluster1/smp_histogram_icache_bpi_f3.csv --primary "SMP" --secondary "SMP - RC"

python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bpi_f3.csv figures/histograms/x60-histogram_dcache_cmp --overlay results/histogram-outputs/no-prefetching/amp_histogram_dcache_bpi_f3.csv --overlay2 results/histogram-outputs/smp_histogram_dcache_bpi_f3.csv --primary "AMP" --secondary "AMP - No PF" --tertiary "SMP"
python scripts/plot_histogram.py results/histogram-outputs/amp_histogram_icache_bpi_f3.csv figures/histograms/x60-histogram_icache_cmp --overlay results/histogram-outputs/no-prefetching/amp_histogram_icache_bpi_f3.csv --overlay2 results/histogram-outputs/smp_histogram_icache_bpi_f3.csv --primary "AMP" --secondary "AMP - No PF" --tertiary "SMP"
