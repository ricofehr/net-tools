# net-tools
Some simple Network tools, currently ping and spoofing programs.

## Compile

Requirements
- Linux operating system

For ping program
```
$ make ping
gcc -c ping.c
gcc -Wall -g -O2  -o bin/ping -export-dynamic ping.o
```

For spoofing program
```
$ make spoofing
gcc -c spoofing.c
gcc -Wall -g -O2  -o bin/spoofing -export-dynamic spoofing.o
```

## Folders
```
/		Sources are on the root of Repository
+--bin/		Binary folder where executables are written
```

## Run

Ping example
```
$ sudo ./bin/./ping 192.168.1.1
PING 192.168.1.1 (192.168.1.1) 56(84) bytes of data.
8 bytes from 192.168.1.1: icmp_seq=1 ttl=63
8 bytes from 192.168.1.1: icmp_seq=2 ttl=63
8 bytes from 192.168.1.1: icmp_seq=3 ttl=63
8 bytes from 192.168.1.1: icmp_seq=4 ttl=63
8 bytes from 192.168.1.1: icmp_seq=5 ttl=63
8 bytes from 192.168.1.1: icmp_seq=6 ttl=63
8 bytes from 192.168.1.1: icmp_seq=7 ttl=63
8 bytes from 192.168.1.1: icmp_seq=8 ttl=63
8 bytes from 192.168.1.1: icmp_seq=9 ttl=63
^C
--- ping statistics ---
9 packets transmitted, 9 packets received, 0 packet loss
```

Spoofing example (src: 192.168.1.15, dest: 192.168.1.1, port: 23)
```
$ sudo ./bin/./spoofing 192.168.1.15 192.168.1.1 23
```

## Todo

- Accept hostname for ping
- More verbose for spoofing
- Add other tools
