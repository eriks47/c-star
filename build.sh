#!/bin/sh

gcc -ggdb -lraylib src/demo.c src/a_star.c -o build/demo
./build/main
