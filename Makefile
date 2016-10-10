# Makefile for net tools 
# -------------------

objp = ping.o

"ping" : ping

ping : $(objp)
	gcc -L/usr/local/lib -fexpensive-optimizations -funroll-loops -finline-functions -Wall -g -O2  -o	bin/ping -export-dynamic $(objp) -lm
ping.o: ping.c
	gcc -c ping.c
