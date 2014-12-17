#!/bin/sh
./today.sh > plotdata.dat

gnuplot -e 'set style data lines; set datafile separator "|"; set terminal postscript color solid; set output "today.ps"; set xdata time; set timefmt x "%H:%M:%S"; set xtics format "%H:%M"; set grid; plot "./plotdata.dat" u 1:2 with lines; quit;'

#gnuplot -e 'xrange [0:23]; set datafile separator "|"; set terminal postscript color solid; set output "today.ps"; plot "./plotdata.dat" using 2 with lines; quit;'
rm plotdata.dat
