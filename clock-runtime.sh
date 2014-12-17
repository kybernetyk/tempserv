#!/bin/bash
while :
do
	timeout 5s dcled -b 0 -s 100 -f -r clock.dat 
	timeout 5s dcled -b 0 -s 100 -f -r temp.dat 
done

