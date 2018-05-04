#! /bin/sh
gcc -Wall output_status.c tcser.c  midi.c -pthread -lpigpiod_if2 -lrt
