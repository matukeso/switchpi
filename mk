#! /bin/sh
gcc -Wall main.c sendpgm.c tcser.c  switch232c.c -o switchlog -pthread -lpigpiod_if2 -lrt
