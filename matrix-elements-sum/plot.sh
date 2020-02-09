#!/bin/bash

if [ -z "$1" ] || [ "$1" = "--help" ]
then
    printf 'Usage: %s raw-result-file\n' "$0"
    exit
fi

clean="$(echo "$1" | sed 's/raw-/clean-/')"

./raw-to-plot.py "$1" > "$clean"

N=$(grep '^[^0-9]' "$clean" -c)

gnuplot <<EOF
set term png linewidth 4 size 2048,1536 font Arial 20
set output "$(echo "$clean" | sed 's/\.txt$/.png/')"
set xlabel "Matrix size"
set ylabel "Relative duration, the lower the better"
set logscale x 2
set logscale y
set title "abc"
set xrange [ 1 : 32768 ]
set yrange [ 0 : * ]
set key outside center bottom horizontal Left reverse
plot for [IDX=0:$((N-1))] '$clean' i IDX using 1:2 with linespoints title columnheader(1)
EOF
