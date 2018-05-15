#!/bin/sh

rm -f lab2_list.csv

for i in 1 2 4 8 12 16 24
do
    ./lab2_list --threads=$i --iterations=1000 --sync=m >> lab2_list.csv
done

for i in 1 2 4 8 12 16 24
do
    ./lab2_list --threads=$i --iterations=1000 --sync=s >> lab2_list.csv
done

for i in 1 4 8 12 16
do
    for j in 1 2 4 8 16
    do
	./lab2_list --yield=id --threads=$i --iterations=$j --list=4 >> lab2_list.csv

    done
done

for i in 1 4 8 12 16
do
    for j in 10 20 40 80
    do
	./lab2_list --yield=id --threads=$i --iterations=$j --sync=s --list=4 >> lab2_list.csv
	./lab2_list --yield=id --threads=$i --iterations=$j --sync=m --list=4 >> lab2_list.csv
    done
done 

for i in 1 2 4 8 12
do
    for k in 4 8 16
    do
        ./lab2_list --lists=$k --threads=$i --iterations=1000 --sync=m >> lab2_list.csv
        ./lab2_list --lists=$k --threads=$i --iterations=1000 --sync=s >> lab2_list.csv
    done
done


