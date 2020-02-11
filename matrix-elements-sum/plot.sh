#!/bin/bash

if [ -z "$1" ] || [ "$1" = "--help" ]
then
    printf 'Usage: %s raw-result-file\n' "$0"
    exit
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
clean="$(echo "$1" | sed 's/raw-/clean-/')"

grep -v '^#' "$1" | "$script_dir"/raw-to-plot.py "-" > "$clean"

N=$(grep '^[^0-9]' "$clean" -c)

gnuplot <<EOF
set term png linewidth 6 size 2048,1536 font Arial 32
set pointsize 4
set output "$(echo "$clean" | sed 's/\.txt$/.png/')"
set xlabel "Matrix size (n, as in n×n)"
set ylabel "Relative duration, the lower the better"
set logscale x 2
set logscale y
set title "$(grep '^# *Title:' "$1" | cut -d: -f2 | sed 's/^ *//')"
set xrange [ 1 : 32768 ]
set yrange [ 0 : * ]
set bmargin 7
set key outside center bottom horizontal Left reverse font ",26" 
plot for [IDX=0:$((N-1))] '$clean' i IDX using 1:2 with linespoints title columnheader(1)
EOF
