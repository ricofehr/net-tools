# Makefile for net tools 
# -------------------

objp = ping.o
objs = spoofing.o

"ping" : ping
"spoofing" : spoofing

ping : $(objp)
	gcc -L/usr/local/lib -fexpensive-optimizations -funroll-loops -finline-functions -Wall -g -O2  -o bin/ping -export-dynamic $(objp) -lm
ping.o: ping.c
	gcc -c ping.c

spoofing : $(objs)
	gcc -L/usr/local/lib -fexpensive-optimizations -funroll-loops -finline-functions -Wall -g -O2  -o bin/spoofing -export-dynamic $(objs) -lm
spoofing.o: spoofing.c
	gcc -c spoofing.c
