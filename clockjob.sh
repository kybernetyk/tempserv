#!/bin/sh
# script to feed the dream cheeky LED display with current
# time and temp
# this should be called from a cronjob every minute

cd /home/pi/tempserv
echo `date +%H:%M` `tail -n 1 tempserv.log |cut -d ' ' -f 3 - | sed 's/°/C/'` > clock_and_temp.dat

awk '{print $1}' clock_and_temp.dat > clock.dat
awk '{print $2}' clock_and_temp.dat > temp.dat

