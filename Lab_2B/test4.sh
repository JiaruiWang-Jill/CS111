#!/bin/sh

rm -f lab2_list.csv
for i in 1 2 4 8 12
do
    for k in 1 4 8 16
    do
        ./lab2_list --lists=$k --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
        ./lab2_list --lists=$k --threads=$i --iterations=1000 --sync=s >> lab2b_list.csv
    done
done



for i in 1 2 4 8 12
do
    for k in 1 4 8 16
    do
        ./lab2_list --lists=$k --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
        ./lab2_list --lists=$k --threads=$i --iterations=1000 --sync=s >> lab2b_list.csv
    done
done
