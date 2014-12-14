#!/bin/sh
./last-30-days.sh > plotdata.dat
gnuplot -e 'set style data lines; set datafile separator "|"; set terminal postscript color solid; set output "last30d.ps"; set xdata time; set timefmt x "%Y-%m-%d %H:%M:%S"; set grid; plot "./plotdata.dat" u 1:2 with lines; quit;'
rm plotdata.dat
