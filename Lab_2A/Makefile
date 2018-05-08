#NAME:Liuyi Shi
#EMAIL:liuyi.shi@outlook.com
#ID:904801945

FILES = lab2_add.c Makefile README testscript.sh *.png *.csv *.gp lab2_list.c SortedList.h SortedList.c

.SILENT:

build:
	gcc lab2_add.c -Wall -Wextra -pthread -o lab2_add
	gcc lab2_list.c SortedList.c SortedList.h -Wall -Wextra -pthread -o lab2_list

tests:  
	chmod +x testscript.sh
	./testscript.sh
graphs:
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

dist:   build tests graphs
	tar -cvzf lab2a-904801945.tar.gz $(FILES)
clean:
	rm -f *.tar.gz ./lab2_add ./lab2_list *.gch *.o
