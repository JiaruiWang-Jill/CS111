.SILENT:

default: client server
    
client:
	gcc -g -o lab1b-client lab1b-client.c -lz
    
server:
	gcc -g -o lab1b-server lab1b-server.c -lz

clean:
	rm -f *.tar.gz lab1b-client lab1b-server *.txt

dist:
	tar -czvf lab1b-904801945.tar.gz Makefile README \
	lab1b-client.c lab1b-server.c