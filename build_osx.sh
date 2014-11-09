#!/bin/sh

clang++ -std=c++11 `pkg-config hidapi --cflags` `pkg-config hidapi --libs` -lsqlite3 -o tempserv CelSQL.cpp main.cpp

