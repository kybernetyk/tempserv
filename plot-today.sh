#!/bin/sh
./today.sh |sed 's/|/ /' > plotdata.dat
gnuplot -e "set terminal postscript color solid; set output 'today.ps'; plot './plotdata.dat'; quit;"
rm plotdata.dat
