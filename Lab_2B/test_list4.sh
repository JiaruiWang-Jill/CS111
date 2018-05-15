#!/bin/sh


for i in 1 4 8 12 16
do
    for j in 1 2 4 8 16
    do
	./lab2_list --yield=id --threads=$i --iterations=$j --list=4 >> lab2_list.csv
	./lab2_list --yield=id --threads=$i --iterations=$j --sync=s --list=4 >> lab2_list.csv
	./lab2_list --yield=id --threads=$i --iterations=$j --sync=m --list=4 >> lab2_list.csv
    done
done



