# Makefile for net tools 
# -------------------

objp = ping.o
objs = spoofing.o

"ping" : ping
"spoofing" : spoofing
"clean" : clean

ping : $(objp)
	gcc -Wall -g -O2  -o bin/ping -export-dynamic $(objp)
ping.o: ping.c
	gcc -c ping.c

spoofing : $(objs)
	gcc -Wall -g -O2  -o bin/spoofing -export-dynamic $(objs)
spoofing.o: spoofing.c
	gcc -c spoofing.c

clean :
	rm -f *.o bin/* 2>/dev/null
