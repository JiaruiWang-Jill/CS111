.SILENT:

default:
	gcc -Wall -Wextra -lmraa -lm -g lab4c_tcp.c -o lab4c_tcp
	gcc -Wall -Wextra -lmraa -lm -lssl -lcrypto -g lab4c_tls.c -o lab4c_tls

clean:
	rm -f *.tar.gz *.txt lab4c_tcp lab4c_tls

dist:
	tar -czf lab4c-904801945.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c
