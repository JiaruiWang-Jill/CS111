#!/bin/sh
for i in 1 2 4 8 12 16 24 
do
    ./lab2_list --threads=$i --iterations=1000 --sync=m --list=1 >> lab2_list.csv
done

for i in 1 2 4 8 12 16 24 
do
    ./lab2_list --threads=$i --iterations=1000 --sync=s --list=1 >> lab2_list.csv
done


