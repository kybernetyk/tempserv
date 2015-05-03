#!/bin/sh
cd /home/pi/tempserv
sudo ./tempserv >> tempserv.log
./current-temp.sh > current_temp.txt
./plot-last24h.sh

