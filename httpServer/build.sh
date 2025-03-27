#!/bin/bash

clang -g -O0 server.c parser.c -o server
./server
