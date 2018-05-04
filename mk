#! /bin/sh
gcc -Wall main.c output_status.c tcser.c  midi.c -o switchlog -pthread -lpigpiod_if2 -lrt
